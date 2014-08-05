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

#ifndef DUMPER_H
#define DUMPER_H

#include <vector>
#include <ev++.h>
#include "common.h"

class WlaMessageBuffer;

class WldDumper
{
public:
    virtual int open(const std::string &resource) = 0;
    virtual int dump(WlaMessageBuffer &msg) = 0;
};

class WldIODumper : public WldDumper
{
public:
    WldIODumper() { filefd = -1; }
    virtual ~WldIODumper() { if (filefd != -1) close(filefd); }

    virtual int open(const std::string &resource);
    virtual int dump(WlaMessageBuffer &msg);

private:
    int filefd;
    static int seq;
};

class WldNetDumper : public WldDumper
{
public:
    WldNetDumper();
    virtual ~WldNetDumper();

    virtual int open(const std::string &resource);
    virtual int dump(WlaMessageBuffer &msg);

private:
    bool validateIpAddress(const std::string &ipAddress);
    void acceptClient(ev::io &watcher, int revents);

private:
    int socket_fd;
    std::vector<int> clients;
    ev::io socket_watcher;
};

class WlaIODumper
{
public:
    WlaIODumper();
    ~WlaIODumper();

    int open(const std::string &path);
    int write(WlaMessageBuffer &msg);

private:
    int filefd;
    static int seq;
};

#endif // DUMPER_H
