/*
 * Copyright © 2014 Samsung Electronics
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
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
        int len = _serverSocket->read(buf, 64 * 1024);
        if (len < 0)
        {
            LOGGER_LOG("failed to read from server %d", len);
            delete buf;
            return;
        }
        else if (len == 0)
        {
            delete buf;
            return;
        }

        WlaMessage *msg = new WlaMessage(WlaMessage::EVENT_TYPE);
        msg->setData(buf, len);

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

            if (!_serverSocket->write(msg->data(), msg->size()))
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
        int len = _clientSocket->read(buf, 64 * 1024);
        if (len < 0)
        {
            LOGGER_LOG("failed to read from client %d", len);
            delete buf;
            return;
        }
        else if (len == 0)
        {
            delete buf;
            return;
        }

        WlaMessage *msg = new WlaMessage(WlaMessage::REQUEST_TYPE);
        msg->setData(buf, len);

        _requests.push(msg);

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

            if (!_clientSocket->write(msg->data(), msg->size()))
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
