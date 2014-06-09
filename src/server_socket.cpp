
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "server_socket.h"

using namespace std;

UnixLocalServer::UnixLocalServer() : _fd(-1), _maxPendingConnections(1), _serverName("")
{
}

UnixLocalServer::~UnixLocalServer()
{
    if (isListening())
        closeServer();
}

void UnixLocalServer::closeServer()
{
    if (!isListening())
        return;

    clearPendingConnections();

    ::close(_fd);
    unlink(_serverName.c_str());

    _serverName.clear();
}

void UnixLocalServer::clearPendingConnections()
{
    if (_pendingConnections.empty())
        return;

    UnixLocalSocket *temp;

    while (!_pendingConnections.empty())
    {
        temp = _pendingConnections.front();
        _pendingConnections.pop();
        delete temp;
    }
}

bool UnixLocalServer::listen(const string &name)
{
    int err = 0;

    if (name.empty())
        return false;

    if (isListening())
        return false;

    _fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (_fd == -1)
        return false;

    sockaddr_un addr;
    memset(&addr, 0, sizeof(sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, name.c_str(), name.length());

    err = ::bind(_fd, (sockaddr *)&addr, sizeof(sockaddr_un));
    if (err)
    {
        ::close(_fd);
        return false;
    }

    ::listen(_fd, _maxPendingConnections);

    _serverName = name;

    return true;
}

void UnixLocalServer::onNewConnection()
{
    int clientSocket;
    sockaddr_un addr;
    socklen_t socklen;

    socklen = sizeof(sockaddr_un);
    clientSocket = accept(_fd, (sockaddr *)&addr, &socklen);
    if (clientSocket == -1)
        return;

    UnixLocalSocket *localSocket = new UnixLocalSocket;
    localSocket->setSocketDescriptor(clientSocket);
    _pendingConnections.push(localSocket);
}

bool UnixLocalServer::waitForConnection(int ms, bool *timedout)
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(_fd, &readfds);

    timeval timeout;
    timeout.tv_sec = ms / 1000;
    timeout.tv_usec = (ms % 1000) * 1000;

    while (select(1, &readfds, NULL, NULL, &timeout) == -1)
    {
        if (errno == EINTR)
            continue;
        else if (errno == ETIMEDOUT)
        {
            *timedout = true;
            return false;
        }
    }

    onNewConnection();

    *timedout = false;

    if (!_pendingConnections.empty())
        return false;

    return true;
}

UnixLocalSocket *UnixLocalServer::nextPendingConnection()
{
    if (_pendingConnections.empty())
        return NULL;

    UnixLocalSocket *socket = _pendingConnections.back();
    _pendingConnections.pop();

    return socket;
}
