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
