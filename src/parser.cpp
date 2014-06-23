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
        DEBUG_LOG("got message");
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

    DEBUG_LOG("seq %d, msg %d, cmsg %d", seq, msg->getMsgSize(), msg->getControlMsgSize());
    DEBUG_LOG("timestamp %d.%d", msg->getHeader()->timestamp.tv_sec, msg->getHeader()->timestamp.tv_usec);
    DEBUG_LOG("flags %x", msg->getHeader()->flags);

    lseek(file, msg->getMsgSize(), SEEK_CUR);

    if (bit_isset(msg->getHeader()->flags, CMESSAGE_PRESENT_BIT))
    {
        lseek(file, msg->getControlMsgSize(), SEEK_CUR);
    }

    return msg;
}
