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


#ifndef PROXY_H
#define PROXY_H

#include <string>
#include <ev++.h>
#include <set>
#include "socket.h"
#include "server_socket.h"
#include "connection.h"
#include "analyzer.h"

#include "raw_message_sink.h"

namespace WlAnalyzer {

/**
 * @brief The WlaProxyServer class
 *
 * This class represents a proxy object that intercepts
 * connections to the wayland compositor.
 * The proxy server is able to track multiple connections with
 * the comppositor.
 */
class WlaProxyServer
{
public:

    WlaProxyServer();
    virtual ~WlaProxyServer();

    /**
     * @brief init
     * @param socketPath    proxy socket path
     * @return 0 on succes
     *         -1 on failure
     *
     * Creates a listening socket which intercepted wayland clients
     * are supposed to connect to to.
     */
    int init(const std::string &socketPath);

    /**
     * @brief startServer
     *
     * Starts listening for incomming wayland clients.
     */
    void startServer();

    /**
     * @brief stopServer
     *
     * Stops listening for wayland clients.
     */
    void stopServer();

    /**
     * @brief closeConnection
     * @param conn connection that is being closed
     */
    void closeConnection(WlaConnection *conn);
    void setSink(const shared_ptr<RawMessageSink> &value);

private:
    void connectClient(ev::io &watcher, int revents);

    WldServer _serverSocket;
    ev::io _io;
    ev::default_loop _loop;

    std::set<WlaConnection *> _connections;
    shared_ptr<RawMessageSink> sink_;
};

} // namespace WlAnalyzer

#endif // PROXY_H
