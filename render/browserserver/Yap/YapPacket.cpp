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
#include <byteswap.h>

#include <glib.h>

#include "YapDefs.h"
#include "YapPacket.h"

typedef enum YapType_t {
    kYapTypeBool   = (1 << 0),
    kYapTypeInt8   = (1 << 1),
    kYapTypeInt16  = (1 << 2),
    kYapTypeInt32  = (1 << 3),
    kYapTypeInt64  = (1 << 4),
    kYapTypeDouble = (1 << 5),
    kYapTypeString = (1 << 6),
    kYapTypeUInt16 = (1 << 7)
} YapType_t;

YapPacket::YapPacket(uint8_t* buffer)
    : m_buffer(buffer)
    , m_forWriting(true)
    , m_currReadPos(0)
    , m_readTotalLen(0)
    , m_currWritePos(0)
{
}

YapPacket::YapPacket(uint8_t* buffer, int readTotalLen)
    : m_buffer(buffer)
    , m_forWriting(false)
    , m_currReadPos(0)
    , m_readTotalLen(readTotalLen)
    , m_currWritePos(0)
{
}

int YapPacket::length() const
{
    if (m_forWriting)
        return m_currWritePos;
    else
        return m_readTotalLen;
}

void YapPacket::setReadTotalLength(int len)
{
    m_readTotalLen = len;    
}

void YapPacket::reset()
{
    m_currWritePos = 0;
    m_currReadPos  = 0;
}

void YapPacket::operator<<(bool val)
{
    g_return_if_fail(m_forWriting);
    g_return_if_fail((m_currWritePos + 2) <= kMaxMsgLen);

    m_buffer[m_currWritePos++] = kYapTypeBool;
    m_buffer[m_currWritePos++] = val;
}

void YapPacket::operator<<(int8_t val)
{
    g_return_if_fail(m_forWriting);
    g_return_if_fail((m_currWritePos + 2) <= kMaxMsgLen);

    m_buffer[m_currWritePos++] = kYapTypeInt8;
    m_buffer[m_currWritePos++] = val;
}

void YapPacket::operator<<(int16_t val)
{
    g_return_if_fail(m_forWriting);
    g_return_if_fail((m_currWritePos + 3) <= kMaxMsgLen);

    m_buffer[m_currWritePos++] = kYapTypeInt16;

    uint8_t* pSrc = (uint8_t*)(&val);
    uint8_t* pDst = m_buffer + m_currWritePos;

    pDst[0] = pSrc[1];
    pDst[1] = pSrc[0];
    m_currWritePos += 2;
}

void YapPacket::operator<<(uint16_t val)
{
    g_return_if_fail(m_forWriting);
    g_return_if_fail((m_currWritePos + 3) <= kMaxMsgLen);

    m_buffer[m_currWritePos++] = kYapTypeUInt16;

    uint8_t* pSrc = (uint8_t*)(&val);
    uint8_t* pDst = m_buffer + m_currWritePos;

    pDst[0] = pSrc[1];
    pDst[1] = pSrc[0];
    m_currWritePos += sizeof(val);
}

void YapPacket::operator<<(int32_t val)
{
    g_return_if_fail(m_forWriting);
    g_return_if_fail((m_currWritePos + 5) <= kMaxMsgLen);

    m_buffer[m_currWritePos++] = kYapTypeInt32;

    uint8_t* pSrc = (uint8_t*)(&val);
    uint8_t* pDst = m_buffer + m_currWritePos;

    pDst[0] = pSrc[3];
    pDst[1] = pSrc[2];
    pDst[2] = pSrc[1];
    pDst[3] = pSrc[0];
    m_currWritePos += 4;
}

void YapPacket::operator<<(int64_t val)
{
    g_return_if_fail(m_forWriting);
    g_return_if_fail((m_currWritePos + 9) <= kMaxMsgLen);

    m_buffer[m_currWritePos++] = kYapTypeInt64;

    uint8_t* pSrc = (uint8_t*)(&val);
    uint8_t* pDst = m_buffer + m_currWritePos;

    pDst[0] = pSrc[7];
    pDst[1] = pSrc[6];
    pDst[2] = pSrc[5];
    pDst[3] = pSrc[4];
    pDst[4] = pSrc[3];
    pDst[5] = pSrc[2];
    pDst[6] = pSrc[1];
    pDst[7] = pSrc[0];
    m_currWritePos += 8;
}

void YapPacket::operator<<(double val)
{
    g_return_if_fail(m_forWriting);
    g_return_if_fail((m_currWritePos + 9) <= kMaxMsgLen);

    m_buffer[m_currWritePos++] = kYapTypeDouble;

    uint8_t* pSrc = (uint8_t*)(&val);
    uint8_t* pDst = m_buffer + m_currWritePos;

    pDst[0] = pSrc[7];
    pDst[1] = pSrc[6];
    pDst[2] = pSrc[5];
    pDst[3] = pSrc[4];
    pDst[4] = pSrc[3];
    pDst[5] = pSrc[2];
    pDst[6] = pSrc[1];
    pDst[7] = pSrc[0];
    m_currWritePos += 8;
}

void YapPacket::operator<<(const char* val)
{
    int strLen = val ? strlen(val) : 0;

    g_return_if_fail(m_forWriting);
    // Jihad (inspector 2026-10-07, P1): a string that does not fit used to be SKIPPED while the
    // fields after it were still written, so the reader took the next field for this one and the
    // adapter got a NULL string — whose strlen crashed the card's host. Send an EMPTY string in
    // its place (the type byte and a zero length) so every later field stays where it belongs.
    if ((m_currWritePos + strLen + 3) > kMaxMsgLen) {
        g_warning("YapPacket: a %d-byte string does not fit; sent empty", strLen);
        strLen = 0;
    }
    g_return_if_fail((m_currWritePos + strLen + 3) <= kMaxMsgLen);

    m_buffer[m_currWritePos++] = kYapTypeString;

    uint8_t* pSrc = (uint8_t*)(&strLen);
    uint8_t* pDst = m_buffer + m_currWritePos;

    pDst[0] = pSrc[1];
    pDst[1] = pSrc[0];
    m_currWritePos += 2;

    if (strLen) {
        memcpy(m_buffer + m_currWritePos, val, strLen);
        m_currWritePos += strLen;
    }
}

void YapPacket::operator>>(bool& val)
{
    g_return_if_fail(!m_forWriting);
    g_return_if_fail((m_currReadPos + 2) <= m_readTotalLen);

    YapType_t type = (YapType_t) m_buffer[m_currReadPos++];
    if (type != kYapTypeBool) {
        fprintf(stderr, "Arg type is not bool: %d\n", type);
        g_return_if_fail(false);
    }

    val = m_buffer[m_currReadPos++];
}

void YapPacket::operator>>(int8_t& val)
{
    g_return_if_fail(!m_forWriting);
    g_return_if_fail((m_currReadPos + 2) <= m_readTotalLen);

    YapType_t type = (YapType_t) m_buffer[m_currReadPos++];
    if (type != kYapTypeInt8) {
        fprintf(stderr, "Arg type is not char: %d\n", type);
        g_return_if_fail(false);
    }

    val = m_buffer[m_currReadPos++];
}

void YapPacket::operator>>(int16_t& val)
{
    g_return_if_fail(!m_forWriting);
    g_return_if_fail((m_currReadPos + 3) <= m_readTotalLen);

    YapType_t type = (YapType_t) m_buffer[m_currReadPos++];
    if (type != kYapTypeInt16) {
        fprintf(stderr, "Arg type is not short: %d\n", type);
        g_return_if_fail(false);
    }

    uint8_t* pSrc = m_buffer + m_currReadPos;
    uint8_t* pDst = (uint8_t*)(&val);

    pDst[0] = pSrc[1];
    pDst[1] = pSrc[0];
    m_currReadPos += 2;
}

void YapPacket::operator>>(uint16_t& val)
{
    g_return_if_fail(!m_forWriting);
    g_return_if_fail((m_currReadPos + 3) <= m_readTotalLen);

    YapType_t type = (YapType_t) m_buffer[m_currReadPos++];
    if (type != kYapTypeUInt16) {
        fprintf(stderr, "Arg type is not unsigned short: %d\n", type);
        g_return_if_fail(false);
    }

    const uint8_t* pSrc = m_buffer + m_currReadPos;
    uint8_t* pDst = (uint8_t*)(&val);

    pDst[0] = pSrc[1];
    pDst[1] = pSrc[0];
    m_currReadPos += sizeof(val);
}

void YapPacket::operator>>(int32_t& val)
{
    g_return_if_fail(!m_forWriting);
    g_return_if_fail((m_currReadPos + 5) <= m_readTotalLen);

    YapType_t type = (YapType_t) m_buffer[m_currReadPos++];
    if (type != kYapTypeInt32) {
        fprintf(stderr, "Arg type is not int: %d\n", type);
        g_return_if_fail(false);
    }

    uint8_t* pSrc = m_buffer + m_currReadPos;
    uint8_t* pDst = (uint8_t*)(&val);

    pDst[0] = pSrc[3];
    pDst[1] = pSrc[2];
    pDst[2] = pSrc[1];
    pDst[3] = pSrc[0];
    m_currReadPos += 4;
}

void YapPacket::operator>>(int64_t& val)
{
    g_return_if_fail(!m_forWriting);
    g_return_if_fail((m_currReadPos + 9) <= m_readTotalLen);

    YapType_t type = (YapType_t) m_buffer[m_currReadPos++];
    if (type != kYapTypeInt64) {
        fprintf(stderr, "Arg type is not long: %d\n", type);
        g_return_if_fail(false);
    }

    uint8_t* pSrc = m_buffer + m_currReadPos;
    uint8_t* pDst = (uint8_t*)(&val);

    pDst[0] = pSrc[7];
    pDst[1] = pSrc[6];
    pDst[2] = pSrc[5];
    pDst[3] = pSrc[4];
    pDst[4] = pSrc[3];
    pDst[5] = pSrc[2];
    pDst[6] = pSrc[1];
    pDst[7] = pSrc[0];
    m_currReadPos += 8;
}

void YapPacket::operator>>(double& val)
{
    g_return_if_fail(!m_forWriting);
    g_return_if_fail((m_currReadPos + 9) <= m_readTotalLen);

    YapType_t type = (YapType_t) m_buffer[m_currReadPos++];
    if (type != kYapTypeDouble) {
        fprintf(stderr, "Arg type is not double: %d\n", type);
        g_return_if_fail(false);
    }

    uint8_t* pSrc = m_buffer + m_currReadPos;
    uint8_t* pDst = (uint8_t*)(&val);

    pDst[0] = pSrc[7];
    pDst[1] = pSrc[6];
    pDst[2] = pSrc[5];
    pDst[3] = pSrc[4];
    pDst[4] = pSrc[3];
    pDst[5] = pSrc[2];
    pDst[6] = pSrc[1];
    pDst[7] = pSrc[0];
    m_currReadPos += 8;
}

// Jihad (inspector 2026-10-07, P1): a string field that cannot be read leaves the caller with
// "" — never NULL. Every handler passes what it read straight on (STRINGZ_TO_NPVARIANT does
// strlen), so one malformed or truncated message must not become a NULL in the card's host.
static void jihadYapEmptyString(char*& val)
{
    if (!val) val = (char*) calloc(1, 1);
}

void YapPacket::operator>>(char*& val)
{
    if (m_forWriting || (m_currReadPos + 3) > m_readTotalLen) { jihadYapEmptyString(val); }
    g_return_if_fail(!m_forWriting);
    g_return_if_fail((m_currReadPos + 3) <= m_readTotalLen);

    YapType_t type = (YapType_t) m_buffer[m_currReadPos++];
    if (type != kYapTypeString) {
        fprintf(stderr, "Arg type is not string: %d\n", type);
        jihadYapEmptyString(val);
        g_return_if_fail(false);
    }

    // Jihad hardening (Codex P0): use an UNSIGNED length so a wire value like
    // 0xffff can't become negative, pass the bounds check, and blow up malloc/
    // memcpy as a huge size_t. Bound the read against the packet length.
    uint16_t strLen = 0;

    uint8_t* pSrc = m_buffer + m_currReadPos;
    uint8_t* pDst = (uint8_t*)(&strLen);

    pDst[0] = pSrc[1];
    pDst[1] = pSrc[0];
    m_currReadPos += 2;

    if ((size_t)(m_currReadPos + strLen) > (size_t)m_readTotalLen) { jihadYapEmptyString(val); }
    g_return_if_fail((size_t)(m_currReadPos + strLen) <= (size_t)m_readTotalLen);
    char* strBuf = (char*) malloc((size_t)strLen + 1);
    if (!strBuf) { jihadYapEmptyString(val); }
    g_return_if_fail(strBuf != NULL);   // on OOM the out-param is "" if calloc(1,1) managed, else unchanged
    val = strBuf;

    memcpy(val, m_buffer + m_currReadPos, strLen);
    val[strLen] = 0;

    m_currReadPos += strLen;
}

