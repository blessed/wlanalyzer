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
    iov.iov_base = buf;
    iov.iov_len = MAX_BUF_SIZE;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsg;
    msg.msg_controllen = CMSG_LEN(MAX_FDS * sizeof(int));

    hdr.flags = 0;
    hdr.msg_len = 0;
    hdr.cmsg_len = 0;
    hdr.timestamp.tv_sec = 0;
    hdr.timestamp.tv_usec = 0;
}

WlaMessageBuffer::~WlaMessageBuffer()
{
}

int WlaMessageBuffer::sendMessage(UnixLocalSocket &socket)
{
    if (hdr.msg_len == 0)
    {
        DEBUG_LOG("message is size 0");
        return -1;
    }

    iov.iov_len = hdr.msg_len;

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
        hdr.msg_len = 0;
    }
    else if (len > 0)
    {
        gettimeofday(&hdr.timestamp, NULL);
        hdr.msg_len = len;
        hdr.cmsg_len = msg.msg_controllen;
        if (hdr.cmsg_len > 0)
            set_bit(&hdr.flags, CMESSAGE_PRESENT_BIT, true);
    }

    return len;
}

void WlaMessageBuffer::setHeader(const WlaMessageBufferHeader *hdr)
{
    memcpy(&hdr, hdr, sizeof(WlaMessageBufferHeader));
}

void WlaMessageBuffer::setType(WlaMessageBuffer::MESSAGE_TYPE type)
{
    if (type == EVENT_TYPE)
        set_bit(&hdr.flags, MESSAGE_EVENT_TYPE_BIT, true);
    else
        set_bit(&hdr.flags, MESSAGE_EVENT_TYPE_BIT, false);
}

WlaMessageBuffer::MESSAGE_TYPE WlaMessageBuffer::getType() const
{
    if (bit_isset(hdr.flags, MESSAGE_EVENT_TYPE_BIT))
        return EVENT_TYPE;
    else
        return REQUEST_TYPE;
}

void WlaMessageBuffer::setMsg(const char *msg, int size)
{
    if (size > MAX_BUF_SIZE)
    {
        DEBUG_LOG("message too big");
        return;
    }

    memcpy(buf, msg, size);
}

void WlaMessageBuffer::setControlMsg(const char *cmsg, int size)
{
    memcpy(this->cmsg, cmsg, size);
}
