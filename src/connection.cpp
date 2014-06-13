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

WlaConnection::WlaConnection(UnixLocalSocket *client, UnixLocalSocket *server)
{
    if (!(client && server))
        return;

    _clientSocket = client;
    _serverSocket = server;

    _serverWatcher.set<WlaConnection, &WlaConnection::serverHandler>(this);
    _clientWatcher.set<WlaConnection, &WlaConnection::clientHandler>(this);

    _serverWatcher.start(_serverSocket->getSocketDescriptor(), EV_READ);
    _clientWatcher.start(_clientSocket->getSocketDescriptor(), EV_READ);
}

WlaConnection::~WlaConnection()
{
    close();
}

void WlaConnection::close()
{
    // TODO: stop connection
    _clientSocket->disconnectFromServer();
    delete _clientSocket;

    _serverSocket->disconnectFromServer();
    delete _serverSocket;
}

void WlaConnection::serverHandler(ev::io &watcher, int revents)
{
    if (revents & EV_ERROR)
    {
        LOGGER_LOG("bad event");
        return;
    }

    if (revents & EV_READ)
    {
        char *buf = new char[64 * 1024];
//        int len = _serverSocket->read(buf, 64 * 1024);
//        if (len < 0)
//        {
//            LOGGER_LOG("failed to read from server %d", len);
//            delete buf;
//            return;
//        }
//        else if (len == 0)
//        {
//            delete buf;
//            return;
//        }

        msghdr *hdr = _serverSocket->readMessage(buf, 64 * 1024);
        if (!hdr)
        {
            LOGGER_LOG("failed to read from server");
            return;
        }

        WlaMessage *msg = new WlaMessage(WlaMessage::EVENT_TYPE);
        msg->setHeader(hdr);

        _events.push(msg);

        _clientWatcher.stop();
        _clientWatcher.set(EV_READ | EV_WRITE);
        _clientWatcher.start();
    }
    else if (revents & EV_WRITE)
    {
        while (!_requests.empty())
        {
            WlaMessage *msg = _requests.front();
            _requests.pop();

            if (!_serverSocket->writeMessage(msg->getHeader()))
            {
                LOGGER_LOG("failed to write to server");
            }

            delete msg;
        }

        _serverWatcher.stop();
        _serverWatcher.set(EV_READ);
        _serverWatcher.start();
    }
}

void WlaConnection::clientHandler(ev::io &watcher, int revents)
{
    if (revents & EV_ERROR)
    {
        LOGGER_LOG("bad event");
        return;
    }

    if (revents & EV_READ)
    {
        char *buf = new char[64 * 1024];
//        int len = _clientSocket->read(buf, 64 * 1024);
//        if (len < 0)
//        {
//            LOGGER_LOG("failed to read from client %d", len);
//            delete buf;
//            return;
//        }
//        else if (len == 0)
//        {
//            delete buf;
//            return;
//        }

        msghdr *msg = _clientSocket->readMessage(buf, 64 * 1024);
        if (!msg)
        {
            LOGGER_LOG("Failed to read message from client");
            return;
        }

        WlaMessage *message = new WlaMessage(WlaMessage::REQUEST_TYPE);
//        message->setData(buf, len);
        message->setHeader(msg);

        _requests.push(message);

        _serverWatcher.stop();
        _serverWatcher.set(EV_READ | EV_WRITE);
        _serverWatcher.start();
    }
    else if (revents & EV_WRITE)
    {
        while (!_events.empty())
        {
            WlaMessage *msg = _events.front();
            _events.pop();

            if (!_clientSocket->writeMessage(msg->getHeader()))
            {
                LOGGER_LOG("failed to write to server");
            }

            delete msg;
        }

        _clientWatcher.stop();
        _clientWatcher.set(EV_READ);
        _clientWatcher.start();
    }
}
