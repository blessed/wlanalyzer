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
#include "server_socket.h"

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <fcntl.h>

using namespace std;

namespace WlAnalyzer {

WlaServerSocket::WlaServerSocket() : WlaSocketBase(), _maxPendingConnections(1), _serverName(""),
    _listening(false)
{
}

WlaServerSocket::~WlaServerSocket()
{
    closeServer();
}

bool WlaServerSocket::listen(const string &name)
{
    if (name.empty())
    {
        wld_log("empty server name\n");
        return false;
    }

    if (isListening())
    {
        wld_log("server is already listening\n");
        return false;
    }

    if (!bind(name))
    {
        ::close(_fd);
        return false;
    }

    ::listen(_fd, _maxPendingConnections);

    DEBUG_LOG("listening on %s", name.c_str());

    _serverName = name;
    _listening = true;

    return true;
}

bool WlaServerSocket::waitForConnection(int ms, bool *timedout)
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(_fd, &readfds);

    timeval timeout;
    timeout.tv_sec = ms / 1000;
    timeout.tv_usec = (ms % 1000) * 1000;

    DEBUG_LOG("");

    timeval *arg = (ms != 0) ? &timeout : NULL;
    while (select(FD_SETSIZE, &readfds, NULL, NULL, arg) == -1)
    {
        DEBUG_LOG("");

        if (errno == EINTR)
        {
            DEBUG_LOG("EINTR\n");
            continue;
        }
        else if (errno == ETIMEDOUT)
        {
            *timedout = true;
            return false;
        }
    }

    DEBUG_LOG("");
    onNewConnection();

    *timedout = false;

    if (_pendingConnections.empty())
        return false;

    DEBUG_LOG("");

    return true;
}

WlaClientSocket *WlaServerSocket::nextPendingConnection()
{
    if (_pendingConnections.empty())
        return NULL;

    WlaClientSocket *socket = _pendingConnections.back();
    _pendingConnections.pop();

    return socket;
}

void WlaServerSocket::close()
{
    closeServer();
}

bool WlaServerSocket::bind(const string &resource)
{
    int err;

    _fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (_fd == -1)
    {
        wld_log("socket() failed\n");
        return false;
    }

    int val = 1;
    setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof val);

    sockaddr_un addr;
    memset(&addr, 0, sizeof(sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, resource.c_str(), resource.length());

    err = ::bind(_fd, (sockaddr *)&addr, sizeof(sockaddr_un));
    if (err)
    {
        wld_log("bind() failed\n");
        perror(NULL);
        return false;
    }

    return true;
}

void WlaServerSocket::onNewConnection()
{
    int clientSocket;
    sockaddr_un addr;
    socklen_t socklen;

    DEBUG_LOG("new connection");

    socklen = sizeof(sockaddr_un);
    clientSocket = accept(_fd, (sockaddr *)&addr, &socklen);
    if (clientSocket == -1)
        return;

    WlaClientSocket *localSocket = new WlaClientSocket;
    localSocket->setFd(clientSocket);
    _pendingConnections.push(localSocket);
}

void WlaServerSocket::closeServer()
{
    if (!isListening())
        return;

    clearPendingConnections();

    ::close(_fd);
    unlink(_serverName.c_str());

    DEBUG_LOG("server closed");

    _listening = false;
    _serverName.clear();
}

void WlaServerSocket::clearPendingConnections()
{
    if (_pendingConnections.empty())
        return;

    WlaClientSocket *temp;

    while (!_pendingConnections.empty())
    {
        temp = _pendingConnections.front();
        _pendingConnections.pop();
        delete temp;
    }
}

bool WlaNetServerSocket::bind(const string &resource)
{
    int err;

    _fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1)
    {
        wld_log("socket() failed\n");
        return false;
    }

    int val = 1;
    setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof val);

    DEBUG_LOG("");

    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(resource.c_str()));

    err = ::bind(getFd(), (sockaddr *)&addr, sizeof(sockaddr_in));
    if (err == -1)
    {
        wld_log("bind() %d to %s failed\n", getFd(), resource.c_str());
        DEBUG_LOG("%d", atoi(resource.c_str()));
        perror(NULL);
        return false;
    }

    return true;
}

void WlaNetServerSocket::onNewConnection()
{
    int clientSocket;
    sockaddr_in addr;
    socklen_t socklen;

    DEBUG_LOG("new connection");

    socklen = sizeof(sockaddr_in);
    clientSocket = accept(_fd, (sockaddr *)&addr, &socklen);
    if (clientSocket == -1)
        return;

    WlaClientSocket *localSocket = new WlaNetClientSocket;
    localSocket->setFd(clientSocket);
    _pendingConnections.push(localSocket);

    DEBUG_LOG("");
}

} // WlAnalyzer
