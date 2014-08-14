/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Samsung Electronics
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <arpa/inet.h>
#include <sys/time.h>
#include "common.h"
#include "socket.h"

const int MESSAGE_EVENT_TYPE_BIT = 0x00;
const int CMESSAGE_PRESENT_BIT = 0x01;

const int CLIENT_ID_OFFSET = 0;
const int SIZE_OFFSET = 6;
const int OPCODE_OFFSET = 4;
const int PAYLOAD_OFFSET = 8;

struct WlaMessageBufferHeader
{
    int serializeToBuf(char *buf, size_t size) const
    {
        if (size < getSerializeSize())
        {
            DEBUG_LOG("buffer too small");
            return -1;
        }

        uint32_t nbo_f = htonl(flags);
        memcpy(buf, &nbo_f, sizeof(nbo_f));
        buf += sizeof(nbo_f);

        uint32_t nbo_ts = htonl(timestamp.tv_sec);
        memcpy(buf, &nbo_ts, sizeof(nbo_ts));
        buf += sizeof(nbo_ts);
        uint32_t nbo_tus = htonl(timestamp.tv_usec);
        memcpy(buf, &nbo_tus, sizeof(nbo_tus));
        buf += sizeof(nbo_tus);

        uint32_t nbo_mlen = htonl(msg_len);
        memcpy(buf, &nbo_mlen, sizeof(nbo_mlen));
        buf += sizeof(nbo_mlen);

        uint32_t nbo_cmlen = htonl(cmsg_len);
        memcpy(buf, &nbo_cmlen, sizeof(nbo_cmlen));

        return getSerializeSize();
    }

    void deserializeFromBuf(const char *buf, size_t size)
    {
        if (size < getSerializeSize())
        {
            DEBUG_LOG("buffer too small");
            return;
        }

        uint32_t hbo_f = byteArrToUInt32(buf);
        buf += sizeof(hbo_f);
        flags = ntohl(hbo_f);

        uint32_t hbo_ts = byteArrToUInt32(buf);
        buf += sizeof(hbo_ts);
        timestamp.tv_sec = ntohl(hbo_ts);
        uint32_t hbo_tus = byteArrToUInt32(buf);
        buf += sizeof(hbo_tus);
        timestamp.tv_usec = ntohl(hbo_tus);

        uint32_t hbo_mlen = byteArrToUInt32(buf);
        buf += sizeof (hbo_mlen);
        msg_len = ntohl(hbo_mlen);

        uint32_t hbo_cmlen = byteArrToUInt32(buf);
        cmsg_len = ntohl(hbo_cmlen);
    }

    static size_t getSerializeSize()
    {
        return sizeof(flags) + 2 * sizeof(uint32_t)
                + sizeof(msg_len) + sizeof(cmsg_len);
    }

    uint32_t flags;
    timeval timestamp;
    uint32_t msg_len;
    uint32_t cmsg_len;
};

class WlaMessageBuffer
{
public:
    enum MESSAGE_TYPE
    {
        REQUEST_TYPE,
        EVENT_TYPE
    };

    WlaMessageBuffer();
    ~WlaMessageBuffer();

    int sendMessage(WldSocket &socket);
    int receiveMessage(WldSocket &socket);

    void setHeader(const WlaMessageBufferHeader *hdr);
    WlaMessageBufferHeader *getHeader() { return &hdr; }

    void setType(MESSAGE_TYPE type);
    MESSAGE_TYPE getType() const;
    const timeval *getTimeStamp() const { return &hdr.timestamp; }

    uint32_t getMsgSize() const { return hdr.msg_len; }
    void setMsg(const char *msg, int size);
    const char *getMsg() const { return buf; }

    uint32_t getControlMsgSize() const { return hdr.cmsg_len; }
    void setControlMsg(const char *cmsg, int size);
    const char *getControlMsg() const { return cmsg; }

private:
    static const int MAX_BUF_SIZE = 4096;
    static const int MAX_FDS = 28;

    WlaMessageBufferHeader hdr;

    char buf[MAX_BUF_SIZE];

    msghdr msg;
    char cmsg[CMSG_LEN(MAX_FDS * sizeof(int))];
    iovec iov;
};

#endif // MESSAGE_H
