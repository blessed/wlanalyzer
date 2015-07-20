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

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include "socket.h"
#include "common.h"

namespace WlAnalyzer {

WlaSocketBase::WlaSocketBase() : _fd(-1)
{
}

WlaSocketBase::~WlaSocketBase()
{
}

WlaSocketBase &WlaSocketBase::operator=(const WlaSocketBase &other)
{
    if (other._fd != -1)
        _fd = dup(_fd);

    return *this;
}

void WlaSocketBase::start(int eventMask)
{
    _watcher.start(_fd, eventMask);
}

void WlaSocketBase::stop()
{
    _watcher.stop();
}

bool operator==(int fd, const WlaSocketBase &socket)
{
    return fd == socket.getFd();
}

} // WlAnalyzer
