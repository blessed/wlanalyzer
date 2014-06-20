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

#include "message.h"

WlaMessageBuffer::WlaMessageBuffer()
{
    msgSize = 0;

    iov.iov_base = buf;
    iov.iov_len = MAX_BUF_SIZE;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsg;
    msg.msg_controllen = CMSG_LEN(MAX_FDS * sizeof(int));

    msgType = REQUEST_TYPE;
}

WlaMessageBuffer::~WlaMessageBuffer()
{
}

int WlaMessageBuffer::sendMessage(UnixLocalSocket &socket)
{
    if (msgSize == 0)
    {
        DEBUG_LOG("message is size 0");
        return -1;
    }

    iov.iov_len = msgSize;

    int len = socket.writeMsg(&msg);
    if (len < 0)
    {
        DEBUG_LOG("failed to write message");
        perror(NULL);
    }

    return len;
}

int WlaMessageBuffer::receiveMessage(UnixLocalSocket &socket)
{
    iov.iov_len = MAX_BUF_SIZE;
    int len = socket.readMsg(&msg);
    if (len < 0)
    {
        DEBUG_LOG("failed to read message");
        perror(NULL);
        msgSize = 0;
    }
    else if (len > 0)
    {
        gettimeofday(&timestamp, NULL);
        msgSize = len;
    }

    return len;
}

uint32_t WlaMessageBuffer::clientID() const
{
    if (!msgSize)
        return -1;

    uint32_t id = 0;
    const char *c_id = buf;
    for (int i = 0; i < 4; ++i)
    {
        id |= c_id[i] << i * 8;
    }

    return id;
}

uint16_t WlaMessageBuffer::opcode() const
{
    if (!msgSize)
        return -1;

    uint16_t code = 0;
    const char *arr = buf + OPCODE_OFFSET;
    for (int i = 0; i < 2; ++i)
    {
        code |= arr[i] << i * 8;
    }

    return code;
}

uint16_t WlaMessageBuffer::size() const
{
    if (!msgSize)
        return -1;

    uint16_t size = 0;
    const char *arr = buf + MSG_SIZE_OFFSET;
    for (int i = 0; i < 2; ++i)
    {
        size |= arr[i] << i * 8;
    }

    return size;
}
