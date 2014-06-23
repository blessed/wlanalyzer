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
    if (revents % EV_ERROR)
    {
//        DEBUG_LOG("got error event");
        return;
    }

    if (!(revents & EV_READ))
    {
        DEBUG_LOG("no read events caught");
        return;
    }

    parse();
}

int WlaBinParser::parse()
{
    int res = 0;
    char hdr[HEADER_SIZE];
    WlaMessageBuffer *msg;

    while ((msg = nextMessage()) != NULL)
    {
//        WlaMessageHeader *msgHeader = getHeader(hdr);
    }
}

WlaMessageHeader *WlaBinParser::getHeader(char *buf)
{
    WlaMessageHeader *hdr = new WlaMessageHeader;

    hdr->client_id = (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
    hdr->size = buf[4] | (buf[5] << 8);
    hdr->opcode = buf[6] | (buf[7] << 8);

    return hdr;
}

WlaMessageBuffer *WlaBinParser::nextMessage()
{
    int len;
    WlaMessageBufferHeader *msg = new WlaMessageBufferHeader;

    while (len = read(file, msg, sizeof(WlaMessageBufferHeader)) == -1 && errno == EAGAIN)
    {
        continue;
    }

    if (len == -1)
    {
        DEBUG_LOG("failed to read from file");
        delete msg;
        return NULL;
    }

    WlaMessageBuffer *msg_buf = new WlaMessageBuffer;

    if (msg->flags & CMESSAGE_PRESENT)
    {
        uint32_t cmsg_len;

        read(file, &cmsg_len, sizeof(uint32_t));
        DEBUG_LOG("control message present (length %d)", cmsg_len);

        char *tmp = new char[cmsg_len];
        read(file, tmp, sizeof(char) * cmsg_len);
        delete [] tmp;
    }

    uint32_t msg_len;
    read(file, &msg_len, sizeof(uint32_t));
    DEBUG_LOG("msg payload size %d", msg_len);
    char *tmp = new char[msg_len];
    read(file, tmp, sizeof(char) * msg_len);

    delete [] tmp;

    return NULL;
}
