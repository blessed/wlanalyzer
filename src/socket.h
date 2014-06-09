#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <sys/un.h>
#include <string>

enum UnixLocalSocketError
{
    ConnectionRefusedError,
    ServerNotFoundError,
    ConnectionError,
    SocketTimeoutError,
    SocketResourceError,
    UnsupportedSocketOperationError,
    UnknownSocketError,
    NoError
};

class UnixLocalSocket
{
public:
    UnixLocalSocket();
    ~UnixLocalSocket();

    UnixLocalSocketError connectToServer(const char *path);
    UnixLocalSocketError connectToServer(const std::string &path);
    UnixLocalSocketError disconnectFromServer();

    void setSocketDescriptor(int fd);
    int getSocketDescriptor() const
    {
        if (isConnected())
            return _fd;

        return -1;
    }

    bool isConnected() const { return _connected; }

    long read(char *data, long max_size);
    bool write(const char *data, long c);

private:
    void shutdown();

private:
    int _fd;
    std::string _path;
    bool _connected;
};

#endif // SOCKET_H
