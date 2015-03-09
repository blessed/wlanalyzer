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
int WldNetDumper::seq = 0;

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

	filefd = ::open(resource.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR	| S_IRGRP | S_IROTH);
    if (filefd == -1)
	{
		DEBUG_LOG("Failed to create file %s", resource.c_str());
		perror(NULL);
        return -1;
	}

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
    client = -1;
}

WldNetDumper::~WldNetDumper()
{
    if (socket_fd != -1)
        ::close(socket_fd);

    if (client != -1)
        ::close(client);

    if (!message_queue.empty())
    {
        std::vector<WlaMessageBuffer *>::iterator it = message_queue.begin();
        while (it != message_queue.end())
        {
            delete (*it);
            it = message_queue.erase(it);
        }
    }
}

int WldNetDumper::open(const std::string &resource)
{
    if (server_socket.isListening())
    {
        DEBUG_LOG("Dumper already listening");
        return -1;
    }

    if (!server_socket.listen(resource))
        return false;

//    socket_watcher.set<WldNetDumper, &WldNetDumper::acceptClient>(this);
//    socket_watcher.start(server_socket.getFd(), EV_READ);

    bool timedout;
    server_socket.waitForConnection(0, &timedout);
    socket_watcher.fd = server_socket.getFd();
    acceptClient(socket_watcher, EV_READ);

//    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
//    if (socket_fd < 0)
//    {
//        DEBUG_LOG("failed to create socket");
//        return -1;
//    }

//    int val = 1;
//    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof val);

//    sockaddr_in srv_addr;
//    memset(&srv_addr, 0, sizeof(sockaddr_in));
//    srv_addr.sin_family = AF_INET;
//    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//    srv_addr.sin_port = htons(atoi(resource.c_str()));

//    if (bind(socket_fd, (sockaddr *)&srv_addr, sizeof(sockaddr_in)) < 0)
//    {
//        DEBUG_LOG("Failed to bind socket to port %s", resource.c_str());
//        close(socket_fd);
//        return -1;
//    }

//    if (listen(socket_fd, 5) < 0)
//    {
//        DEBUG_LOG("Failed to listen on server socket");
//        close(socket_fd);
//        return -1;
//    }

//    socket_watcher.set<WldNetDumper, &WldNetDumper::acceptClient>(this);
//    socket_watcher.start(socket_fd, EV_READ);

    return 0;
}

int WldNetDumper::dump(WlaMessageBuffer &msg)
{
    DEBUG_LOG("");

    if (!client_socket->isConnected())
    {
        DEBUG_LOG("No connection with client");

        WlaMessageBuffer *new_msg = new WlaMessageBuffer;
        *new_msg = msg;
        message_queue.push_back(new_msg);
        return 0;
    }

    int sequence_no = seq++;
    client_socket->write((const char *)&sequence_no, sizeof(sequence_no));

    char *buf = new char[WlaMessageBufferHeader::getSerializeSize()];
    memset(buf, 0, WlaMessageBufferHeader::getSerializeSize());
    int len = msg.getHeader()->serializeToBuf(buf, WlaMessageBufferHeader::getSerializeSize());

    if (!client_socket->write(buf, len))
        return -1;

    if (!client_socket->write(msg.getMsg(), msg.getMsgSize()))
        return -1;

    if (msg.getControlMsgSize() > 0)
    {
        if (!client_socket->write(msg.getControlMsg(), msg.getControlMsgSize()))
            return -1;
    }

//    if (client <= 0)
//    {
//        DEBUG_LOG("");

//        WlaMessageBuffer *new_msg = new WlaMessageBuffer;
//        *new_msg = msg;
//        message_queue.push_back(new_msg);
//        return 0;
//    }

//    int sequence_no = seq++;
//    if (!sendData((char *)&sequence_no, sizeof(seq)))
//        return -1;

//    char *buf = new char[WlaMessageBufferHeader::getSerializeSize()];
//    memset(buf, 0, WlaMessageBufferHeader::getSerializeSize());
//    int len = msg.getHeader()->serializeToBuf(buf, WlaMessageBufferHeader::getSerializeSize());

//    if (!sendData(buf, len))
//        return -1;

//    if (!sendData(msg.getMsg(), msg.getMsgSize()))
//        return -1;

//    if (msg.getControlMsgSize() > 0)
//    {
//        if (!sendData(msg.getControlMsg(), msg.getControlMsgSize()))
//            return -1;
//    }

//    int len;
//    do
//    {
//        len = send(client, msg.getMsg(), msg.getMsgSize(), 0);
//    } while (len < 0 && errno == EINTR || errno == EWOULDBLOCK);

//    DEBUG_LOG("");

//    if (len < 0 && errno == EPIPE)
//    {
//        Logger::getInstance()->log("Connection with analyzer broken, closing socket\n");
//        ::close(client);
//        client = -1;
//    }

    return 0;
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

//    bool timedout;
//    server_socket.waitForConnection(1000, &timedout);
    client_socket = server_socket.nextPendingConnection();
    socket_watcher.stop();

    if (message_queue.size() > 0)
    {
        DEBUG_LOG("");

        std::vector<WlaMessageBuffer *>::iterator it = message_queue.begin();
        for (; it != message_queue.end();)
        {
            DEBUG_LOG("%d", message_queue.size());

            int ret;

            int sequence_no = seq++;
            if (!client_socket->write((char *)&sequence_no, sizeof(seq)))
            {
                DEBUG_LOG("");
                return;
            }

            char *buf = new char[WlaMessageBufferHeader::getSerializeSize()];
            memset(buf, 0, WlaMessageBufferHeader::getSerializeSize());
            int len = (*it)->getHeader()->serializeToBuf(buf, WlaMessageBufferHeader::getSerializeSize());

            ret = client_socket->write(buf, len);
            delete [] buf;

            if (!ret)
            {
                DEBUG_LOG("");
                return;
            }

            if (!client_socket->write((*it)->getMsg(), (*it)->getMsgSize()))
            {
                DEBUG_LOG("");
                return;
            }

            if ((*it)->getControlMsgSize() > 0)
            {
                if (!client_socket->write((*it)->getControlMsg(), (*it)->getControlMsgSize()))
                {
                    DEBUG_LOG("");
                    return;
                }
            }

            delete *it;
            it = message_queue.erase(it);
        }
    }

//    int fd = accept(socket_fd, (sockaddr *)NULL, NULL);
//    if (fd < 0)
//    {
//        DEBUG_LOG("Failed to accept connection");
//        return;
//    }

//    socket_watcher.stop();
//    client = fd;

//    if (message_queue.size() > 0)
//    {
//        DEBUG_LOG("");

//        std::vector<WlaMessageBuffer *>::iterator it = message_queue.begin();
//        for (; it != message_queue.end(); it++)
//        {
//            DEBUG_LOG("%d", message_queue.size());

//            int ret;

//            int sequence_no = seq++;
//            if (!sendData((char *)&sequence_no, sizeof(seq)))
//            {
//                DEBUG_LOG("");
//                return;
//            }

//            char *buf = new char[WlaMessageBufferHeader::getSerializeSize()];
//            memset(buf, 0, WlaMessageBufferHeader::getSerializeSize());
//            int len = (*it)->getHeader()->serializeToBuf(buf, WlaMessageBufferHeader::getSerializeSize());

//            ret = sendData(buf, len);
//            delete [] buf;

//            if (!ret)
//            {
//                DEBUG_LOG("");
//                return;
//            }

//            if (!sendData((*it)->getMsg(), (*it)->getMsgSize()))
//            {
//                DEBUG_LOG("");
//                return;
//            }

//            if ((*it)->getControlMsgSize() > 0)
//            {
//                if (!sendData((*it)->getControlMsg(), (*it)->getControlMsgSize()))
//                {
//                    DEBUG_LOG("");
//                    return;
//                }
//            }

//            delete *it;
//        }

//        message_queue.clear();
//    }
}

int WldNetDumper::sendData(const char *data, size_t size)
{
    int len;

    do
    {
        len = send(client, data, size, MSG_NOSIGNAL);
    } while ((len < 0) && ((errno == EINTR) || (errno == EWOULDBLOCK)));

    if (len < 0)
    {
        if (errno == EPIPE)
        {
            Logger::getInstance()->log("Lost connection with analyzer, closing socket\n");
            ::close(client);
            client = -1;

            socket_watcher.start(socket_fd, EV_READ);

            return 0;
        }
        else
            return -1;
    }

    return len;
}
