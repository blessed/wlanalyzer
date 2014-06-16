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

#include "common.h"
#include "message.h"

WlaMessage::WlaMessage()
{
    size = 0;

    iov.iov_base = buf;
    iov.iov_len = MAX_BUF_SIZE;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsg;
    msg.msg_controllen = CMSG_LEN(MAX_FDS * sizeof(int));
}

WlaMessage::~WlaMessage()
{
}

const msghdr *WlaMessage::getMsg()
{
    return &msg;
}

int WlaMessage::sendMessage(UnixLocalSocket &socket)
{
    iov.iov_len = size;

    int len = socket.writeMsg(&msg);
    if (len < 0)
    {
        DEBUG_LOG("failed to write message");
        perror(NULL);
    }
    else if (len > 0)
    {
        DEBUG_LOG("wrote %d", len);
    }

    return len;
}

int WlaMessage::receiveMessage(UnixLocalSocket &socket)
{
    iov.iov_len = MAX_BUF_SIZE;
    int len = socket.readMsg(&msg);
    if (len < 0)
    {
        DEBUG_LOG("failed to read message");
        perror(NULL);
        size = 0;
    }
    else if (len > 0)
    {
        DEBUG_LOG("read %d", len);
        size = len;
    }

    return len;
}
