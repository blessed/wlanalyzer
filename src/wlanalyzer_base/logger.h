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


#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

#ifndef DEBUG_BUILD
#define DEBUG_LOG(msg, ...)
#else
#define DEBUG_LOG(msg, ...) \
    Logger::getInstance()->log("%s:%d-> " msg "\n", __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif // DEBUG_BUILD

namespace WlAnalyzer {

class Logger
{
public:

    static Logger *getInstance();

    int log(const char *format, ...);

private:
    Logger();
    ~Logger();

    void open();
    void close();

private:
    static Logger *_instance;
    FILE *_fd;
};

} // WlAnalyzer

#endif // LOGGER_H
