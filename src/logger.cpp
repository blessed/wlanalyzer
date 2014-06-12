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
