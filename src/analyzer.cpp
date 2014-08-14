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

    return 0;
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
        if (opcode >= intf.requests.size())
        {
            DEBUG_LOG("The request %d/%d for %s interface is invalid", opcode, intf.requests.size(),
                      intf.name.c_str());
            return;
        }
        msg = &intf.requests[opcode];
    }
    else
    {
        if (opcode >= intf.events.size())
        {
            DEBUG_LOG("The event %d/%d for %s interface is invalid", opcode, intf.events.size(),
                      intf.name.c_str());
            return;
        }
        msg = &intf.events[opcode];
    }

    if (!msg)
    {
        DEBUG_LOG("Couldn't retrieve message %d:%d of %s", object_id, opcode, intf.name.c_str());
        return;
    }

    Logger::getInstance()->log("%s@%u.%s@%u()\n", intf.name.c_str(), object_id, msg->signature.c_str(), opcode);
    analyzeMessage(intf, *msg, object_id, payload);
}

int WldProtocolAnalyzer::analyzeMessage(const WldInterface &intf, const WldMessage &msg, uint32_t obj_id, const char *payload)
{
    if (msg.type == WLD_MSG_EVENT && intf.name == "wl_registry" &&
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
    else if (intf.name == "wl_registry" &&
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

        const WldInterface *prot_intf = protocol->getInterface(strName);
        if (prot_intf)
        {
            objects[new_id] = *prot_intf;

            Logger::getInstance()->log("%s.", msg.intf_name.c_str());
            Logger::getInstance()->log("%s(", msg.signature.c_str());
            Logger::getInstance()->log("%u, \"%s\", %u, new id@%u)\n", id, prot_intf->name.c_str(), ver, new_id);
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
    else if (msg.signature == "destroy")
    {
        objects.erase(obj_id);
    }
    else
    {
        extractArguments(msg, payload);
    }

    return 0;
}

void WldProtocolAnalyzer::extractArguments(const WldMessage &msg, const char *payload)
{
    const char *p = payload;

    std::vector<WldArg>::const_iterator it = msg.args.begin();
    for (; it != msg.args.end(); it++)
    {
        if (it->type == WLD_ARG_NEWID)
        {
            const WldInterface *intf = protocol->getInterface(it->interface);
            if (!intf)
                continue;

            int id = byteArrToUInt32(p);
            p += 4;
            objects[id] = *intf;
        }
        else if (it->type == WLD_ARG_STRING)
        {
            int len = byteArrToUInt32(p);
            p += 4;
            p += len;
        }
        else if (it->type == WLD_ARG_ARRAY)
        {
            int len = byteArrToUInt32(p);
            p += 4;
            p += len;
        }
        else
        {
            p += 4;
        }
    }
}

bool WldProtocolAnalyzer::createsObject(const WldMessage &msg)
{
    std::vector<WldArg>::const_iterator it = msg.args.begin();
    for (; it != msg.args.end(); it++)
    {
        if (it->type == WLD_ARG_NEWID)
            return true;
    }

    return false;
}

WldProtocolAnalyzer::NewId WldProtocolAnalyzer::getNewId(const WldMessage &msg, const char *payload)
{
    NewId id = { 0, "" };
    const char *p = payload;
    std::vector<WldArg>::const_iterator it = msg.args.begin();

    while (it != msg.args.end())
    {
        switch (it->type)
        {
        case WLD_ARG_ARRAY:
        {
            int len = byteArrToUInt32(p);
            p += 4;
            p += len;
            break;
        }
        case WLD_ARG_STRING:
        {
            int len = byteArrToUInt32(p);
            p += 4;
            p += len;
            break;
        }
        case WLD_ARG_NEWID:
        {
            int id = byteArrToUInt32(p);
            p += 4;
            int name = byteArrToUInt32(p);
            p += 4;
            int scheme = byteArrToUInt32(p);

            DEBUG_LOG("%d %d %d", id, name, scheme);
        }
        default:
            p += 4;
            break;
        }

        it++;
    }

    return id;
}
