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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <pthread.h>
#include <stack>
#include <vector>
#include <ev++.h>
#include "socket.h"
#include "common.h"
#include "message.h"
#include "wayland_raw_source.h"
#include <tr1/unordered_map>

namespace WlAnalyzer {

class WldMessageSink;
class WlaProxyServer;
class WlaConnection;

class WlaLink : public WldSocket
{
public:
	WlaLink(const WldSocket &src, const WldSocket &link, WlaMessageBuffer::MESSAGE_TYPE dir);
	virtual ~WlaLink();

	void setConnection(WlaConnection *connection)
	{
		_connection = connection;
	}

private:
	void receiveMessage(ev::io &watcher, int revents);
	void transmitMessage(ev::io &watcher, int revents);

	WldSocket _endpoint;
	WlaMessageBuffer::MESSAGE_TYPE _direction;
	WlaConnection *_connection;
	std::stack<WlaMessageBuffer *> _messages;
};

class WlaConnection
{
public:
    WlaConnection(WlaProxyServer *parent, WldMessageSink *dumper = NULL);
    ~WlaConnection();

    void createConnection(WldSocket client, WldSocket server);
    void closeConnection();
    void setDumper(WldMessageSink *dumper);

	void processMessage(WlaMessageBuffer *msg);

    void setSink(const shared_ptr<RawMessageSink> &sink);

private:
	WlaLink *_client_link;
	WlaLink *_comp_link;

    bool running;

    WlaProxyServer *parent;
    WldMessageSink *dumper;

    std::stack<WlaMessageBuffer *> events;
    std::stack<WlaMessageBuffer *> requests;

    WaylandRawSource request_source_;
    WaylandRawSource event_source_;

    shared_ptr<RawMessageSink> sink_;
};

} // namespace WlAnalyzer

#endif // CONNECTION_H
