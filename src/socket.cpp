
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
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

void UnixLocalSocket::setSocketDescriptor(int fd)
{
    if (isConnected())
        disconnectFromServer();

    _fd = fd;
    _connected = true;
}

long UnixLocalSocket::read(char *data, long max_size)
{
    if (!isConnected())
        return -1;

    int err = recv(_fd, data, max_size, 0);
    if (err == -1)
        return -1;

    return err;
}

bool UnixLocalSocket::write(const char *data, long c)
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
