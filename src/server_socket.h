#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include <string>
#include <queue>
#include "socket.h"

class UnixLocalServer
{
public:
    UnixLocalServer();
    ~UnixLocalServer();

    void setMaxPendingConnections(unsigned int num)
    {
        _maxPendingConnections = num;
    }

    std::string getServerName() const
    {
        return _serverName;
    }

    bool listen(const std::string &name);
    bool isListening() const { return !_serverName.empty(); }

    bool waitForConnection(int ms, bool *timedout);
    UnixLocalSocket *nextPendingConnection();

private:
    void closeServer();
    void clearPendingConnections();

    void onNewConnection();

private:
    int _fd;
    unsigned int _maxPendingConnections;
    std::string _serverName;
    std::queue<UnixLocalSocket *> _pendingConnections;
};

#endif // SERVER_SOCKET_H
