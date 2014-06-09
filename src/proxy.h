#ifndef PROXY_H
#define PROXY_H

class ProtocolInterceptor
{
public:
    ProtocolInterceptor();
    ~ProtocolInterceptor();

    bool setup();
    void start();
    void stop();

private:
    // TODO: add write_queue
};

#endif // PROXY_H
