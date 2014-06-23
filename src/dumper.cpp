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

#include "message.h"
#include "dumper.h"

int WlaIODumper::seq = 0;

static void set_bit(uint32_t *val, int num, bool bit)
{
    *val = (*val & ~(1 << num)) | (bit << num);
}

static bool bit_isset(const uint32_t &val, int num)
{
    return val & (1 << num);
}

WlaIODumper::WlaIODumper()
{
    filefd = NULL;
}

WlaIODumper::~WlaIODumper()
{
    if (filefd)
        fclose(filefd);
}

int WlaIODumper::open(const std::string &path)
{
    if (path.empty())
    {
        DEBUG_LOG("failed: empty path");
        return -1;
    }

    if (filefd)
        fclose(filefd);

    filefd = ::fopen(path.c_str(), "w+");
    if (!filefd)
        return -1;
    else
        return 1;
}

// TODO: make the writer use strictly defined field bit lengths
// instead of sizeof's
int WlaIODumper::write(const WlaMessageBuffer &msg)
{
    if (!filefd)
        return -1;

    WlaMessageBufferHeader hdr;
    hdr.seq = seq++;

    if (msg.getType() == WlaMessageBuffer::EVENT_TYPE)
        set_bit(&hdr.flags, MESSAGE_EVENT_TYPE, true);

    if (msg.getControlMsgSize() > 0)
        set_bit(&hdr.flags, CMESSAGE_PRESENT, true);

    hdr.timestamp = *msg.getTimeStamp();

    ::fwrite(&hdr, sizeof(WlaMessageBufferHeader), 1, filefd);

    if (bit_isset(hdr.flags, CMESSAGE_PRESENT))
    {
        uint32_t cmsg_len = msg.getControlMsgSize();
        ::fwrite(&cmsg_len, sizeof(cmsg_len), 1, filefd);
        ::fwrite(msg.getControlMsg(), sizeof(uint8_t), cmsg_len, filefd);
        cmsg_len = 0xdeadbeef;
        ::fwrite(&cmsg_len, sizeof(cmsg_len), 1, filefd);
    }

    uint32_t msg_len = msg.getMsgSize();
    ::fwrite(&msg_len, sizeof(msg_len), 1, filefd);
    ::fwrite(msg.getMsg(), sizeof(uint8_t), msg_len, filefd);
    DEBUG_LOG("written %d msg bytes", msg_len);
    msg_len = 0xdeadbabe;
    ::fwrite(&msg_len, sizeof(msg_len), 1, filefd);

    return 0;
}
