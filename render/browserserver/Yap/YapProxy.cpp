/* @@@LICENSE
*
*      Copyright (c) 2012 Hewlett-Packard Development Company, L.P.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
LICENSE@@@ */

/* Modified by the Jihad Browser project (2026), per Apache-2.0 section 4(b): this file differs from
 * HP's isis-project original. The changes are this repository's git history for this file. */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "YapDefs.h"
#include "YapPacket.h"
#include "YapServer.h"
#include "YapProxy.h"

static const int kMaxConnectTries = 5;
#include <poll.h>

YapProxy::ExpressHook YapProxy::s_expressHook = 0;
YapProxy::DispatchHook YapProxy::s_dispatchHook = 0;
uint32_t YapProxy::s_currentTag = 0;

gboolean YapProxyIoFunction(GIOChannel* channel, GIOCondition condition, void* data);

YapProxy::Message::Message( const char* hdr, const uint8_t* data, int dataLen ) : data(NULL)
{
    ::memcpy(this->hdr, hdr, sizeof(this->hdr));
    this->data = new uint8_t[dataLen];
    if (this->data != NULL) {
        ::memcpy(this->data, data, dataLen);
    }
    len = dataLen;
}

YapProxy::Message::~Message()
{
    delete [] data;
}

/**
 * Constructor.
 *
 * @param server The Yap server.
 * @param cmdSocketFd The command socket file descriptor. Can be -1 if there is no command socket.
 * @param The path to the message socket. If NULL then outbound messages will be recorded and
 *            sent later once connected.
 */
YapProxy::YapProxy(YapServer* server, int cmdSocketFd, char* msgSocketPath, char* msgSocketPostfix)
    : m_server(server)
    , m_cmdSocketFd(cmdSocketFd)
    , m_msgSocketFd(-1)
    , m_msgSocketPostfix(0)
    , m_privData(0)
    , m_cmdBuffer(0)
    , m_replyBuffer(0)
    , m_msgBuffer(0)
    , m_ioChannel(0)
    , m_ioSource(0)
    , m_inSyncMode(false)
    , m_terminate(false)
    , m_packetCommand(0)
    , m_packetReply(0)
    , m_packetMessage(0)
    , m_threaded(false)
    , m_tagNext(0)
    , m_wakeChannel(0)
    , m_wakeSource(0)
{
    m_wakePipe[0] = m_wakePipe[1] = -1;
    if (msgSocketPostfix) {

        int length = strlen(msgSocketPostfix);

        if (length > 0) {

            m_msgSocketPostfix = new char[length + 1];
            strncpy(m_msgSocketPostfix, msgSocketPostfix, length);
            m_msgSocketPostfix[length] = '\0';
        }
    }

    m_cmdBuffer   = new uint8_t[kMaxMsgLen];
    m_replyBuffer = new uint8_t[kMaxMsgLen];
    m_msgBuffer   = new uint8_t[kMaxMsgLen];

    m_packetCommand = new YapPacket(m_cmdBuffer, 0);
    m_packetReply   = new YapPacket(m_replyBuffer);
    m_packetMessage = new YapPacket(m_msgBuffer);

    GMainContext* mainCtxt = g_main_loop_get_context(m_server->mainLoop());

    if (m_cmdSocketFd != -1 && s_expressHook && ::pipe(m_wakePipe) == 0) {
        // T-515: a reader thread reads the commands; the main loop wakes on the pipe
        ::fcntl(m_wakePipe[0], F_SETFL, ::fcntl(m_wakePipe[0], F_GETFL) | O_NONBLOCK);
        m_wakeChannel = g_io_channel_unix_new(m_wakePipe[0]);
        m_wakeSource  = g_io_create_watch(m_wakeChannel, (GIOCondition) G_IO_IN);
        g_source_set_callback(m_wakeSource, (GSourceFunc) YapProxy::wakeFunction, this, NULL);
        g_source_attach(m_wakeSource, mainCtxt);
        g_source_set_priority(m_wakeSource, G_PRIORITY_HIGH);
        m_threaded = ::pthread_create(&m_reader, NULL, YapProxy::readerMain, this) == 0;
        if (!m_threaded) {
            g_source_destroy(m_wakeSource); g_source_unref(m_wakeSource); m_wakeSource = 0;
            g_io_channel_unref(m_wakeChannel); m_wakeChannel = 0;
            ::close(m_wakePipe[0]); ::close(m_wakePipe[1]); m_wakePipe[0] = m_wakePipe[1] = -1;
        }
        fprintf(stderr, "[jihad-bs] YAP commands read on %s\n", m_threaded ? "a reader thread (the express lane)" : "the main loop (no thread)");
    }
    if (m_cmdSocketFd != -1 && !m_threaded) {
        m_ioChannel = g_io_channel_unix_new(m_cmdSocketFd);
        m_ioSource  = g_io_create_watch(m_ioChannel, (GIOCondition) (G_IO_IN | G_IO_HUP));

        g_source_set_callback(m_ioSource, (GSourceFunc) YapProxyIoFunction, this, NULL);
        g_source_attach(m_ioSource, mainCtxt);
        g_source_set_priority(m_ioSource, G_PRIORITY_HIGH);
    }

    if (msgSocketPath) {
        struct sockaddr_un socketAddr = {0};
        socketAddr.sun_family = AF_LOCAL;
        if (strlen(msgSocketPath) < sizeof(socketAddr.sun_path)) {
            ::strncpy(socketAddr.sun_path, msgSocketPath, sizeof(socketAddr.sun_path));
            socketAddr.sun_path[sizeof(socketAddr.sun_path)-1] = '\0';
        }
        else {
            fprintf(stderr, "Socket path length too long\n");
            return;
        }
        m_msgSocketFd = ::socket(PF_LOCAL, SOCK_STREAM, 0);

        fprintf(stderr, "Connecting to browser-adapter (client) socket: %s ...\n", msgSocketPath);

        int connectTryCount = 0;
        bool connected = false;
        while (connectTryCount < kMaxConnectTries) {

            if (::connect(m_msgSocketFd, (struct sockaddr*) &socketAddr, SUN_LEN(&socketAddr)) == 0) {
                connected = true;
                break;
            }
            else if (errno == EAGAIN || errno == EINTR || errno == ETIMEDOUT) {

                fprintf(stderr, "Retrying connect. Error was: %s\n", strerror(errno));
                connectTryCount++;
                continue;
            }
            else {

                break;
            }
        }

        if (!connected) {
            fprintf(stderr, "Failed to connect to client's msg socket. Messages will not work\n");
            ::close(m_msgSocketFd);
            m_msgSocketFd = -1;
        }
        else {
            fprintf(stderr, "Connected to client socket\n");
        }
    }
}

YapProxy::~YapProxy()
{
    delete[] m_msgSocketPostfix;   // allocated with new char[length+1] — must be delete[] (was UB)
    m_msgSocketPostfix = 0;

    if (m_threaded) {
        // T-515: the reader's blocked read returns once the socket is shut down. Never called
        // with the server's lock held: the reader may be waiting for it in the express hook.
        if (m_cmdSocketFd != -1) ::shutdown(m_cmdSocketFd, SHUT_RDWR);
        ::pthread_join(m_reader, NULL);
        m_threaded = false;
    }
    if (m_wakeSource) {
        g_source_destroy(m_wakeSource);
        g_source_unref(m_wakeSource);
        m_wakeSource = 0;
    }
    if (m_wakeChannel) {
        g_io_channel_unref(m_wakeChannel);
        m_wakeChannel = 0;
    }
    if (m_wakePipe[0] != -1) ::close(m_wakePipe[0]);
    if (m_wakePipe[1] != -1) ::close(m_wakePipe[1]);
    while (!m_q.empty()) { delete m_q.front(); m_q.pop_front(); }

    if (m_ioSource) {
        g_source_destroy(m_ioSource);
    }
    if (m_ioChannel) {
        g_io_channel_shutdown(m_ioChannel, TRUE, NULL);
        g_io_channel_unref(m_ioChannel);
    }

    delete [] m_cmdBuffer;
    delete [] m_replyBuffer;
    delete [] m_msgBuffer;

    delete m_packetCommand;
    delete m_packetReply;
    delete m_packetMessage;

    if (m_msgSocketFd != -1)
        ::close(m_msgSocketFd);

    if (m_cmdSocketFd != -1)
        ::close(m_cmdSocketFd);

    while (!m_queuedMessages.empty()) {
        delete m_queuedMessages.front();
        m_queuedMessages.pop();
    }
}

bool YapProxy::isRecordProxy() const
{
    return m_msgSocketFd == -1;
}

/**
 * Transfer the queued messages from the source proxy to this proxy sending them
 * out.
 */
void YapProxy::transferQueuedMessage(YapProxy* srcProxy)
{
    while (!srcProxy->m_queuedMessages.empty()) {
        Message* message = srcProxy->m_queuedMessages.front();
        srcProxy->m_queuedMessages.pop();

        if (message) {
            bool sent = writeSocket(m_msgSocketFd, message->hdr, sizeof(message->hdr));
            if (sent)
                sent = writeSocket(m_msgSocketFd, (char*)message->data, message->len);

            delete message;

            if (!sent) {
                fprintf(stderr, "Error sending queued message");
            }
        }
    }
}

void YapProxy::setPrivateData(void* priv)
{
    m_privData = priv;
}

void* YapProxy::privateData() const
{
    return m_privData;
}

YapPacket* YapProxy::packetMessage()
{
    m_sendLock.lock();   // T-515: released by sendMessage(), which every caller makes next
    m_packetMessage->reset();
    return m_packetMessage;
}

void YapProxy::setTerminate()
{
    m_terminate = true;
}

void YapProxy::sendMessage()
{
    YapLockGuard sendGuard(m_sendLock, true);   // T-515: taken in packetMessage()
    char     pktHeader[4];
    uint16_t pktLen = 0;

    if (m_packetMessage->length() == 0) {
        fprintf(stderr, "Message is empty\n");
        return;
    }

    ::memset(pktHeader, 0, sizeof(pktHeader));

    pktLen = m_packetMessage->length();
    pktLen = bswap_16(pktLen);
    ::memcpy(pktHeader, &pktLen, 2);

    if (m_msgSocketFd != -1) {
        bool sent = writeSocket(m_msgSocketFd, pktHeader, 4);
        if (sent) {
            sent = writeSocket(m_msgSocketFd, (char*) m_msgBuffer, m_packetMessage->length());
        }
        if (!sent) {
            fprintf(stderr, "ERROR sending message, errno=%d.", errno);
            m_sendFailed = true;   // the card's socket is gone (EPIPE): a reply will never come
        }
    }
    else {
        // Save this message for later and send if socket is opened.
        m_queuedMessages.push(new Message(pktHeader, m_msgBuffer, m_packetMessage->length()));
    }
}

void YapProxy::ioFunction(GIOChannel* channel, GIOCondition condition)
{
    char     pktHeader[4] = { 0, 0, 0, 0 };
    uint16_t pktLen    = 0;
    uint8_t  pktFlags  = 0;
    char * ppp = 0;

    if (condition & G_IO_HUP)
        goto Detached;

    // Get the packet header
    if (!readSocket(m_cmdSocketFd, pktHeader, 4)) {
        fprintf(stderr, "YAP: Failed to read packet header\n");
        goto Detached;
    }
    ppp = ((char *)&pktHeader[0]);
    pktLen   = *((uint16_t*) ppp);
    pktLen   = bswap_16(pktLen);
    pktFlags = (pktHeader[3]);
    if (pktLen > kMaxMsgLen) {
        fprintf(stderr, "YAP: Invalid message length %u > %d\n", pktLen, kMaxMsgLen);
        goto Detached;
    }

    // Get the packet data
    if (!readSocket(m_cmdSocketFd, (char*) m_cmdBuffer, pktLen)) {
        fprintf(stderr, "YAP: Failed to read packet data of length: %d\n", pktLen);
        goto Detached;
    }

    if (handlePacket(pktFlags, pktLen))
        return;

 Detached:

    m_server->clientDisconnected(this);
    delete this;
}

// T-515: the command now in m_cmdBuffer (pktLenIn bytes), handled as ioFunction always did; false
// when the proxy must detach (the caller detaches it).
bool YapProxy::handlePacket(uint8_t pktFlags, int pktLenIn)
{
    char     pktHeader[4] = { 0, 0, 0, 0 };
    uint16_t pktLen = (uint16_t) pktLenIn;

    m_inSyncMode = pktFlags & kPacketFlagSyncMask;

    if (m_inSyncMode) {
        m_packetCommand->reset();
        m_packetCommand->setReadTotalLength(pktLen);
        m_packetReply->reset();

        m_server->handleSyncCommand(this, m_packetCommand, m_packetReply);

        pktLen = m_packetReply->length();
        pktLen = bswap_16(pktLen);
        ::memcpy(pktHeader, &pktLen, 2);
        pktHeader[2] = pktFlags;
        pktHeader[3] = 0;

        if (!writeSocket(m_cmdSocketFd, pktHeader, 4)) {
            return false;
        }

        if (m_packetReply->length() > 0) {
            if (!writeSocket(m_cmdSocketFd, (char*) m_replyBuffer, m_packetReply->length())) {
                return false;
            }
        }

        m_packetCommand->reset();
        m_packetCommand->setReadTotalLength(0);
        m_packetReply->reset();
    }
    else {
        m_packetCommand->reset();
        m_packetCommand->setReadTotalLength(pktLen);

        m_server->handleAsyncCommand(this, m_packetCommand);

        m_packetCommand->reset();
        m_packetCommand->setReadTotalLength(0);
    }

    m_inSyncMode = false;

    return !m_terminate;
}

// T-515: the reader thread. Each command is copied into the queue; an async one is offered to the
// express hook first (a scroll, a zoom, a buffer returned: the express lane may answer them while
// the main thread is busy). A closed socket ends the thread with a hang-up entry.
void* YapProxy::readerMain(void* self)
{
    static_cast<YapProxy*>(self)->readerLoop();
    return NULL;
}

bool YapProxy::readWait(int fd, char* buf, int len)
{
    int index = 0;
    while (len > 0) {
        int count = ::read(fd, &buf[index], len);
        if (count > 0) { index += count; len -= count; continue; }
        if (count == 0) return false;   // closed
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            struct pollfd pfd = { fd, POLLIN, 0 };
            ::poll(&pfd, 1, 1000);
            continue;
        }
        return false;
    }
    return true;
}

void YapProxy::readerLoop()
{
    for (;;) {
        char hdr[4];
        if (!readWait(m_cmdSocketFd, hdr, 4)) break;
        uint16_t len;
        ::memcpy(&len, hdr, 2);
        len = bswap_16(len);
        if (len > kMaxMsgLen) {
            fprintf(stderr, "YAP: Invalid message length %u > %d\n", len, kMaxMsgLen);
            break;
        }
        InPacket* p = new InPacket;
        p->flags = (uint8_t) hdr[3]; p->hup = false;
        { YapLockGuard g(m_qLock); p->tag = ++m_tagNext; }
        p->data.resize(len);
        if (len && !readWait(m_cmdSocketFd, (char*) &p->data[0], len)) { delete p; break; }
        if (!(p->flags & kPacketFlagSyncMask) && s_expressHook && s_expressHook(this, len ? &p->data[0] : NULL, len, p->tag)) {
            delete p;   // the express lane applied it
            continue;
        }
        { YapLockGuard g(m_qLock); m_q.push_back(p); }
        char b = 1;
        ssize_t w = ::write(m_wakePipe[1], &b, 1); (void) w;
    }
    InPacket* p = new InPacket;
    p->flags = 0; p->hup = true; p->tag = 0;
    { YapLockGuard g(m_qLock); m_q.push_back(p); }
    char b = 1;
    ssize_t w = ::write(m_wakePipe[1], &b, 1); (void) w;
}

YapPacket* YapProxy::wrapPacket(uint8_t* data, int len)
{
    return len > 0 ? new YapPacket(data, len) : new YapPacket(data);
}

void YapProxy::freePacket(YapPacket* pkt)
{
    delete pkt;
}

bool YapProxy::injectAsync(const uint8_t* data, int len)
{
    if (!m_threaded || len <= 0 || len > kMaxMsgLen) return false;
    uint32_t tag;
    { YapLockGuard g(m_qLock); tag = ++m_tagNext; }
    if (s_expressHook && s_expressHook(this, data, len, tag)) return true;
    InPacket* p = new InPacket;
    p->flags = 0; p->hup = false; p->tag = tag;
    p->data.assign(data, data + len);
    { YapLockGuard g(m_qLock); m_q.push_back(p); }
    char b = 1;
    ssize_t w = ::write(m_wakePipe[1], &b, 1); (void) w;
    return true;
}

gboolean YapProxy::wakeFunction(GIOChannel*, GIOCondition, void* data)
{
    YapProxy* self = static_cast<YapProxy*>(data);
    char buf[64];
    while (::read(self->m_wakePipe[0], buf, sizeof(buf)) > 0) {}
    self->drainQueue();   // may delete self
    return TRUE;
}

// The main loop's side: the queued commands handled in order, each inside the dispatch hook (the
// server's lock). A detach (a hang-up, a failed reply, a terminate) runs inside the hook too, and
// the proxy is deleted after it, outside the lock (the destructor joins the reader).
void YapProxy::drainQueue()
{
    for (;;) {
        InPacket* p = 0;
        {
            YapLockGuard g(m_qLock);
            if (m_q.empty()) return;
            p = m_q.front();
            m_q.pop_front();
        }
        bool alive = false;
        if (s_dispatchHook) s_dispatchHook(true);
        if (!p->hup) {
            const int len = (int) p->data.size();
            if (len) ::memcpy(m_cmdBuffer, &p->data[0], len);
            s_currentTag = p->tag;
            alive = handlePacket(p->flags, len);
            s_currentTag = 0;
        }
        delete p;
        if (!alive) m_server->clientDisconnected(this);
        if (s_dispatchHook) s_dispatchHook(false);
        if (!alive) {
            delete this;
            return;
        }
    }
}

bool YapProxy::readSocket(int fd, char* buf, int len)
{
    int index = 0;

    while (len > 0) {
        int count = ::read(fd, &buf[index], len);
        if (count <= 0) {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            else {
                fprintf(stderr, "Failed to read from socket. Error: %d, %s\n", errno, strerror(errno));
                return false;
            }
        }

        index  += count;
        len    -= count;
    }

    return true;
}

bool YapProxy::writeSocket(int fd, char* buf, int len)
{
    int index = 0;

    while (len > 0) {
        int count = ::write(fd, &buf[index], len);
        if (count <= 0) {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            else {
                fprintf(stderr, "Failed to write to socket. Error: %d, %s\n", errno, strerror(errno));
                return false;
            }
        }

        index  += count;
        len    -= count;
    }

    return true;
}

gboolean YapProxyIoFunction(GIOChannel* channel, GIOCondition condition, void* data)
{
    YapProxy* proxy = static_cast<YapProxy*>(data);
    proxy->ioFunction(channel, condition);
    return TRUE;
}

bool YapProxy::connected() const
{
    return (m_msgSocketFd >= 0);
}

bool YapProxy::sendFailed() const
{
    return m_sendFailed;
}

int YapProxy::messageSocketFd() const
{
    return m_msgSocketFd;
}

int YapProxy::commandSocketFd() const
{
    return m_cmdSocketFd;
}

int YapProxy::serverSocketFd() const
{
    return m_server->serverSocketFd();
}
