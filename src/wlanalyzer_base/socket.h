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


#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <sys/un.h>
#include <string>
#include <ev++.h>

namespace WlAnalyzer {

enum SocketDomain
{
    UNIX_DOMAIN = AF_UNIX,
    NET4_DOMAIN = AF_INET,
    NET6_DOMAIN = AF_INET6
};

enum SocketError
{
    ConnectionRefusedError,
    ServerNotFoundError,
    ConnectionError,
    SocketTimeoutError,
    SocketResourceError,
    UnsupportedSocketOperationError,
    UnknownSocketError,
    InvalidServerAddress,
    NoError
};

class WlaSocketBase
{
public:
    WlaSocketBase();
    virtual ~WlaSocketBase();

    WlaSocketBase &operator=(const WlaSocketBase &other);

    template<class T, void (T::*method)(ev::io &w, int)>
    void set(T *object)
    {
        _watcher.set<T, method>(object);
    }

    void start(int eventMask);
    void stop();

    int getFd() const { return _fd; }

    bool operator==(int fd) { return _fd == fd; }
    bool operator==(const ev::io &watcher) { return _fd == watcher.fd; }
    friend bool operator==(int fd, const WlaSocketBase &socket);

protected:
    int _fd;

private:
    ev::io _watcher;
};

bool operator==(int fd, const WlaSocketBase &socket);

} // WlAnalyzer

#endif // SOCKET_H
