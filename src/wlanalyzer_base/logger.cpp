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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "logger.h"

Logger *Logger::_instance = NULL;

Logger::~Logger()
{
    if (_instance)
        delete _instance;
}

Logger *Logger::getInstance()
{
    if (!_instance)
        _instance = new Logger;

    return _instance;
}

int Logger::log(const char *format, ...)
{
    int l;

    va_list vargs;
    va_start(vargs, format);
    l = vfprintf(_fd, format, vargs);
    va_end(vargs);

    return l;
}

Logger::Logger()
{
    open();
}

void Logger::open()
{
    const char *env = getenv("WLANALYZER_LOGFILE");
    if (env)
    {
        FILE *file = fopen(env, "w+");
        if (!file)
            _fd = stderr;
        else
            _fd = file;
    }
    else
        _fd = stderr;
}

void Logger::close()
{
    if (_fd != stderr)
        fclose(_fd);
}
