
#include <iostream>
#include "socket.h"

using namespace std;

int main()
{
    UnixLocalSocket socket;
    UnixLocalSocketError err;
    if ((err = socket.connectToServer("/tmp/wayland-0")) != NoError)
    {
        cout << "There was some problem with connecting to server " << err << endl;
        return -1;
    }

    return 0;
}
