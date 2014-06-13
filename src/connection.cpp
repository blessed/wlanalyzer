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

#include <errno.h>
#include <sys/select.h>
#include "connection.h"
#include "common.h"

WlaConnection::~WlaConnection()
{
    close();
}

WlaError WlaConnection::openConnection(UnixLocalSocket *client, UnixLocalSocket *server)
{
    if (!client || !server)
    {
        LOGGER_LOG("invalid arguments");
        return WlaConnectionFailed;
    }

    if (_connected)
    {
        LOGGER_LOG("already connected");
        return WlaConnectionFailed;
    }

    _client = client;
    _server = server;

    _clientWatcher.set<WlaConnection, &WlaConnection::handleConnection>(this);
    _serverWatcher.set<WlaConnection, &WlaConnection::handleConnection>(this);
}

void WlaConnection::close()
{
    _client->disconnectFromServer();
    delete _client;

    _server->disconnectFromServer();
    delete _server;
}

void WlaConnection::handleConnection(ev::io &watcher, int revents)
{
    UnixLocalSocket *src, *dst;

    if (revents & EV_ERROR)
    {
        LOGGER_LOG("connection handle error");
        return;
    }

    if (watcher.fd == *_client)
    {
        src = _client;
        dst = _server;
    }
    else
    {
        src = _server;
        dst = _client;
    }

    if (revents & EV_READ)
    {
        WlaBuffer *input = src->read();
    }
}
