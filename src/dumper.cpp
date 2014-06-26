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

#include <fcntl.h>
#include "message.h"
#include "dumper.h"

int WlaIODumper::seq = 0;

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

    if (filefd == -1)
        close(filefd);

    filefd = ::open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC);
    if (filefd == -1)
        return -1;
    else
        return 1;
}

// TODO: make the writer use strictly defined field bit lengths
// instead of sizeof's
int WlaIODumper::write(WlaMessageBuffer &msg)
{
    if (!filefd)
        return -1;

    ::write(filefd, &seq, sizeof(uint32_t));
    seq++;

    ::write(filefd, msg.getHeader(), sizeof(WlaMessageBufferHeader));
    ::write(filefd, msg.getMsg(), sizeof(uint8_t) * msg.getMsgSize());
    if (msg.getControlMsgSize() > 0)
        ::write(filefd, msg.getControlMsg(), sizeof(uint8_t) * msg.getControlMsgSize());

    return 0;
}
