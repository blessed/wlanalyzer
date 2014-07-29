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

WlaBinParser::WlaBinParser() : analyzer(NULL)
{
    file = -1;
}

WlaBinParser::~WlaBinParser()
{
    close(file);
}

int WlaBinParser::openFile(const std::string &path)
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
    filewtch.start(file, EV_READ);

    return 0;
}

void WlaBinParser::attachAnalyzer(WldProtocolAnalyzer *analyzer)
{
    this->analyzer = analyzer;
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

int WlaBinParser::parse()
{
    WlaMessageBuffer *msg;

    while ((msg = nextMessage()) != NULL)
    {
        parseMessage(msg);

        delete msg;
    }

    return 0;
}

WlaMessageBuffer *WlaBinParser::nextMessage()
{
    int len;
    WlaMessageBuffer *msg = new WlaMessageBuffer;

    uint32_t seq;
    read(file, &seq, sizeof(uint32_t));

    while ((len = read(file, msg->getHeader(), sizeof(WlaMessageBufferHeader))) < 0 && errno == EAGAIN)
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

    if (len < sizeof(WlaMessageBufferHeader))
    {
        lseek(file, -len, SEEK_CUR);
        delete msg;
        return NULL;
    }

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

void WlaBinParser::parseMessage(WlaMessageBuffer *msg)
{
    const char *msg_buf = msg->getMsg();
    int i = 0;

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

        /*
        Logger::getInstance()->log("%s msg (%s.%03d), id %d, opcode %d, size %d\n",
                  msg->getType() == WlaMessageBuffer::EVENT_TYPE ? "event" : "request",
                  timestr, msg->getTimeStamp()->tv_usec / 1000,
                  client_id, opcode, size);
        */

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
