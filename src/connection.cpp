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
#include "proxy.h"
#include "dumper.h"
#include "connection.h"

using namespace std;

WlaConnection::WlaConnection(WlaProxyServer *parent, WlaIODumper *writer)
{
    this->parent = parent;
    this->writer = writer;
}

WlaConnection::~WlaConnection()
{
    closeConnection();
    parent->closeConnection(this);
}

void WlaConnection::createConnection(UnixLocalSocket cli, UnixLocalSocket server)
{
    client = cli;
    wayland = server;

    client.set<WlaConnection, &WlaConnection::handleConnection>(this);
    wayland.set<WlaConnection, &WlaConnection::handleConnection>(this);

    wayland.start(EV_READ);
    client.start(EV_READ);

    DEBUG_LOG("connected %d with %d", client.getSocketDescriptor(),
              wayland.getSocketDescriptor());
}

void WlaConnection::handleConnection(ev::io &watcher, int revents)
{
    if (revents & EV_ERROR)
    {
        DEBUG_LOG("invalid event %s", strerror(errno));
        return;
    }

    if (revents & EV_READ)
    {
        if (watcher.fd == client)
        {
            WlaMessage *msg = handleRead(client, wayland);
            if (!msg)
            {
                DEBUG_LOG("peer disconnected");
                delete this;
                return;
            }

            writer->write(*msg);
            msg->setType(WlaMessage::REQUEST_TYPE);
            requests.push(msg);
        }
        else
        {
            WlaMessage *msg = handleRead(wayland, client);
            if (!msg)
            {
                DEBUG_LOG("peer disconnected");
                delete this;
                return;
            }

            writer->write(*msg);
            msg->setType(WlaMessage::EVENT_TYPE);
            events.push(msg);
        }
    }
    else if (revents & EV_WRITE)
    {
        if (watcher.fd == client)
            handleWrite(client, events);
        else
            handleWrite(wayland, requests);
    }
}

WlaMessage *WlaConnection::handleRead(UnixLocalSocket &src, UnixLocalSocket &dst)
{
    WlaMessage *msg = new WlaMessage;
    int len = msg->receiveMessage(src);
    if (len < 0)
    {
        delete msg;
        return NULL;
    }
    else if (len == 0)
    {
        delete msg;
        return NULL;
    }

    dst.stop();
    dst.start(EV_READ | EV_WRITE);

    return msg;
}

void WlaConnection::handleWrite(UnixLocalSocket &dst, std::stack<WlaMessage *> &msgStack)
{
    while (!msgStack.empty())
    {
        WlaMessage *msg = msgStack.top();

        int len = msg->sendMessage(dst);;

        msgStack.pop();
        delete msg;
    }

    dst.stop();
    dst.start(EV_READ);
}

void WlaConnection::closeConnection()
{
    client.stop();
    wayland.stop();

    while (!requests.empty())
    {
        WlaMessage *msg = requests.top();
        delete msg;
        requests.pop();
    }

    while (!requests.empty())
    {
        WlaMessage *msg = events.top();
        delete msg;
        events.pop();
    }
}
