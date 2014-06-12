#ifndef INTERCEPTOR_H
#define INTERCEPTOR_H

#include <string>
#include <pthread.h>
#include <vector>
#include "socket.h"
#include "server_socket.h"
#include "connection.h"
#include "thread.h"

using std::vector;

class WldInterceptor : protected Thread
{
public:
    enum InterceptErr
    {
        NoError,
        ProxySocketErr
    };

public:
    WldInterceptor();
    ~WldInterceptor();

    bool createProxySocket(const std::string &name);
    bool runProxy();
    void stopProxy();

protected:
    virtual void run();

private:
    void createConnection(UnixLocalSocket *client);

private:
    vector<WlaConnection *> _connections;
    bool _configured;

    UnixLocalServer _interceptServerSocket;
    UnixLocalSocket _waylandSocket;
};

#endif // INTERCEPTOR_H
