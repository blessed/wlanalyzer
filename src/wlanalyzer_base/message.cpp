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
#include <cassert>

WlaMessageBuffer::WlaMessageBuffer()
{
    hdr.flags = 0;
    hdr.msg_len = 0;
    hdr.cmsg_len = 0;
    hdr.timestamp.tv_sec = 0;
    hdr.timestamp.tv_usec = 0;
}

WlaMessageBuffer::~WlaMessageBuffer()
{
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


WldMessageBufferSocket::WldMessageBufferSocket()
{
}

WldMessageBufferSocket::~WldMessageBufferSocket()
{
}

WlaMessageBuffer *WldMessageBufferSocket::receiveMessage(WldSocket &socket)
{
    WlaMessageBuffer *msgbuffer = new WlaMessageBuffer;
    if (!msgbuffer)
    {
        Logger::getInstance()->log("Error: Memory allocation failed\n");
        return NULL;
    }

    iovec iov;
    iov.iov_base = msgbuffer->getMsg();
    iov.iov_len = MAX_BUF_SIZE;

    msghdr msg;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = msgbuffer->getControlMsg();
    msg.msg_controllen = CMSG_LEN(MAX_FDS * sizeof(int));

    int len = socket.readMsg(&msg);
    if (len < 0)
    {
        DEBUG_LOG("Failed to read msg");
        perror(NULL);
        delete msgbuffer;
        return NULL;
    }
    else if (len > 0)
    {
        WlaMessageBufferHeader *hdr = msgbuffer->getHeader();
        gettimeofday(&hdr->timestamp, NULL);
        hdr->msg_len = len;
        hdr->cmsg_len = msg.msg_controllen;
        if (hdr->cmsg_len > 0)
            set_bit(&hdr->flags, CMESSAGE_PRESENT_BIT, true);
    }
    else
    {
        delete msgbuffer;
        return NULL;
    }

    return msgbuffer;
}

bool WldMessageBufferSocket::sendMessage(WlaMessageBuffer *msgbuffer, WldSocket &socket)
{
    iovec iov;
    iov.iov_base = msgbuffer->getMsg();
    iov.iov_len = msgbuffer->getMsgSize();

    msghdr msg;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = msgbuffer->getControlMsg();
    msg.msg_controllen = msgbuffer->getControlMsgSize();

    int len = socket.writeMsg(&msg);
    if (len < 0)
    {
        DEBUG_LOG("Error: Failed to send message\n");
        perror(NULL);
        return false;
    }

    return true;
}
