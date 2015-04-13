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

using namespace WlAnalyzer;

WlaProxyServer::WlaProxyServer() : _loop(EVBACKEND_SELECT), dumper(NULL), parser(NULL)
{
    DEBUG_LOG("_loop.backend %d", _loop.backend());
}

WlaProxyServer::~WlaProxyServer()
{
    stopServer();
    setDumper(NULL);
}

int WlaProxyServer::init(const std::string &socketPath)
{
    if (_serverSocket.isListening())
    {
        DEBUG_LOG("Server is listening already");
        return -1;
    }

    std::string fullPath = std::string(getenv("XDG_RUNTIME_DIR")) +
            "/" + socketPath;
    if (!_serverSocket.listen(fullPath))
    {
        DEBUG_LOG("Failed to listen on %s", socketPath.c_str());
        return -1;
    }

    _io.set<WlaProxyServer, &WlaProxyServer::connectClient>(this);
    _io.start(_serverSocket.getFd(), EV_READ);

    return 0;
}

int WlaProxyServer::startServer()
{
    _loop.run();

    return 0;
}

void WlaProxyServer::stopServer()
{
    _io.stop();

    if (_serverSocket.isListening())
        _serverSocket.close();

    if (parser)
        parser->parse();

    std::set<WlaConnection *>::const_iterator it = _connections.begin();
    for (; it != _connections.end(); it++)
    {
        (*it)->closeConnection();
    }

    _loop.break_loop();
}

void WlaProxyServer::closeConnection(WlaConnection *conn)
{
    _connections.erase(conn);

    if (_connections.empty())
        stopServer();
}

void WlaProxyServer::setDumper(WldMessageSink *dumper)
{
    std::set<WlaConnection *>::const_iterator it = _connections.begin();
    for (; it != _connections.end(); it++)
    {
        (*it)->setDumper(dumper);
    }

    if (this->dumper)
        delete this->dumper;

    this->dumper = dumper;
}

void WlaProxyServer::setParser(WldParser *parser)
{
    if (this->parser)
        delete this->parser;

    this->parser = parser;
    this->parser->enable(true);
}

void WlaProxyServer::connectClient(ev::io &watcher, int revents)
{
    if (revents & EV_ERROR)
    {
        DEBUG_LOG("got invalid event");
        _io.stop();
        return;
    }

    WldSocket wayland;
    std::string waylandPath = std::string(getenv("XDG_RUNTIME_DIR")) +
            "/" + std::string(getenv("WAYLAND_DISPLAY"));
    if (wayland.connectToServer(waylandPath) != NoError)
    {
        DEBUG_LOG("failed to connect to server");
        stopServer();
        return;
    }

    sockaddr_un addr;
    socklen_t addrlen = sizeof(sockaddr_un);
    int fd = accept(watcher.fd, (sockaddr *)&addr, &addrlen);
    if (fd == -1)
    {
        DEBUG_LOG("failed to accept connection");
        stopServer();
        return;
    }

    WldSocket client;
    client.setSocketDescriptor(fd);

    WlaConnection *connection = new WlaConnection(this, dumper);
    if (!connection)
    {
        DEBUG_LOG("Failed to create connection between client and compositor");
        stopServer();
        return;
    }
    connection->setSink(sink_);
    connection->createConnection(client, wayland);

    _connections.insert(connection);
}

void WlaProxyServer::setSink(const shared_ptr<RawMessageSink> &sink)
{
    sink_ = sink;
    std::set<WlaConnection *>::iterator it;
    for (it = _connections.begin(); it != _connections.end(); ++it) {
        (*it)->setSink(sink);
    }
}

