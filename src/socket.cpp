
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <fcntl.h>
#include "socket.h"
#include "common.h"

UnixLocalSocket::UnixLocalSocket() : _fd(-1), _path(""), _connected(false)
{
}

UnixLocalSocket::~UnixLocalSocket()
{
    if (isConnected())
    {
        disconnectFromServer();
    }
}

UnixLocalSocketError UnixLocalSocket::connectToServer(const char *path)
{
    std::string strPath(path);
    return connectToServer(strPath);
}

UnixLocalSocketError UnixLocalSocket::connectToServer(const std::string &path)
{
    int err;

    if (path.empty())
        return ServerNotFoundError;

    if (isConnected())
        disconnectFromServer();

    _fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (check_error(_fd))
        return SocketResourceError;

    _path = path;
    sockaddr_un address;
    memset(&address, 0, sizeof(sockaddr_un));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, _path.c_str(), _path.size());
    err = connect(_fd, (sockaddr *)&address, sizeof(sockaddr_un));
    if (err == -1)
    {
        disconnectFromServer();

        switch (errno)
        {
        case EINVAL:
        case ECONNREFUSED:
            return ConnectionRefusedError;
        case ENOENT:
            return ServerNotFoundError;
        case ETIMEDOUT:
            return SocketTimeoutError;

        default:
            return UnknownSocketError;
        }
    }

    Logger::getInstance()->log("Connected to %s\n", _path.c_str());

    _connected = true;

    return NoError;
}

UnixLocalSocketError UnixLocalSocket::disconnectFromServer()
{
    if (isConnected())
    {
        close(_fd);
        _fd = -1;

        _path.clear();
        _connected = false;
    }
}

void UnixLocalSocket::setSocketDescriptor(int fd, int flags)
{
    if (isConnected())
        disconnectFromServer();

    if (flags)
    {
        fcntl(fd, F_SETFL, flags);
    }

    _fd = fd;
    _connected = true;
}

long UnixLocalSocket::read(char *data, long max_size) const
{
    if (!isConnected())
        return -1;

    int err = recv(_fd, data, max_size, MSG_DONTWAIT);
    if (err == -1 && (errno == EWOULDBLOCK || errno == EAGAIN))
        return 0;

    return err;
}

bool UnixLocalSocket::write(const char *data, long c) const
{
    while (c > 0)
    {
        int wrote = send(_fd, data, c, 0);
        if (wrote == -1)
            return false;

        c -= wrote;
    }

    return true;
}
