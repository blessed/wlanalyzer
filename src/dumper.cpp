/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Samsung Electronics
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <arpa/inet.h>
#include <fcntl.h>
#include "message.h"
#include "dumper.h"

int WlaIODumper::seq = 0;
int WldIODumper::seq = 0;

WlaIODumper::WlaIODumper()
{
    filefd = -1;
}

WlaIODumper::~WlaIODumper()
{
    if (filefd != -1)
        close(filefd);
}

int WlaIODumper::open(const std::string &path)
{
    if (path.empty())
    {
        DEBUG_LOG("failed: empty path");
        return -1;
    }

    if (filefd != -1)
        close(filefd);

    filefd = ::open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC);
    if (filefd == -1)
        return -1;
    else
        return 1;
}

int WlaIODumper::write(WlaMessageBuffer &msg)
{
    if (!filefd)
        return -1;

    DEBUG_LOG("");

    ::write(filefd, &seq, sizeof(uint32_t));
    seq++;

    ::write(filefd, msg.getMsg(), sizeof(uint8_t) * msg.getMsgSize());
    if (msg.getControlMsgSize() > 0)
        ::write(filefd, msg.getControlMsg(), sizeof(uint8_t) * msg.getControlMsgSize());

    return 0;
}


int WldIODumper::open(const std::string &resource)
{
    if (resource.empty())
    {
        DEBUG_LOG("failed: empty path");
        return -1;
    }

    if (filefd != -1)
        close(filefd);

    filefd = ::open(resource.c_str(), O_RDWR | O_CREAT | O_TRUNC);
    if (filefd == -1)
        return -1;
    else
        return 1;
}

int WldIODumper::dump(WlaMessageBuffer &msg)
{
    if (!filefd)
        return -1;

    ::write(filefd, &seq, sizeof(uint32_t));
    seq++;

    char *buf = new char[WlaMessageBufferHeader::getSerializeSize()];
    memset(buf, 0, WlaMessageBufferHeader::getSerializeSize());

    int len = msg.getHeader()->serializeToBuf(buf, WlaMessageBufferHeader::getSerializeSize());

    ::write(filefd, buf, len);
    ::write(filefd, msg.getMsg(), sizeof(uint8_t) * msg.getMsgSize());
    if (msg.getControlMsgSize() > 0)
        ::write(filefd, msg.getControlMsg(), sizeof(uint8_t) * msg.getControlMsgSize());

    delete [] buf;

    return 0;
}


WldNetDumper::WldNetDumper()
{
    socket_fd = -1;
}

WldNetDumper::~WldNetDumper()
{
    if (socket_fd != -1)
        ::close(socket_fd);
}

int WldNetDumper::open(const std::string &resource)
{
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        DEBUG_LOG("failed to create socket");
        return -1;
    }

    sockaddr_in srv_addr;
    memset(&srv_addr, 0, sizeof(sockaddr_in));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons(atoi(resource.c_str()));

    if (bind(socket_fd, (sockaddr *)&srv_addr, sizeof(sockaddr_in)) < 0)
    {
        DEBUG_LOG("Failed to bind socket to port %s", resource.c_str());
        close(socket_fd);
        return -1;
    }

    if (listen(socket_fd, 5) < 0)
    {
        DEBUG_LOG("Failed to listen on server socket");
        close(socket_fd);
        return -1;
    }

    socket_watcher.set<WldNetDumper, &WldNetDumper::acceptClient>(this);
    socket_watcher.start(socket_fd, EV_READ);

    return 0;
}

int WldNetDumper::dump(WlaMessageBuffer &msg)
{
    if (clients.empty())
        return 0;

    std::vector<int>::iterator it = clients.begin();
    for (; it != clients.begin(); it++)
    {
        int len;
        do
        {
            len = send(*it, msg.getMsg(), msg.getMsgSize(), 0);
        } while (len < 0 && errno == EINTR || errno == EWOULDBLOCK);

        if (len < 0 && errno == EPIPE)
        {
            Logger::getInstance()->log("Connection with analyzer broken, closing socket\n");
            ::close((*it));
            clients.erase(it);
        }
    }
}

bool WldNetDumper::validateIpAddress(const std::string &ipAddress)
{
    sockaddr_in sock;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sock.sin_addr));

    return result != 0;
}

void WldNetDumper::acceptClient(ev::io &watcher, int revents)
{
    if (revents & EV_ERROR)
        return;

    DEBUG_LOG("");

    int fd = accept(socket_fd, (sockaddr *)NULL, NULL);
    if (fd < 0)
    {
        DEBUG_LOG("Failed to accept connection");
        return;
    }

    clients.push_back(fd);
}
