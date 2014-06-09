#ifndef INTERCEPTOR_H
#define INTERCEPTOR_H

#include "server_socket.h"

class WldInterceptor
{
public:
    enum InterceptorErr
    {
        NoError,
        ConfigureErr
    };

public:
    WldInterceptor();
    ~WldInterceptor();

    InterceptorErr configure();
    int start();
    int stop();

private:
    bool _configured;

    UnixLocalServer _interceptSocket;
    UnixLocalSocket _waylandSocket;
};

#endif // INTERCEPTOR_H
