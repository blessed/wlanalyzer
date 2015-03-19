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

#include <tr1/unordered_map>
#include <vector>
#include "common.h"
#include "xml/protocol_parser.h"

class WldProtocolAnalyzer
{
public:
    WldProtocolAnalyzer();
    ~WldProtocolAnalyzer();

    int addProtocolSpec(const std::string &path);
    int coreProtocol(const std::string &path);
    void lookup(uint32_t object_id, uint32_t opcode, WLD_MESSAGE_TYPE type, const char *payload);

private:
    struct NewId
    {
        uint32_t id;
        std::string interface;
    };

    int analyzeMessage(const WldInterface &intf, const WldMessage &msg, uint32_t obj_id, const char *payload);
    void extractArguments(const WldMessage &msg, const char *payload);
    bool createsObject(const WldMessage &msg);
    NewId getNewId(const WldMessage &msg, const char *payload);

private:
    WldProtocolDefinition *protocol;
    typedef std::tr1::unordered_map<uint32_t, std::string> names_t;
    typedef std::tr1::unordered_map<uint32_t, WldInterface> objects_t;
    objects_t objects;
    names_t names;
};

#endif // ANALYZER_H
