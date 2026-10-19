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

#ifndef YAPPROXY_H
#define YAPPROXY_H

#include <stdint.h>
#include <glib.h>
#include <queue>
#include <deque>
#include <vector>
#include <mutex>
#include <atomic>
#include <pthread.h>

class YapServer;
class YapPacket;

class YapProxy
{
public:
    // Jihad T-515, the express lane: when the server sets the express hook, each proxy reads its
    // command socket on a thread of its own and the main loop handles the commands from a queue,
    // as it handled them from the socket before. The hook sees every async command first, on the
    // reader thread; true means it applied the command and the main loop must not see it. The
    // dispatch hook runs around the main loop's handling of each command (the server's lock).
    // tag: the command's number in this proxy's arrival order; the main loop's handling of it can
    // read it back with currentTag()
    typedef bool (*ExpressHook)(YapProxy* proxy, const uint8_t* data, int len, uint32_t tag);
    static uint32_t currentTag() { return s_currentTag; }
    typedef void (*DispatchHook)(bool begin);
    static ExpressHook s_expressHook;
    static DispatchHook s_dispatchHook;

    const char* postfix() const { return m_msgSocketPostfix; }
    void  setPrivateData(void* priv);
    void* privateData() const;
    void  setTerminate();
    bool  isRecordProxy() const;

    YapPacket* packetMessage();
    // T-515, device rounds (the inject channel's pan): an async command handed in as the reader
    // thread would hand it in (the express hook, then the queue), from any thread; false without
    // the reader thread
    bool injectAsync(const uint8_t* data, int len);
    // T-515: a packet over the caller's bytes (YapPacket's constructors are the proxy's): for
    // reading len bytes, or for writing (len 0, the buffer kMaxMsgLen long); freed with freePacket
    static YapPacket* wrapPacket(uint8_t* data, int len);
    static void freePacket(YapPacket* pkt);
    void sendMessage();
    void transferQueuedMessage(YapProxy* srcProxy);

    bool connected() const;

    int messageSocketFd() const;
    int commandSocketFd() const;
    int serverSocketFd() const;

private:

    struct Message {
        char     hdr[4]; ///< Message header
        uint8_t* data;   ///< Message data
        int      len;    ///< Message data length

        Message( const char* hdr, const uint8_t* data, int dataLen );
        ~Message();

        private:

        Message* operator=(const Message& rhs);
        Message(const Message& rhs);
    };

    YapProxy(YapServer* server, int cmdSocketFd, char* msgSocketPath, char* msgSocketPostfix);
    ~YapProxy();

    void ioFunction(GIOChannel* channel, GIOCondition condition);
    // T-515: the reader thread, the queue and the main loop's side of it
    struct InPacket { std::vector<uint8_t> data; uint8_t flags; bool hup; uint32_t tag; };
    std::atomic<uint32_t> m_tagNext;
    static uint32_t s_currentTag;   // the main loop's command being handled (its thread only)
    static void* readerMain(void* self);
    void readerLoop();
    bool readWait(int fd, char* buf, int len);
    static gboolean wakeFunction(GIOChannel* channel, GIOCondition condition, void* data);
    void drainQueue();
    bool handlePacket(uint8_t flags, int len);   // the command in m_cmdBuffer; false: the proxy is gone
    bool          m_threaded;
    pthread_t     m_reader;
    int           m_wakePipe[2];
    GIOChannel*   m_wakeChannel;
    GSource*      m_wakeSource;
    std::mutex    m_qLock;
    std::deque<InPacket*> m_q;
    std::mutex    m_sendLock;   // packetMessage() .. sendMessage(): the main thread and the express lane
    bool readSocket(int fd, char* buf, int len);
    bool writeSocket(int fd, char* buf, int len);

    YapServer*  m_server;
    int         m_cmdSocketFd;
    int         m_msgSocketFd;
    char*       m_msgSocketPostfix;
    void*       m_privData;
    uint8_t*    m_cmdBuffer;
    uint8_t*    m_replyBuffer;
    uint8_t*    m_msgBuffer;
    GIOChannel* m_ioChannel;
    GSource*    m_ioSource;

    bool        m_inSyncMode;
    bool        m_terminate;
    std::queue<Message*> m_queuedMessages; ///< Messages sent before connection go here.

    YapPacket*  m_packetCommand;
    YapPacket*  m_packetReply;
    YapPacket*  m_packetMessage;

    // Copy not allowed
    YapProxy(const YapProxy&);
    YapProxy& operator=(const YapProxy&);

    friend class YapServer;
    friend gboolean YapProxyIoFunction(GIOChannel* channel, GIOCondition condition, void* data);
};

#endif /* YAPPROXY_H */
