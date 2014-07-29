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

#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include <tr1/unordered_map>
#include <vector>
#include <pugixml.hpp>

typedef int32_t fixed_t;

enum WldArgType
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

struct WldArgTypeHasher
{
    size_t
    operator()(WldArgType __val) const
    {
        return std::tr1::hash<int>()(static_cast<int>(__val));
    }
};

enum WLD_MESSAGE_TYPE
{
    WLD_MSG_UNKNOWN,
    WLD_MSG_REQUEST,
    WLD_MSG_EVENT
};

union WldArgVal
{
    int32_t i;
    uint32_t u;
    fixed_t f;
    const char *s;
    const void *o;
    uint32_t n;
    const void *a;
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
    WldArgType type;
    uint32_t size;
    WldArgVal value;
};

struct WldInterface;

struct WldMessage
{
    WldMessage()
    {
        type = WLD_MSG_UNKNOWN;
        signature = "";
    }

    std::string intf_name;
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

class WldProtocolDefinition
{
public:
    void addInterface(const WldInterface &interface)
    {
        interfaceList.push_back(interface);
    }

    const WldInterface *getInterface(const std::string &name) const;

private:
    std::vector<WldInterface> interfaceList;
};

class WldProtocolScanner
{
public:
    WldProtocolScanner();

    bool openProtocolFile(const std::string &path);
    WldProtocolDefinition *getProtocolDefinition(WldProtocolDefinition *protocolDef = NULL);

private:
    static void init();

    bool scanInterface(const pugi::xml_node &node, WldInterface &interface);
    bool getMessages(const pugi::xml_node &node, WldInterface &interface, WLD_MESSAGE_TYPE type);
    bool scanArgs(const pugi::xml_node &node, WldMessage &msg);

private:
    pugi::xml_document doc;
    typedef std::tr1::unordered_map<std::string, WldArgType> types_t;
    typedef std::tr1::unordered_map<WldArgType, size_t, WldArgTypeHasher> type_size_t;
    static types_t types;
    static type_size_t type_size;
    static bool initialized;
};

#endif // PROTOCOL_PARSER_H
