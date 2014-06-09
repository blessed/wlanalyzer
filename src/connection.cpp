/*
 * Copyright © 2014 Samsung Electronics
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include <errno.h>
#include <sys/select.h>
#include "connection.h"

ClientConnection::ClientConnection(UnixLocalSocket *client, UnixLocalSocket *server)
{
    if (!(client && server))
        return;

    _clientSocket = client;
    _serverSocket = server;
}

ClientConnection::~ClientConnection()
{
    stop();
}

void ClientConnection::run()
{
    fd_set readfds;

    while (_running)
    {
        FD_ZERO(&readfds);
        FD_SET(_clientSocket->getSocketDescriptor(), &readfds);
        FD_SET(_serverSocket->getSocketDescriptor(), &readfds);

        int max_fd = _clientSocket->getSocketDescriptor() > _serverSocket->getSocketDescriptor()
                ? _clientSocket->getSocketDescriptor() : _serverSocket->getSocketDescriptor();

        while (select(max_fd + 1, &readfds, NULL, NULL, NULL) == -1)
        {
            if (errno == EINTR)
                continue;

            break;
        }

        if (FD_ISSET(_clientSocket->getSocketDescriptor(), &readfds))
            passOn(_clientSocket, _serverSocket);

        if (FD_ISSET(_serverSocket->getSocketDescriptor(), &readfds))
            passOn(_serverSocket, _clientSocket);
    }
}

void ClientConnection::passOn(UnixLocalSocket *src, UnixLocalSocket *dst)
{
    char buf[1024];
    int len;

    len = src->read(buf, 1024);
    dst->write(buf, len);
}
