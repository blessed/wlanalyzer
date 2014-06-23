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

#include <sys/time.h>
#include "common.h"
#include "socket.h"

const int MESSAGE_EVENT_TYPE_BIT = 0x00;
const int CMESSAGE_PRESENT_BIT = 0x01;

struct WlaMessageBufferHeader
{
    WlaMessageBufferHeader() : flags(0)
    {
        timestamp.tv_sec = 0;
        timestamp.tv_usec = 0;
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

    int sendMessage(UnixLocalSocket &socket);
    int receiveMessage(UnixLocalSocket &socket);

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
