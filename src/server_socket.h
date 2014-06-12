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

    int getFd() const { return _fd; }

    bool listen(const std::string &name);
    bool isListening() const { return _listening; }

    bool waitForConnection(int ms, bool *timedout);
    UnixLocalSocket *nextPendingConnection();

    void close();

private:
    void closeServer();
    void clearPendingConnections();

    void onNewConnection();

private:
    int _fd;
    unsigned int _maxPendingConnections;
    std::string _serverName;
    bool _listening;
    std::queue<UnixLocalSocket *> _pendingConnections;
};

#endif // SERVER_SOCKET_H
