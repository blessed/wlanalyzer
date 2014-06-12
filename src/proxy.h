#ifndef PROXY_H
#define PROXY_H

#include <string>
#include <ev++.h>
#include <vector>
#include "socket.h"
#include "server_socket.h"
#include "connection.h"

class WlaProxyServer
{
public:
    WlaProxyServer();
    virtual ~WlaProxyServer();

    int init(const std::string &socketPath);
    int openServer();
    void closeServer();

private:
    void connectClient(ev::io &watcher, int revents);

private:
    UnixLocalServer _serverSocket;
    ev::io _io;
    ev::default_loop _loop;

    std::vector<WlaConnection *> _connections;
};

#endif // PROXY_H
