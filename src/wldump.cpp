#include <iostream>
#include "interceptor.h"

using namespace std;

int main(int argc, char *argv[])
{
    WldInterceptor interceptor;

    interceptor.swapSockets();
    interceptor.start();

    return 0;
}
