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

#ifndef PARSER_H
#define PARSER_H

#include <ev++.h>
#include "message.h"
#include "common.h"
#include "analyzer.h"

class WldParser
{
public:
    WldParser();
    virtual ~WldParser();

    void attachAnalyzer(WldProtocolAnalyzer *analyzer);
    virtual int openResource(const std::string &resource) = 0;
    int parse();
	void enable(bool state = true)
	{
	}

protected:
    virtual WlaMessageBuffer *nextMessage() = 0;
    void parseMessage(WlaMessageBuffer *msg);

protected:
    WldProtocolAnalyzer *analyzer;
};

// TODO: Refactor WlaBinParser so that it holds an internal message queue.
// Make nextMessage() generic!
class WlaBinParser : public WldParser
{
public:
    WlaBinParser();
    ~WlaBinParser();

    int openResource(const std::string &path);
//    void attachAnalyzer(WldProtocolAnalyzer *analyzer);
    void enable(bool state = true);

private:
    void handleFileEvent(ev::io &watcher, int revents);
    void timerEvent(ev::timer &timer, int revents);
    WlaMessageBuffer *nextMessage();

private:
    ev::timer timer;
    int file;
    ev::io filewtch;
};

class WldNetParser : public WldParser
{
public:
    WldNetParser();
    ~WldNetParser();

    int openResource(const std::string &resource);

private:
    void handleSocketEvent(ev::io &watcher, int revents);
    WlaMessageBuffer *nextMessage();

private:
    WldNetSocket socket;
    ev::io socketwtch;
};

#endif // PARSER_H
