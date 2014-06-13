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


#ifndef REQUEST_H
#define REQUEST_H

#include <sys/time.h>
#include <sys/socket.h>
#include "common.h"

class WlaMessage
{
public:

    enum WLA_MSG_TYPE
    {
        REQUEST_TYPE,
        EVENT_TYPE
    };

    WlaMessage(WLA_MSG_TYPE type) : _size(-1), _buf(NULL), _type(type), _hdr(NULL)
    {
    }

    WlaMessage(const WlaMessage &copy);
    ~WlaMessage();

    int size() const { return _size; }
    void setData(char *buf, int size);
    const char *data() const { return _buf; }

    void setHeader(msghdr *hdr);
    msghdr *getHeader() const;

private:
    int _size;
    char *_buf;
    msghdr *_hdr;
    WLA_MSG_TYPE _type;
    timeval _timestamp;
};

#endif // REQUEST_H
