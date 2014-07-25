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

#ifndef ANALYZER_H
#define ANALYZER_H

#include <vector>
#include "common.h"

typedef int32_t fixed_t;

enum WLD_ARG_TYPE
{
    WLD_ARG_UNKNOWN,
    WLD_ARG_INT,
    WLD_ARG_UINT,
    WLD_ARG_FIXED,
    WLD_ARG_STRING,
    WLD_ARG_OBJECT,
    WLD_ARG_NEWID,
    WLD_ARG_ARRAY,
    WLD_ARG_FD
};

enum WLD_MESSAGE_TYPE
{
    WLD_MSG_REQUEST,
    WLD_MSG_EVENT
};

union WldArgVal
{
    int32_t i;
    uint32_t u;
    fixed_t f;
    const char *s;
    void *o;
    uint32_t n;
    void *a;
    int32_t h;
};

struct WldArg
{
    WldArg()
    {
        name = "";
        type = WLD_ARG_UNKNOWN;
        value.i = 0;
    }

    std::string name;
    WLD_ARG_TYPE type;
    WldArgVal value;
};

struct WldMessage
{
    WLD_MESSAGE_TYPE type;
    std::string signature;
    std::vector<WldArg> args;
};

struct WldInterface
{
    uint32_t version;
    std::string name;
    std::vector<WldMessage> requests;
    std::vector<WldMessage> events;
};

#endif // ANALYZER_H
