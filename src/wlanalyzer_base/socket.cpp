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

WldSocket::WldSocket() : _fd(-1), _connected(false)
{
}

WldSocket::WldSocket(const WldSocket &copy)
{
    if (copy.isConnected())
    {
        _fd = dup(copy._fd);
        _connected = copy.isConnected();
    }
}

WldSocket::~WldSocket()
{
    if (isConnected())
    {
        DEBUG_LOG("Closed socket %d", _fd);
        disconnectFromServer();
    }
}

SocketError WldSocket::connectToServer(const char *path)
{
    std::string strPath(path);
    return connectToServer(strPath);
}

SocketError WldSocket::connectToServer(const std::string &path)
{
//    int err;

    if (path.empty())
        return ServerNotFoundError;

    if (isConnected())
        disconnectFromServer();

    SocketError err = connect(path);
    if (err != NoError)
    {
        disconnectFromServer();
        return err;
    }

//    sockaddr_un address;
//    memset(&address, 0, sizeof(sockaddr_un));
//    address.sun_family = AF_UNIX;
//    strncpy(address.sun_path, path.c_str(), path.size());
//    err = ::connect(_fd, (sockaddr *)&address, sizeof(sockaddr_un));
//    if (err == -1)
//    {
//        disconnectFromServer();

//        switch (errno)
//        {
//        case EINVAL:
//        case ECONNREFUSED:
//            return ConnectionRefusedError;
//        case ENOENT:
//            return ServerNotFoundError;
//        case ETIMEDOUT:
//            return SocketTimeoutError;

//        default:
//            return UnknownSocketError;
//        }
//    }

    Logger::getInstance()->log("Connected to %s\n", path.c_str());
    _connected = true;

    return NoError;
}

SocketError WldSocket::disconnectFromServer()
{
    if (_connected)
    {
        close(_fd);
        _fd = -1;

        _connected = false;
    }

    return NoError;
}

void WldSocket::setSocketDescriptor(int fd, int flags)
{
    if (isConnected())
        disconnectFromServer();

    if (flags)
    {
        fcntl(fd, F_SETFL, flags);
    }

    _fd = fd;
    _connected = true;
}

void WldSocket::start(int eventMask)
{
    ev::io::start(_fd, eventMask);
}

WldSocket &WldSocket::operator=(const WldSocket &copy)
{
    _fd = dup(copy._fd);
    _connected = copy._connected;

    return *this;
}

bool operator==(int fd, const WldSocket &sock)
{
    return fd == sock.getSocketDescriptor();
}

long WldSocket::read(char *data, long max_size) const
{
    if (!isConnected())
        return -1;

    int err;
    do
    {
		err = recv(_fd, data, max_size, MSG_WAITALL);
	} while (err == -1 && (errno == EWOULDBLOCK || errno == EAGAIN));

    return err;
}

bool WldSocket::write(const char *data, size_t c) const
{
    while (c > 0)
    {
		int wrote = send(_fd, data, c, MSG_NOSIGNAL);
        if (wrote == -1)
            return false;

        c -= wrote;
    }

    return true;
}

size_t WldSocket::readUntil(char *data, size_t max_size) const
{
    size_t sz = 0;
    int r;

    while (max_size > 0)
    {
        r = read(data, max_size);
        if (r <= 0)
            return r;

        data += r;
        sz += r;
        max_size -= r;
    }

    return sz;
}

bool WldSocket::writeUntil(const char *data, size_t c) const
{
    size_t sz = 0;

    do
    {
        int ret;
        ret = write(data + sz, c);
        if (ret <= 0)
            return false;

        sz += ret;
    } while (sz < c);

    return true;
}

int WldSocket::readMsg(msghdr *msg)
{
    int len;
    do
    {
        len = recvmsg(_fd, msg, 0);
    } while (len < 0 && errno == EINTR);

    return len;
}

int WldSocket::writeMsg(const msghdr *msg)
{
    int len;
    do
    {
        len = sendmsg(_fd, msg, 0);
    } while (len < 0 && errno == EINTR);

    return len;
}

SocketError WldSocket::connect(const std::string &path)
{
    DEBUG_LOG("");

    _fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (check_error(_fd))
        return SocketResourceError;

    sockaddr_un address;
    memset(&address, 0, sizeof(sockaddr_un));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, path.c_str(), path.size());
    int err = ::connect(_fd, (sockaddr *)&address, sizeof(sockaddr_un));
    if (err == -1)
    {
        disconnectFromServer();

        switch (errno)
        {
        case EINVAL:
        case ECONNREFUSED:
            return ConnectionRefusedError;
        case ENOENT:
            return ServerNotFoundError;
        case ETIMEDOUT:
            return SocketTimeoutError;

        default:
            return UnknownSocketError;
        }
    }

    return NoError;
}


WldNetSocket::WldNetSocket(const WldNetSocket &copy) : WldSocket(copy)
{
}

SocketError WldNetSocket::connect(const std::string &path)
{
    DEBUG_LOG("");

    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (check_error(_fd))
        return SocketResourceError;

    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));

    size_t idx = path.find(':');
    std::string ip = path.substr(0, idx);
    std::string port = path.substr(idx + 1);
    if (port.empty())
        return InvalidServerAddress;


    addr.sin_port = htons(atoi(port.c_str()));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    int err = ::connect(_fd ,(sockaddr *)&addr, sizeof(sockaddr_in));
    if (err == -1)
    {
        disconnectFromServer();

        DEBUG_LOG("%d", errno);

        switch (errno)
        {
        case EINVAL:
        case ECONNREFUSED:
            return ConnectionRefusedError;
        case ENOENT:
            return ServerNotFoundError;
        case ETIMEDOUT:
            return SocketTimeoutError;

        default:
            return UnknownSocketError;
        }
    }

    return NoError;
}
