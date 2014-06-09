#include <iostream>

#include "socket.h"
#include "server_socket.h"

using namespace std;

int main(int argc, char *argv[])
{
    UnixLocalServer server;
    if (!server.listen("/tmp/wayland-0"))
    {
        cout << "There was some error when creating the server" << endl;
        return -1;
    }

    bool failed;
    if (!server.waitForConnection(100, &failed))
    {
        if (failed)
            cout << "Error on timeout" << endl;
    }

    UnixLocalSocket *client = server.nextPendingConnection();
    if (client->isConnected())
        cout << "happily connected" << endl;

    return 0;
}
