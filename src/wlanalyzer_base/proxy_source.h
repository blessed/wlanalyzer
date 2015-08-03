#ifndef WLAPROXYSOURCE_H
#define WLAPROXYSOURCE_H

#include "server_socket.h"
#include "raw_message_source.h"
#include "connection.h"
#include <string>
#include <vector>

namespace WlAnalyzer {

class WlaProxySource : public WaylandRawSource
{
public:
    WlaProxySource();
    ~WlaProxySource();

    int initialize(const std::string &socketName);
    void start();
    void stop();

private:
    void acceptConnection(ev::io &watcher, int revent);

    WlaServerSocket _proxyServerSocket;
    ev::default_loop _loop;
    std::vector<std::shared_ptr<WlaConnection>> _connections;
};

} // namespace WlAnalyzer

#endif // WLAPROXYSOURCE_H
