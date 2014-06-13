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


#include <string>
#include "common.h"
#include "proxy.h"

WlaProxyServer::WlaProxyServer()
{
}

WlaProxyServer::~WlaProxyServer()
{
    closeServer();
}

int WlaProxyServer::init(const std::string &socketPath)
{
    if (_serverSocket.isListening())
    {
        LOGGER_LOG("Server is listening already");
        return -1;
    }

    std::string fullPath = std::string(getenv("XDG_RUNTIME_DIR")) +
            "/" + socketPath;
    if (!_serverSocket.listen(fullPath))
    {
        LOGGER_LOG("Failed to listen on %s", socketPath.c_str());
        return -1;
    }

    _io.set<WlaProxyServer, &WlaProxyServer::connectClient>(this);
    _io.start(_serverSocket.getFd(), EV_READ);
}

int WlaProxyServer::openServer()
{
    _loop.run();
}

void WlaProxyServer::closeServer()
{
    // TODO: close all connections

    for (std::vector<WlaConnection *>::iterator it = _connections.begin();
         it != _connections.end(); ++it)
    {
        (*it)->close();
        delete *it;
    }

    _io.stop();

    if (_serverSocket.isListening())
        _serverSocket.close();
}

void WlaProxyServer::connectClient(ev::io &watcher, int revents)
{
    if (revents & EV_ERROR)
    {
        LOGGER_LOG("got invalid event");
        _io.stop();
        return;
    }

    UnixLocalSocket *wayland = new UnixLocalSocket;
    std::string waylandPath = std::string(getenv("XDG_RUNTIME_DIR")) +
            "/" + std::string(getenv("WAYLAND_DISPLAY"));
    if (wayland->connectToServer(waylandPath) != NoError)
    {
        LOGGER_LOG("failed to connect to server");
        return;
    }

    sockaddr_un addr;
    socklen_t addrlen = sizeof(sockaddr_un);
    int fd = accept(watcher.fd, (sockaddr *)&addr, &addrlen);
    if (fd == -1)
    {
        LOGGER_LOG("failed to accept connection");
        return;
    }

    UnixLocalSocket *client = new UnixLocalSocket;
    client->setSocketDescriptor(fd);

    WlaConnection *connection = new WlaConnection(client, wayland);
    if (!connection)
    {
        LOGGER_LOG("Failed to create connection between client and compositor");
        delete connection;
        return;
    }

    _connections.push_back(connection);
}
