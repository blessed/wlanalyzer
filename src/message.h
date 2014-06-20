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

    void setType(MESSAGE_TYPE type) { msgType = type; }
    MESSAGE_TYPE getType() const { return msgType; }
    const timeval *getTimeStamp() const { return &timestamp; }

    uint32_t getMsgSize() const { return msgSize; }
    const char *getMsg() const { return buf; }

    uint32_t getControlMsgSize() const { return msg.msg_controllen; }
    const char *getControlMsg() const { return cmsg; }

    uint32_t clientID() const;
    uint16_t opcode() const;
    uint16_t size() const;

private:
    static const int MAX_BUF_SIZE = 4096;
    static const int MAX_FDS = 28;
    static const int CLIENT_ID_OFFSET = 0;
    static const int MSG_SIZE_OFFSET = 4;
    static const int OPCODE_OFFSET = 6;
    bool received;

    char buf[MAX_BUF_SIZE];
    int msgSize;
    MESSAGE_TYPE msgType;
    timeval timestamp;

    msghdr msg;
    char cmsg[CMSG_LEN(MAX_FDS * sizeof(int))];
    iovec iov;
};

#endif // MESSAGE_H
