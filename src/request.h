/*
 * Copyright © 2014 Samsung Electronics
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifndef REQUEST_H
#define REQUEST_H

#include <sys/time.h>
#include "common.h"

class WlaMessage
{
public:

    enum WLA_MSG_TYPE
    {
        REQUEST_TYPE,
        EVENT_TYPE
    };

    WlaMessage(WLA_MSG_TYPE type) : _size(-1), _buf(NULL), _type(type)
    {
    }

    WlaMessage(const WlaMessage &copy);
    ~WlaMessage();

    int size() const { return _size; }
    void setData(char *buf, int size);
    const char *data() const { return _buf; }

private:
    int _size;
    char *_buf;
    WLA_MSG_TYPE _type;
    timeval _timestamp;
};

#endif // REQUEST_H
