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

class WldSocket : public ev::io
{
public:
    WldSocket();
    WldSocket(const WldSocket &copy);
    virtual ~WldSocket();

    SocketError connectToServer(const char *path);
    SocketError connectToServer(const std::string &path);
    SocketError disconnectFromServer();

    void setSocketDescriptor(int fd, int flags = 0);
    int getSocketDescriptor() const
    {
        if (isConnected())
            return _fd;

        return -1;
    }

    void start(int eventMask);

    bool isConnected() const { return _connected; }

    WldSocket &operator=(const WldSocket &copy);

    bool operator==(int fd) { return fd == _fd; }
    friend bool operator==(int fd, const WldSocket &sock);

    long read(char *data, long max_size) const;
    bool write(const char *data, size_t c) const;

    size_t readUntil(char *data, size_t max_size) const;
    bool writeUntil(const char *data, size_t c) const;

    int readMsg(msghdr *msg);
    int writeMsg(const msghdr *msg);

protected:
    virtual SocketError connect(const std::string &path);

private:
    void shutdown();

protected:
    int _fd;

private:
//    int _fd;
    bool _connected;
};
bool operator==(int fd, const WldSocket &sock);

class WldNetSocket : public WldSocket
{
public:
    WldNetSocket() { }
    WldNetSocket(const WldNetSocket &copy);
    virtual ~WldNetSocket() { }

protected:
    SocketError connect(const std::string &path);
};

#endif // SOCKET_H
