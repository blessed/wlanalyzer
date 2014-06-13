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

#include <string.h>
#include "request.h"

WlaMessage::WlaMessage(const WlaMessage &copy)
{
    if (copy._size > 0)
    {
        _buf = new char[copy.size()];
        memcpy(_buf, copy._buf, copy.size());
        _size = copy._size;
    }
}

WlaMessage::~WlaMessage()
{
    if (_size)
    {
        delete _buf;
    }

    if (_hdr)
    {
        for (int i = 0; i < _hdr->msg_iovlen; i++)
        {
            delete [] (char *)_hdr->msg_iov[i].iov_base;
        }

        delete (cmsghdr *)_hdr->msg_control;
        delete _hdr->msg_iov;
        delete _hdr;
    }
}

void WlaMessage::setData(char *buf, int size)
{
    _buf = buf;
    _size = size;

    gettimeofday(&_timestamp, NULL);
}

void WlaMessage::setHeader(msghdr *hdr)
{
    if (!hdr)
        return;

    _hdr = hdr;
}

msghdr *WlaMessage::getHeader() const
{
    return _hdr;
}
