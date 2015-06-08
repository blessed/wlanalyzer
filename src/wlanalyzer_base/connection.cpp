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

#include "connection.h"

#include <algorithm>
#include <cassert>

#include "common.h"
#include "proxy.h"
#include "raw_message.h"

using namespace std;

namespace WlAnalyzer {

WlaConnection::WlaConnection(WlaProxyServer *parent, WldMessageSink *writer) :
    request_source_(true), event_source_(false)
{
	_comp_link = nullptr;
	_client_link = nullptr;
    running = false;
    this->parent = parent;
    this->dumper = writer;
}

WlaConnection::~WlaConnection()
{
    closeConnection();
    parent->closeConnection(this);
}

void WlaConnection::createConnection(WldSocket cli, WldSocket server)
{
	_client_link = new WlaLink(cli, server, WlaMessageBuffer::REQUEST_TYPE);
	_comp_link = new WlaLink(server, cli, WlaMessageBuffer::EVENT_TYPE);
	_client_link->setConnection(this);
	_comp_link->setConnection(this);
	_client_link->start(EV_READ);
	_comp_link->start(EV_READ);

	DEBUG_LOG("connected %d with %d", cli.getSocketDescriptor(),
			  server.getSocketDescriptor());
}

void WlaConnection::setDumper(WldMessageSink *dumper)
{
	this->dumper = dumper;
}

void WlaConnection::processMessage(WlaMessageBuffer *msg)
{
	if (msg->getType() == WlaMessageBuffer::REQUEST_TYPE)
		request_source_.processBuffer(msg->getTimeStamp()->tv_sec, msg->getTimeStamp()->tv_usec,
									  msg->getMsg(), msg->getMsgSize());
	else
		event_source_.processBuffer(msg->getTimeStamp()->tv_sec, msg->getTimeStamp()->tv_usec,
									  msg->getMsg(), msg->getMsgSize());
}

void WlaConnection::setSink(const shared_ptr<RawMessageSink> &sink)
{
    sink_ = sink;
    request_source_.setSink(sink_);
    event_source_.setSink(sink_);
}

void WlaConnection::closeConnection()
{
    if (!running)
        return;

	_client_link->stop();
	_comp_link->stop();

    while (!requests.empty())
    {
        WlaMessageBuffer *msg = requests.top();
        delete msg;
        requests.pop();
    }

    while (!requests.empty())
    {
        WlaMessageBuffer *msg = events.top();
        delete msg;
        events.pop();
    }

	running = false;
}

WlaLink::WlaLink(const WldSocket &src, const WldSocket &link, WlaMessageBuffer::MESSAGE_TYPE dir) : WldSocket(src),
	_endpoint(link), _direction(dir)
{
	_connection = nullptr;

	set<WlaLink, &WlaLink::receiveMessage>(this);
	_endpoint.set<WlaLink, &WlaLink::transmitMessage>(this);
}

WlaLink::~WlaLink()
{
}

void WlaLink::receiveMessage(ev::io &watcher, int revents)
{
	if (revents & EV_ERROR)
		return;

	WlaMessageBuffer *msg = WldMessageBufferSocket::receiveMessage(*this);
	if (msg == nullptr) {
		return;
	}

	if (_connection)
		_connection->processMessage(msg);

	msg->setType(_direction);
	_messages.push(msg);
	_endpoint.stop();
	_endpoint.start(EV_READ | EV_WRITE);
}

void WlaLink::transmitMessage(ev::io &watcher, int revents)
{
	while (!_messages.empty()) {
		WlaMessageBuffer *msg = _messages.top();
		WldMessageBufferSocket::sendMessage(msg, _endpoint);
		_messages.pop();
		delete msg;
	}

	_endpoint.stop();
	_endpoint.start(EV_READ);
}

} // namespace WlAnalyzer
