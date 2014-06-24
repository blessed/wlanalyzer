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

#include <sys/stat.h>
#include <fcntl.h>
#include "dumper.h"
#include "parser.h"

WlaBinParser::WlaBinParser()
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

    set<WlaBinParser, &WlaBinParser::handleFileEvent>(this);
    start(file, EV_READ);

    return 0;
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

    if (len <= 0)
    {
        delete msg;
        return NULL;
    }

    if (len < sizeof(WlaMessageBufferHeader))
    {
        lseek(file, -len, SEEK_CUR);
        delete msg;
        return NULL;
    }

    int pos = lseek(file, 0, SEEK_CUR);

    DEBUG_LOG("pos in file %d (%d) seq %d message size %d flags %x", pos, len, seq, msg->getMsgSize(), msg->getHeader()->flags);

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
        DEBUG_LOG("cmsg present");
        char *cmsg_buf = new char[msg->getControlMsgSize()];
        len = read(file, cmsg_buf, msg->getControlMsgSize());
        if ( len < msg->getControlMsgSize())
            DEBUG_LOG("too short");
        msg->setControlMsg(cmsg_buf, msg->getControlMsgSize());
        delete [] cmsg_buf;
    }

    return msg;
}

void WlaBinParser::parseMessage(WlaMessageBuffer *msg)
{
    const char *msg_buf = msg->getMsg();
    int i = 0;

    while (i < msg->getMsgSize())
    {
        uint32_t client_id = byteArrToUInt32(&msg_buf[CLIENT_ID_OFFSET]);
        uint16_t size = byteArrToUInt16(&msg_buf[SIZE_OFFSET]);
        uint16_t opcode = byteArrToUInt16(&msg_buf[OPCODE_OFFSET]);

        DEBUG_LOG("client id = %d, size = %d, opcode = %d", client_id,
                  size, opcode);

        if (size == 0)
            break;

        msg_buf += size;
        i += size;
    }
}
