#include "common.h"
#include "proxy_source.h"

#include <cassert>

namespace WlAnalyzer {

WlaProxySource::WlaProxySource() : _loop(EVBACKEND_SELECT)
{
    DEBUG_LOG("_loop.backend %d", _loop.backend());
}

WlaProxySource::~WlaProxySource()
{
    for (size_t i = 0; i < _connections.size(); ++i) {
        _connections[i]->closeConnection();
    }
}

int WlaProxySource::initialize(const std::string &socketName)
{
    if (socketName.length() == 0)
    {
        DEBUG_LOG("Invalid proxy server socket name");
        return -1;
    }

    std::string fullPath = std::string(getenv("XDG_RUNTIME_DIR")) +
            "/" + socketName;

    if (!_proxyServerSocket.listen(fullPath)) {
        Logger::getInstance()->log("Failed to listen on %s\n", socketName.c_str());
        return -1;
    }

    _proxyServerSocket.set<WlaProxySource, &WlaProxySource::acceptConnection>(this);
    _proxyServerSocket.start(EV_READ);

    return 0;
}

void WlaProxySource::start()
{
    _loop.run();
}

void WlaProxySource::stop()
{
    _proxyServerSocket.stop();

    if (_proxyServerSocket.isListening())
        _proxyServerSocket.close();

    _loop.break_loop(ev::ALL);
}

void WlaProxySource::acceptConnection(ev::io &watcher, int revent)
{
    if (revent & EV_ERROR) {
        DEBUG_LOG("got invalid event");
        stop();
        return;
    }

    WlaClientSocket wayland;
    std::string waylandPath = std::string(getenv("XDG_RUNTIME_DIR")) +
            "/" + std::string(getenv("WAYLAND_DISPLAY"));
    if (wayland.connectTo(waylandPath) != NoError) {
        DEBUG_LOG("Failed to accept a new connection");
        stop();
        return;
    }

    bool timedout;
    WlaServerSocket *acceptingServer = &((WlaProxySource *)watcher.data)->_proxyServerSocket;
    if (!acceptingServer->waitForConnection(1000, &timedout)) {
        DEBUG_LOG("Connection not accepted");
        stop();
        return;
    }

    if (timedout) {
        DEBUG_LOG("Connection timed out");
        stop();
        return;
    }

    WlaClientSocket *client = _proxyServerSocket.nextPendingConnection();
    assert(client != nullptr);

    shared_ptr<WlaConnection> connection(new WlaConnection());
    connection->initializeConnection(*client, wayland);
    connection->setWaylandSource(this);
    _connections.push_back(connection);

    delete client;
}

} // namespace WlAnalyzer

