#include "common.h"
#include "client_socket.h"

#include <fcntl.h>
#include <cassert>
#include <netinet/ip.h>
#include <arpa/inet.h>

namespace WlAnalyzer {

WlaClientSocket::WlaClientSocket() : WlaSocketBase(), _connected(false)
{
}

WlaClientSocket::WlaClientSocket(const WlaClientSocket &copy)
{
    if (copy.isConnected()) {
        _fd = dup(copy._fd);
        _connected = true;
    }
}

WlaClientSocket::WlaClientSocket(int fd, bool connected)
{
    _fd = dup(fd);
    _connected = connected;
}

WlaClientSocket::~WlaClientSocket()
{
    if (isConnected()) {
        disconnect();
    }
}

WlaClientSocket &WlaClientSocket::operator=(const WlaClientSocket &other)
{
    if (other.isConnected()) {
        _fd = dup(other._fd);
        _connected = true;
    }

    return *this;
}

void WlaClientSocket::setFd(int fd, int flags)
{
    if (isConnected())
        disconnect();

    if (flags)
        fcntl(fd, F_SETFL, flags);

    _fd = fd;
    _connected = true;
}

SocketError WlaClientSocket::connectTo(const char *path)
{
    std::string strPath(path);
    return connectTo(strPath);
}

SocketError WlaClientSocket::connectTo(const std::string &path)
{
    if (path.empty())
        return ServerNotFoundError;

    if (isConnected())
        disconnect();

    SocketError err = connect(path);
    if (err != NoError) {
        disconnect();
        return err;
    }

    DEBUG_LOG("Connected to %s", path.c_str());
    _connected = true;
    _connectionUrl = path;

    return NoError;
}

SocketError WlaClientSocket::disconnect()
{
    if (_connected) {
        close(_fd);
        _fd = -1;

        _connected = false;
    }

    return NoError;
}

long WlaClientSocket::read(char *data, size_t max_size) const
{
    if (!isConnected())
        return -1;

    int err;
    do {
        err = recv(_fd, data, max_size, MSG_WAITALL);
    } while (err == -1 && (errno == EWOULDBLOCK || errno == EAGAIN));

    return err;
}

bool WlaClientSocket::write(const char *data, size_t c) const
{
    while (c > 0) {
        int wrote = send(_fd, data, c, MSG_NOSIGNAL);
        if (wrote == -1)
            return false;

        c -= wrote;
    }

    return true;
}

int WlaClientSocket::readMsg(msghdr *hdr)
{
    int len;

    do {
        len = recvmsg(_fd, hdr, 0);
    } while (len < 0 && errno == EINTR);

    return len;
}

int WlaClientSocket::writeMsg(const msghdr *hdr)
{
    int len;

    do {
        len = sendmsg(_fd, hdr, 0);
    } while (len < 0 && errno == EINTR);

    return len;
}

SocketError WlaClientSocket::connect(const std::string &path)
{
    assert(!path.empty());

    _fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (check_error(_fd))
        return SocketResourceError;

    sockaddr_un address;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, path.c_str(), path.size());
    int err = ::connect(_fd, (sockaddr *)&address, sizeof(address));
    if (err == -1) {
        disconnect();

        switch (errno) {
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

    return NoError;
}

WlaNetClientSocket::WlaNetClientSocket(const WlaNetClientSocket &copy) : WlaClientSocket(copy)
{
}

SocketError WlaNetClientSocket::connect(const std::string &path)
{
    DEBUG_LOG("");

    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (check_error(_fd))
        return SocketResourceError;

    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));

    size_t idx = path.find(':');
    std::string ip = path.substr(0, idx);
    std::string port = path.substr(idx + 1);
    if (port.empty())
        return InvalidServerAddress;


    addr.sin_port = htons(atoi(port.c_str()));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    int err = ::connect(_fd ,(sockaddr *)&addr, sizeof(sockaddr_in));
    if (err == -1)
    {
        disconnect();

        DEBUG_LOG("%d", errno);

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

    return NoError;
}

} // namespace WlAnalyzer
