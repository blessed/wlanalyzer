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

#include <string.h>
#include "analyzer.h"

WldProtocolAnalyzer::WldProtocolAnalyzer() : protocol(NULL)
{
}

WldProtocolAnalyzer::~WldProtocolAnalyzer()
{
    if (protocol)
        delete protocol;
}

int WldProtocolAnalyzer::addProtocolSpec(const std::string &path)
{
    WldProtocolScanner scanner;
    if (!scanner.openProtocolFile(path))
        return -1;

    protocol = scanner.getProtocolDefinition(protocol);
    if (!protocol)
        return -1;

    return 0;
}

int WldProtocolAnalyzer::coreProtocol(const std::string &path)
{
    if (addProtocolSpec(path))
        return -1;

    WldInterface dummy;
    dummy.name = "NULL";
    dummy.version = 0;

    const WldInterface *display = protocol->getInterface("wl_display");
    if (!display)
    {
        DEBUG_LOG("");
        return -1;
    }
    const WldInterface *proxy = protocol->getInterface("wl_registry");
    if (!proxy)
    {
        DEBUG_LOG("");
        return -1;
    }

    DEBUG_LOG("");

    objects[0] = dummy;
    objects[1] = *display;
    objects[2] = *proxy;
}

void WldProtocolAnalyzer::lookup(uint32_t object_id, uint32_t opcode, WLD_MESSAGE_TYPE type, const char *payload)
{
    const WldMessage *msg = NULL;

    objects_t::const_iterator it = objects.find(object_id);
    if (it == objects.end())
    {
        Logger::getInstance()->log("Unknown message type @%u:%u\n", object_id, opcode);
        return;
    }

    const WldInterface &intf = it->second;

    if (type == WLD_MSG_REQUEST)
    {
        if (opcode > intf.requests.size())
        {
            DEBUG_LOG("%d", intf.requests.size());
            return;
        }
        msg = &intf.requests[opcode];
    }
    else
    {
        if (opcode > intf.events.size())
        {
            DEBUG_LOG("%d", intf.requests.size());
            return;
        }
        msg = &intf.events[opcode];
    }

    if (!msg)
    {
        DEBUG_LOG("Couldn't retrieve message %d:%d of %s", object_id, opcode, intf.name.c_str());
        return;
    }

    Logger::getInstance()->log("%s@%u.%s()\n", intf.name.c_str(), object_id, msg->signature.c_str());
    analyzeMessage(*msg, payload);
}

int WldProtocolAnalyzer::analyzeMessage(const WldMessage &msg, const char *payload)
{
    if (msg.type == WLD_MSG_EVENT && msg.intf_name == "wl_registry" &&
            msg.signature == "global")
    {
        const char *p = payload;

        uint32_t id = byteArrToUInt32(p);
        p += 4;

        uint32_t name_len = byteArrToUInt32(p);
        p += 4;
        char *name = new char[name_len];
        strncpy(name, p, name_len);

        std::string strName = name;

        delete [] name;

        names[id] = strName;

        Logger::getInstance()->log("Found new name %d->%s\n", id, strName.c_str());
    }
    else if (msg.type == WLD_MSG_REQUEST && msg.intf_name == "wl_registry" &&
             msg.signature == "bind")
    {
        const char *p = payload;

        uint32_t id = byteArrToUInt32(p);
        p += 4;

        uint32_t name_len = byteArrToUInt32(p);
        p += 4;

        char *name = new char[name_len];
        strncpy(name, p, name_len);
        p += name_len;
        std::string strName = name;
        delete [] name;

        if ((uintptr_t)p % 4)
        {
            p += 4 - ((uintptr_t)p % 4);
        }

        uint32_t ver = byteArrToUInt32(p);
        p += 4;

        uint32_t new_id = byteArrToUInt32(p);

        const WldInterface *intf = protocol->getInterface(strName);
        if (intf)
        {
            objects[new_id] = *intf;

            Logger::getInstance()->log("%s.", msg.intf_name.c_str());
            Logger::getInstance()->log("%s(", msg.signature.c_str());
            Logger::getInstance()->log("%u, \"%s\", %u, new id@%u)\n", id, intf->name.c_str(), ver, new_id);
//            Logger::getInstance()->log("id %d, new_id %d, name %s, ver %d\n", id, new_id, intf->name.c_str(), ver);
        }

        // print available objects
        /*
        DEBUG_LOG("Available objects:");
        objects_t::const_iterator it = objects.begin();
        for (; it != objects.end(); it++)
        {
            DEBUG_LOG("%u: %s", it->first, it->second.name.c_str());
        }
        */
    }

    return -1;
}
