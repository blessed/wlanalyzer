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

#include "thread.h"

Thread::Thread()
{
    pthread_mutex_init(&_running_mutex, NULL);
}

Thread::~Thread()
{
    pthread_mutex_destroy(&_running_mutex);
}

bool Thread::start()
{
    pthread_mutex_lock(&_running_mutex);
    if (!_running)
    {
        _running = true;
        pthread_mutex_unlock(&_running_mutex);
    }
    else
    {
        pthread_mutex_unlock(&_running_mutex);
        return false;
    }

    return (pthread_create(&_thread, NULL, entry, this) == 0);
}

void Thread::stop()
{
    pthread_mutex_lock(&_running_mutex);
    if (_running)
    {
        _running = false;
        pthread_mutex_unlock(&_running_mutex);
    }
    else
    {
        pthread_mutex_unlock(&_running_mutex);
        return;
    }

    pthread_join(_thread, NULL);
}

void *Thread::entry(void *This)
{
    ((Thread *)This)->run();

    return NULL;
}
