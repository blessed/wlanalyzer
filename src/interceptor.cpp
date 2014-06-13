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

#include <unistd.h>
#include "common.h"
#include "interceptor.h"

WldInterceptor::WldInterceptor()
{
    _configured = false;
}

WldInterceptor::~WldInterceptor()
{
    if (_interceptServerSocket.isListening())
        _interceptServerSocket.close();

    WLD_DEBUG_LOG("Called destructor");
}

bool WldInterceptor::createProxySocket(const std::string &name)
{
    char *runtime_dir;

    if (name.empty())
    {
        WLD_DEBUG_LOG("missing proxy socket path");
        return false;
    }

    runtime_dir = getenv("XDG_RUNTIME_DIR");
    if (!runtime_dir)
    {
        WLD_DEBUG_LOG("XDG_RUNTIME_DIR is not set");
        return false;
    }


    std::string path = std::string(runtime_dir) + "/" + name;
    if (!access(path.c_str(), F_OK))
    {
        Logger::getInstance()->log("file %s already exists. Removing\n", path.c_str());
        unlink(path.c_str());
    }

    if (!_interceptServerSocket.listen(path))
    {
        WLD_DEBUG_LOG("failed to listen on socket %s", path.c_str());
        return false;
    }

    _configured = true;

    return true;
}

bool WldInterceptor::runProxy()
{
    if (!_configured)
        return false;

    return start();
}

void WldInterceptor::stopProxy()
{
    stop();
}

void WldInterceptor::run()
{
    while (_running)
    {
        bool timeout;

        Logger::getInstance()->log("tick\n");

        if (_interceptServerSocket.waitForConnection(1000, &timeout))
        {
            WLD_DEBUG_LOG("connection incomming");
            UnixLocalSocket *incomming = _interceptServerSocket.nextPendingConnection();
            createConnection(incomming);
        }

        if (timeout)
            WLD_DEBUG_LOG("connection timed out");
    }

    LOGGER_LOG("exiting run\n");
}

void WldInterceptor::createConnection(UnixLocalSocket *client)
{
    WlaConnection *conn;
    conn = new WlaConnection(client, &_waylandSocket);
    if (!conn)
        return;

    WLD_DEBUG_LOG("created connection");

    LOGGER_LOG("");

    _connections.push_back(conn);
}
