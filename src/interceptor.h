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

class WldInterceptor : public Thread
{
public:
    enum InterceptorErr
    {
        NoError,
        ConfigureErr,
        CreateSocketFail,
        ConnectToWaylandFail
    };

public:
    WldInterceptor();
    ~WldInterceptor();

    InterceptorErr swapSockets();

protected:
    virtual void run();

private:
    void createConnection(UnixLocalSocket *client);

private:
    static const std::string ORIG_WLD_SOCKET;
    vector<ClientConnection *> _connections;

    bool _configured;

    UnixLocalServer _interceptServerSocket;
    UnixLocalSocket _waylandSocket;
};

#endif // INTERCEPTOR_H
