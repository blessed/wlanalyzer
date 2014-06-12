
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <fcntl.h>

#include "common.h"
#include "server_socket.h"

using namespace std;

UnixLocalServer::UnixLocalServer() : _fd(-1), _maxPendingConnections(1), _serverName(""),
    _listening(false)
{
}

UnixLocalServer::~UnixLocalServer()
{
    Logger::getInstance()->log("called destructor\n");
    closeServer();
}

void UnixLocalServer::closeServer()
{
    if (!isListening())
        return;

    clearPendingConnections();

    ::close(_fd);
    unlink(_serverName.c_str());

    Logger::getInstance()->log("server closed\n");

    _listening = false;
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
    {
        wld_log("empty server name\n");
        return false;
    }

    if (isListening())
    {
        wld_log("server is already listening\n");
        return false;
    }

    _fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (_fd == -1)
    {
        wld_log("socket() failed\n");
        return false;
    }

    sockaddr_un addr;
    memset(&addr, 0, sizeof(sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, name.c_str(), name.length());

    err = ::bind(_fd, (sockaddr *)&addr, sizeof(sockaddr_un));
    if (err)
    {
        wld_log("bind() failed\n");
        perror(NULL);
        ::close(_fd);
        return false;
    }

    ::listen(_fd, _maxPendingConnections);

    int val = 1;
    setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof val);

    LOGGER_LOG("listening on %s", name.c_str());

    _serverName = name;
    _listening = true;

    return true;
}

void UnixLocalServer::onNewConnection()
{
    int clientSocket;
    sockaddr_un addr;
    socklen_t socklen;

    LOGGER_LOG("new connection");
    LOGGER_LOG("dupa");

    socklen = sizeof(sockaddr_un);
    clientSocket = accept(_fd, (sockaddr *)&addr, &socklen);
    if (clientSocket == -1)
        return;

    LOGGER_LOG("Hm hm");

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
        {
            LOGGER_LOG("EINTR\n");
            continue;
        }
        else if (errno == ETIMEDOUT)
        {
            *timedout = true;
            return false;
        }
    }

    onNewConnection();

    *timedout = false;

    if (_pendingConnections.empty())
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

void UnixLocalServer::close()
{
    closeServer();
}
