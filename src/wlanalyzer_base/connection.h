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
#include "client_socket.h"
#include "common.h"
#include "wayland_raw_source.h"

namespace WlAnalyzer {

class WlaProxyServer;
class WlaConnection;

struct WlaMessageBuffer
{
    WlaMessageBuffer()
    {
        is_request = true;
        msg_len = 0;
        cmsg_len = 0;
        timestamp.tv_sec = 0;
        timestamp.tv_usec = 0;
    }

    static const int MAX_BUF_SIZE = 4096;
    static const int MAX_FDS = 28;

    bool is_request;
    timeval timestamp;
    char msg[MAX_BUF_SIZE];
    uint16_t msg_len;
    char cmsg[CMSG_LEN(MAX_FDS * sizeof(int))];
    uint16_t cmsg_len;
};

/**
 * @brief The WlaLink class
 *
 * Represents a unidirectional link between two wayland sockets.
 * It is responsible for transmitting messages from source to destination
 * socket.
 * Depending on the direction of the link the class transmits
 * either requests or events.
 * Each link should belong to a WlaConnection object.
 */
class WlaLink
{
public:
    enum LinkType {
        /** Link creates request messages */
        REQUEST_LINK,
        /** Link creates event messages */
        EVENT_LINK
    };

    /**
     * @brief Constructor
     * @param parent The connection that this link should belong to
     * @param src The source socket of this link
     * @param link The endpoint of the link
     * @param dir Type of messages this link transmits
     */
    WlaLink(WlaConnection *parent, const WlaClientSocket &src,
            const WlaClientSocket &link, LinkType type);
    virtual ~WlaLink();

    LinkType getLinkType() const { return _linkType; }

    WlaMessageBuffer *getLatestMessage() const
    {
        if (_messages.empty())
            return nullptr;

        return _messages.top();
    }

//    void setSink(const shared_ptr<RawMessageSink> &sink);
    void start();
    void stop();

private:
    void receiveEvent(ev::io &watcher, int revents);
    void transmitEvent(ev::io &watcher, int revents);
    WlaMessageBuffer *receiveMessage();
    bool sendMessage(WlaMessageBuffer *msg);

    WlaClientSocket _sourcepoint;
    WlaClientSocket _endpoint;
    LinkType _linkType;
    std::stack<WlaMessageBuffer *> _messages;
    WlaConnection *_parent;
};

/**
 * @brief The WlaConnection class
 *
 * WlaConnection represents a bidirectional connection
 * between a wayland client and a compositor.
 * Every connection is attached to a pipeline of
 * RawMessageSink and RawMessageSource objects with setSink().
 */
class WlaConnection
{
public:
    WlaConnection();
    ~WlaConnection();

    /**
     * @brief initializeConnection
     * @param client
     * @param server
     *
     * Creates a connection between two sockets.
     */
    void initializeConnection(WlaClientSocket client, WlaClientSocket server);
    void closeConnection();

    /**
     * @brief notifyOfNewMessage
     * @param msgBuffer
     * @param is_request
     *
     * Notifies the connection of a new message on a WlaLink
     */
    void notifyOfNewMessage(const WlaLink *link);

    /**
     * @brief setWaylandSource
     * @param waylandSource
     *
     * Sets a WaylandRawSource that will get notified everytime there is
     * a new protocol message, be it a request or an event.
     */
    void setWaylandSource(WaylandRawSource *source);

private:
    WlaLink *_client_link;
    WlaLink *_comp_link;

    bool running;

    WaylandRawSource *m_waylandSource;

//    WlaMessageParser _model;
};

} // namespace WlAnalyzer

#endif // CONNECTION_H
