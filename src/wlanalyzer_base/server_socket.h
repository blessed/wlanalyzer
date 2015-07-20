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


#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include <string>
#include <queue>
#include "socket.h"
#include "client_socket.h"

namespace WlAnalyzer {

class WlaServerSocket : public WlaSocketBase
{
public:
    WlaServerSocket();
    ~WlaServerSocket();

    void setMaxPendingConnections(uint16_t num)
    {
        _maxPendingConnections = num;
    }

    std::string getServerName() const
    {
        return _serverName;
    }

    bool listen(const std::string &name);
    bool isListening() const { return _listening; }

    bool waitForConnection(int ms, bool *timedout);
    WlaClientSocket *nextPendingConnection();

    void close();

protected:
    virtual bool bind(const std::string &resource);
    virtual void onNewConnection();

    std::queue<WlaClientSocket *> _pendingConnections;

private:
    void closeServer();
    void clearPendingConnections();

    uint16_t _maxPendingConnections;
    std::string _serverName;
    bool _listening;
};

class WlaNetServerSocket : public WlaServerSocket
{
protected:
    bool bind(const std::string &resource);
    void onNewConnection();
};

} // WlAnalyzer

#endif // SERVER_SOCKET_H
