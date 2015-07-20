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
    struct WlaMessageBuffer
    {
        WlaMessageBuffer()
        {
            msg_len = 0;
            cmsg_len = 0;
            timestamp.tv_sec = 0;
            timestamp.tv_usec = 0;
        }

        static const int MAX_BUF_SIZE = 4096;
        static const int MAX_FDS = 28;

        timeval timestamp;
        char msg[MAX_BUF_SIZE];
        uint16_t msg_len;
        char cmsg[CMSG_LEN(MAX_FDS * sizeof(int))];
        uint16_t cmsg_len;
    };

public:
    enum LinkType {
        /** Link creates request messages */
        REQUEST_LINK,
        /** Link creates event messages */
        EVENT_LINK
    };

    /**
     * @brief Constructor
     * @param src The source socket of this link
     * @param link The endpoint of the link
     * @param dir Type of messages this link transmits
     */
    WlaLink(const WlaClientSocket &src, const WlaClientSocket &link, LinkType type);
    virtual ~WlaLink();

    void setSink(const shared_ptr<RawMessageSink> &sink);
    void start();
    void stop();

private:
    void receiveEvent(ev::io &watcher, int revents);
    void transmitEvent(ev::io &watcher, int revents);
    WlaMessageBuffer *receiveMessage();
    bool sendMessage(WlaMessageBuffer *msg);

    WlaClientSocket _sourcepoint;
    WlaClientSocket _endpoint;
    std::stack<WlaMessageBuffer *> _messages;
    WaylandRawSource _msgsource;
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
    WlaConnection(WlaProxyServer *parent);
    ~WlaConnection();

    /**
     * @brief initializeConnection
     * @param client
     * @param server
     *
     * Creates a connection from two sockets.
     */
    void initializeConnection(WlaClientSocket client, WlaClientSocket server);
    void closeConnection();

    /**
     * @brief setSink
     * @param sink
     *
     * Attaches this connection to a message pipeline.
     */
    void setSink(const shared_ptr<RawMessageSink> &sink);

private:
    WlaLink *_client_link;
    WlaLink *_comp_link;

    bool running;

    WlaProxyServer *parent;

    shared_ptr<RawMessageSink> sink_;

//    WlaMessageParser _model;
};

} // namespace WlAnalyzer

#endif // CONNECTION_H
