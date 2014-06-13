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

enum UnixLocalSocketError
{
    ConnectionRefusedError,
    ServerNotFoundError,
    ConnectionError,
    SocketTimeoutError,
    SocketResourceError,
    UnsupportedSocketOperationError,
    UnknownSocketError,
    NoError
};

class UnixLocalSocket
{
public:
    UnixLocalSocket();
    ~UnixLocalSocket();

    UnixLocalSocketError connectToServer(const char *path);
    UnixLocalSocketError connectToServer(const std::string &path);
    UnixLocalSocketError disconnectFromServer();

    void setSocketDescriptor(int fd, int flags = 0);
    int getSocketDescriptor() const
    {
        if (isConnected())
            return _fd;

        return -1;
    }

    bool isConnected() const { return _connected; }
    bool operator==(int fd) { return fd == _fd; }

    long read(char *data, long max_size) const;
    bool write(const char *data, long c) const;

    msghdr *readMessage(char *buf, int size);
    int writeMessage(const msghdr *msg);

private:
    void shutdown();

private:
    int _fd;
    std::string _path;
    bool _connected;
};

#endif // SOCKET_H
