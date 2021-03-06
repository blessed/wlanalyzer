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
#include "dumper.h"
#include "parser.h"
#include "analyzer.h"

class WlaProxyServer
{
public:
    WlaProxyServer();
    virtual ~WlaProxyServer();

    int init(const std::string &socketPath);
    int startServer();
    void stopServer();

    void closeConnection(WlaConnection *conn);

    void setDumper(WldDumper *dumper);
	void setParser(WldParser *parser);
//    void setAnalyzer(WldProtocolAnalyzer *an);

private:
    void connectClient(ev::io &watcher, int revents);
//    void handleCommunication(ev::io &watcher, int revents);

private:
    WldServer _serverSocket;
    ev::io _io;
    ev::default_loop _loop;

//    WlaIODumper writer;
    WldDumper *dumper;
//    WldIODumper writer;
	WldParser *parser;

    std::set<WlaConnection *> _connections;
};

#endif // PROXY_H
