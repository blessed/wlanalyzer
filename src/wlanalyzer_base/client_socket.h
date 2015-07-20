#ifndef WLACLIENTSOCKET_H
#define WLACLIENTSOCKET_H

#include "socket.h"

namespace WlAnalyzer {

class WlaClientSocket : public WlaSocketBase
{
public:
    WlaClientSocket();
    WlaClientSocket(const WlaClientSocket &copy);
    explicit WlaClientSocket(int fd, bool connected = true);
    virtual ~WlaClientSocket();

    WlaClientSocket &operator=(const WlaClientSocket &other);

    void setFd(int fd, int flags = 0);

    SocketError connectTo(const char *path);
    SocketError connectTo(const std::string &path);
    SocketError disconnect();

    bool isConnected() const { return _connected; }

    long read(char *data, size_t max_size) const;
    bool write(const char *data, size_t c) const;

    int readMsg(msghdr *hdr);
    int writeMsg(const msghdr *hdr);

protected:
    virtual SocketError connect(const std::string &path);

private:
    bool _connected;
};

class WlaNetClientSocket : public WlaClientSocket
{
public:
    WlaNetClientSocket() { }
    WlaNetClientSocket(const WlaNetClientSocket &copy);
    virtual ~WlaNetClientSocket() { }

protected:
    SocketError connect(const std::string &path);
};

} // namespace WlAnalyzer

#endif // WLACLIENTSOCKET_H
