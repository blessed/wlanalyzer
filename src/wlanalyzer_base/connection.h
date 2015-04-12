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

namespace WlAnalyzer {

class WldMessageSink;
class WlaIODumper;
class WlaProxyServer;

class WlaConnection
{
public:
    WlaConnection(WlaProxyServer *parent, WldMessageSink *dumper = NULL);
    ~WlaConnection();

    void createConnection(WldSocket client, WldSocket server);
    void closeConnection();
    void setDumper(WldMessageSink *dumper);

    void setSink(const shared_ptr<RawMessageSink> &sink);

private:
    void handleConnection(ev::io &watcher, int revents);
    WlaMessageBuffer *handleRead(WldSocket &src, WldSocket &dst);
    void handleWrite(WldSocket &dst, std::stack<WlaMessageBuffer *> &msgStack);

private:
    WldSocket client;
    WldSocket wayland;

    bool running;

    WlaProxyServer *parent;
//    WlaIODumper *writer;
    WldMessageSink *dumper;

    std::stack<WlaMessageBuffer *> events;
    std::stack<WlaMessageBuffer *> requests;

    WaylandRawSource request_source_;
    WaylandRawSource event_source_;

    shared_ptr<RawMessageSink> sink_;
};

} // namespace WlAnalyzer

#endif // CONNECTION_H
