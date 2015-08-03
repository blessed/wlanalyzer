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
#include <sys/time.h>

#include "common.h"
#include "proxy.h"
#include "raw_message.h"

using namespace std;

namespace WlAnalyzer {

WlaConnection::WlaConnection()
{
    _comp_link = nullptr;
    _client_link = nullptr;
    running = false;
    m_waylandSource = nullptr;
}

WlaConnection::~WlaConnection()
{
    closeConnection();
//    parent->closeConnection(this);
}

void WlaConnection::initializeConnection(WlaClientSocket cli, WlaClientSocket server)
{
    _client_link = new WlaLink(this, cli, server, WlaLink::REQUEST_LINK);
    _comp_link = new WlaLink(this, server, cli, WlaLink::EVENT_LINK);
    _client_link->start();
    _comp_link->start();

    DEBUG_LOG("connected %d with %d", cli.getFd(),
              server.getFd());
}

void WlaConnection::setWaylandSource(WaylandRawSource *source)
{
    m_waylandSource = source;
//    _client_link->setSink(sink);
//    _comp_link->setSink(sink);
}

void WlaConnection::closeConnection()
{
    if (!running)
        return;

    _client_link->stop();
    _comp_link->stop();
    delete _client_link;
    delete _comp_link;

    running = false;
}

void WlaConnection::notifyOfNewMessage(const WlaLink *link)
{
    if (!link || !m_waylandSource)
        return;

    WlaMessageBuffer *msg = link->getLatestMessage();
    m_waylandSource->processBuffer(msg->is_request, msg->timestamp.tv_sec, msg->timestamp.tv_usec, msg->msg, msg->msg_len);
}

WlaLink::WlaLink(WlaConnection *parent, const WlaClientSocket &src, const WlaClientSocket &link, LinkType type) : _sourcepoint(src),
    _endpoint(link), _linkType(type), _parent(parent)
{
    _sourcepoint.set<WlaLink, &WlaLink::receiveEvent>(this);
    _endpoint.set<WlaLink, &WlaLink::transmitEvent>(this);
}

WlaLink::~WlaLink()
{
}

void WlaLink::start()
{
    _sourcepoint.start(EV_READ);
}

void WlaLink::stop()
{
    _sourcepoint.stop();
}

void WlaLink::receiveEvent(ev::io &watcher, int revents)
{
    if (revents & EV_ERROR)
        return;

    WlaMessageBuffer *msg = receiveMessage();
    if (msg == nullptr) {
        return;
    }

    _messages.push(msg);
    _endpoint.stop();
    _endpoint.start(EV_READ | EV_WRITE);
    _parent->notifyOfNewMessage(this);
}

void WlaLink::transmitEvent(ev::io &watcher, int revents)
{
    while (!_messages.empty()) {
        WlaMessageBuffer *msg = _messages.top();
        sendMessage(msg);
        _messages.pop();
        delete msg;
    }

    _endpoint.stop();
    _endpoint.start(EV_READ);
}

WlaMessageBuffer *WlaLink::receiveMessage()
{
    WlaMessageBuffer *buffer = new WlaMessageBuffer;
    if (!buffer) {
        Logger::getInstance()->log("Error: Out of memory\n");
        return nullptr;
    }

    iovec iov;
    iov.iov_base = buffer->msg;
    iov.iov_len = WlaMessageBuffer::MAX_BUF_SIZE;

    msghdr msg;
    msg.msg_name = nullptr;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = buffer->cmsg;
    msg.msg_controllen = CMSG_LEN(WlaMessageBuffer::MAX_FDS * sizeof(int));
    buffer->is_request = _linkType == REQUEST_LINK ? true : false;

    int len = _sourcepoint.readMsg(&msg);
    if (len < 0) {
        Logger::getInstance()->log("Failed to read msg\n");
        perror(nullptr);
        delete buffer;
        return nullptr;
    } else if (len > 0) {
        gettimeofday(&buffer->timestamp, nullptr);
        buffer->msg_len = len;
        buffer->cmsg_len = msg.msg_controllen;
    } else {
        delete buffer;
        return nullptr;
    }

    return buffer;
}

bool WlaLink::sendMessage(WlaMessageBuffer *msg)
{
    iovec iov;
    iov.iov_base = msg->msg;
    iov.iov_len = msg->msg_len;

    msghdr hdr;
    hdr.msg_name = nullptr;
    hdr.msg_namelen = 0;
    hdr.msg_iov = &iov;
    hdr.msg_iovlen = 1;
    hdr.msg_control = msg->cmsg;
    hdr.msg_controllen = msg->cmsg_len;

    int len = _endpoint.writeMsg(&hdr);
    if (len < 0) {
        Logger::getInstance()->log("Error: Failed to send message\n");
        perror(nullptr);
        return false;
    }

    return true;
}

} // namespace WlAnalyzer
