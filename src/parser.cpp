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

#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "dumper.h"
#include "parser.h"

WldParser::WldParser() : analyzer(NULL)
{
}

WldParser::~WldParser()
{
    if (analyzer)
        delete analyzer;
}

void WldParser::attachAnalyzer(WldProtocolAnalyzer *analyzer)
{
    if (this->analyzer)
        delete this->analyzer;

    this->analyzer = analyzer;
}

int WldParser::parse()
{
    WlaMessageBuffer *msg;

    while ((msg = nextMessage()) != NULL)
    {
        parseMessage(msg);

        delete msg;
    }

    return 0;
}

void WldParser::parseMessage(WlaMessageBuffer *msg)
{
    const char *msg_buf = msg->getMsg();
    uint32_t i = 0;

    char timestr[64];
    time_t nowtime;
    tm *nowtm;
    nowtime = msg->getTimeStamp()->tv_sec;
    nowtm = localtime(&nowtime);
    strftime(timestr, sizeof(timestr), "%H:%M:%S", nowtm);

    while (i < msg->getMsgSize())
    {
        uint32_t client_id = byteArrToUInt32(&msg_buf[CLIENT_ID_OFFSET]);
        uint16_t size = byteArrToUInt16(&msg_buf[SIZE_OFFSET]);
        uint16_t opcode = byteArrToUInt16(&msg_buf[OPCODE_OFFSET]);


//        Logger::getInstance()->log("%s msg (%s.%03d), id %d, opcode %d, size %d\n",
//                  msg->getType() == WlaMessageBuffer::EVENT_TYPE ? "event" : "request",
//                  timestr, msg->getTimeStamp()->tv_usec / 1000,
//                  client_id, opcode, size);


        if (analyzer)
        {
            WLD_MESSAGE_TYPE type;
            if (msg->getType() == WlaMessageBuffer::EVENT_TYPE)
                type = WLD_MSG_EVENT;
            else
                type = WLD_MSG_REQUEST;
            analyzer->lookup(client_id, opcode, type, msg_buf + PAYLOAD_OFFSET);
        }

        if (size == 0)
            break;

        msg_buf += size;
        i += size;
    }
}

WlaBinParser::WlaBinParser()
{
    file = -1;
}

WlaBinParser::~WlaBinParser()
{
    close(file);
}

int WlaBinParser::openResource(const std::string &path)
{
    if (path.empty())
    {
        DEBUG_LOG("path is empty");
        return -1;
    }

    if (file != -1)
        close(file);

    file = open(path.c_str(), O_RDONLY);
    if (file == -1)
    {
        DEBUG_LOG("failed to open file %s", path.c_str());
        perror(NULL);
        return -1;
    }

    timer.set<WlaBinParser, &WlaBinParser::timerEvent>(this);
    filewtch.set<WlaBinParser, &WlaBinParser::handleFileEvent>(this);

    return 0;
}

void WlaBinParser::enable(bool state)
{
    if (state)
        filewtch.start(file, EV_READ);
    else
        filewtch.stop();
}

void WlaBinParser::handleFileEvent(ev::io &watcher, int revents)
{
    if (revents & EV_ERROR)
    {
        DEBUG_LOG("got error event");
        return;
    }

    if (revents & EV_READ)
    {
        parse();
    }
}

void WlaBinParser::timerEvent(ev::timer &timer, int revents)
{
    if (revents & EV_ERROR)
    {
        DEBUG_LOG("error");
        return;
    }

    filewtch.start(file, EV_READ);
//    DEBUG_LOG("timer callback");

    timer.stop();
}

WlaMessageBuffer *WlaBinParser::nextMessage()
{
    uint32_t len;
    WlaMessageBuffer *msg = new WlaMessageBuffer;

    uint32_t seq;
    read(file, &seq, sizeof(uint32_t));

    uint32_t size = msg->getHeader()->getSerializeSize();
    char *buf = new char[size];

    while ((len = read(file, buf, size)) < 0 && errno == EAGAIN)
    {
        continue;
    }

    if (len < 0)
    {
        delete msg;
        return NULL;
    }
    else if (len == 0)
    {
        timer.start(0.2, 0.0);
        filewtch.stop();

        delete msg;
        return NULL;
    }

    if (len < size)
    {
        lseek(file, -len, SEEK_CUR);
        delete [] buf;
        delete msg;
        return NULL;
    }

    msg->getHeader()->deserializeFromBuf(buf, size);
    delete [] buf;

    lseek(file, 0, SEEK_CUR);

    char *msg_buf = new char[msg->getMsgSize()];
    if ((len = read(file, msg_buf, msg->getMsgSize())) < 0)
    {
        DEBUG_LOG("failed to read from file");
        delete [] msg_buf;
        delete [] msg;
        return NULL;
    }
    else if (len < msg->getMsgSize())
    {
        DEBUG_LOG("too short");
    }

    msg->setMsg(msg_buf, msg->getMsgSize());
    delete [] msg_buf;

    if (bit_isset(msg->getHeader()->flags, CMESSAGE_PRESENT_BIT))
    {
        char *cmsg_buf = new char[msg->getControlMsgSize()];
        len = read(file, cmsg_buf, msg->getControlMsgSize());
        if ( len < msg->getControlMsgSize())
            DEBUG_LOG("too short");
        msg->setControlMsg(cmsg_buf, msg->getControlMsgSize());
        delete [] cmsg_buf;
    }

    DEBUG_LOG("seq %d flags %d size %d", seq, msg->getHeader()->flags, msg->getHeader()->msg_len);

    return msg;
}

WldNetParser::WldNetParser()
{
}

WldNetParser::~WldNetParser()
{
    socket.disconnectFromServer();
}

int WldNetParser::openResource(const std::string &resource)
{
    if (resource.empty())
    {
        DEBUG_LOG("address data empty");
        return -1;
    }

    if (socket.isConnected())
        socket.disconnectFromServer();

    int r = socket.connectToServer(resource);
    if (r != NoError)
    {
        DEBUG_LOG("Failed to connect to server %s %d", resource.c_str(), r);
        return -1;
    }

    socketwtch.set<WldNetParser, &WldNetParser::handleSocketEvent>(this);
    socketwtch.start(socket.getSocketDescriptor(), EV_READ);

    return 0;
}

void WldNetParser::handleSocketEvent(ev::io &watcher, int revents)
{
    if (revents & EV_ERROR)
    {
        DEBUG_LOG("error from libev");
        return;
    }

    if (revents & EV_READ)
    {
        parse();
    }
}

WlaMessageBuffer *WldNetParser::nextMessage()
{
    WlaMessageBuffer *msg = new WlaMessageBuffer;
    int bytes;

    uint32_t seq;
    bytes = socket.readUntil((char *)&seq, sizeof(seq));
    DEBUG_LOG("Read %d of %d bytes", bytes, sizeof(seq));

    uint32_t size = msg->getHeader()->getSerializeSize();
    char *buf = new char[size];
    memset(buf, 0, size);

    bytes = socket.readUntil(buf, size);
    DEBUG_LOG("Read %d of %d bytes", bytes, size)

    msg->getHeader()->deserializeFromBuf(buf, size);
    delete [] buf;

    DEBUG_LOG("%d", msg->getMsgSize());
    char *msg_buf = new char[msg->getMsgSize()];
    bytes = socket.readUntil(msg_buf, msg->getMsgSize());
    DEBUG_LOG("Read %d of %d bytes", bytes, msg->getMsgSize());

    msg->setMsg(msg_buf, msg->getMsgSize());
    delete [] msg_buf;

    if (bit_isset(msg->getHeader()->flags, CMESSAGE_PRESENT_BIT))
    {
        char *cmsg_buf = new char[msg->getControlMsgSize()];
        bytes = socket.readUntil(cmsg_buf, msg->getControlMsgSize());
        DEBUG_LOG("Read %d bytes", bytes)
        msg->setControlMsg(cmsg_buf, msg->getControlMsgSize());
        delete [] cmsg_buf;
    }

    DEBUG_LOG("seq %d flags %d size %d", seq, msg->getHeader()->flags, msg->getHeader()->msg_len);

    return msg;
}


