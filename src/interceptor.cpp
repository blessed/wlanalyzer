/*
 * Copyright © 2014 Samsung Electronics
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
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
