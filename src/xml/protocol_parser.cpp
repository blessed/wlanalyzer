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


#include "../common.h"
#include "protocol_parser.h"

using namespace pugi;

WldProtocolScanner::types_t WldProtocolScanner::types;
WldProtocolDefinition::type_size_t WldProtocolDefinition::type_size;
bool WldProtocolDefinition::initialized = false;
bool WldProtocolScanner::initialized = false;

WldProtocolScanner::WldProtocolScanner()
{
     init();
}

bool WldProtocolScanner::openProtocolFile(const std::string &path)
{
    xml_parse_result result = doc.load_file(path.c_str());

    Logger::getInstance()->log("result %s: %s\n", result.description(), path.c_str());

    return result;
}

WldProtocolDefinition *WldProtocolScanner::getProtocolDefinition(WldProtocolDefinition *protocolDef)
{
    WldProtocolDefinition *prot;
//    WldProtocolDefinition *prot = new WldProtocolDefinition;
    if (protocolDef)
        prot = protocolDef;
    else
        prot = new WldProtocolDefinition;

    xml_node protocol = doc.child("protocol");
    xml_attribute attr = protocol.first_attribute();

    Logger::getInstance()->log("%s %s = %s\n", protocol.name(), attr.name(), attr.value());

    for (xml_node interfaceNode = protocol.child("interface"); interfaceNode;
         interfaceNode = interfaceNode.next_sibling())
    {
        WldInterface interface;

        interface.name = interfaceNode.attribute("name").value();
        interface.version = interfaceNode.attribute("version").as_int();

        Logger::getInstance()->log("interface: %s version: %d\n", interface.name.c_str(), interface.version);

        if (!scanInterface(interfaceNode, interface))
        {
            delete prot;
            return NULL;
        }

        prot->addInterface(interface);
    }

    return prot;
}

void WldProtocolScanner::init()
{
    if (initialized)
        return;

    DEBUG_LOG("sizeof WldArgTypeHasher %d", sizeof(WldArgTypeHasher));

    types["uint"] = WLD_ARG_UINT;
    types["int"] = WLD_ARG_INT;
    types["string"] = WLD_ARG_STRING;
    types["fixed"] = WLD_ARG_FIXED;
    types["object"] = WLD_ARG_OBJECT;
    types["new_id"] = WLD_ARG_NEWID;
    types["fd"] = WLD_ARG_FD;
    types["array"] = WLD_ARG_ARRAY;

    initialized = true;
}

bool WldProtocolScanner::scanInterface(const xml_node &node, WldInterface &interface)
{
    if (!getMessages(node, interface, WLD_MSG_REQUEST))
        return false;

    if (!getMessages(node, interface, WLD_MSG_EVENT))
        return false;

    return true;
}

bool WldProtocolScanner::getMessages(const xml_node &node, WldInterface &interface, WLD_MESSAGE_TYPE type)
{
    std::string typeStr;
    if (type == WLD_MSG_REQUEST)
        typeStr = "request";
    else
        typeStr = "event";

    for (xml_node eventNode = node.child(typeStr.c_str()); eventNode;
         eventNode = eventNode.next_sibling())
    {
        WldMessage msg;
        msg.type = type;
        msg.signature = eventNode.attribute("name").value();
        msg.intf_name = interface.name;

//        Logger::getInstance()->log("\t%s: %s\n", type == WLD_MSG_REQUEST ? "request" : "event", msg.signature.c_str());

        if (!scanArgs(eventNode, msg))
            return false;

        if (type == WLD_MSG_REQUEST)
            interface.requests.push_back(msg);
        else
            interface.events.push_back(msg);
    }

    return true;
}

bool WldProtocolScanner::scanArgs(const xml_node &node, WldMessage &msg)
{
    for (xml_node argNode = node.child("arg"); argNode; argNode = argNode.next_sibling())
    {
        WldArg arg;
        arg.name = argNode.attribute("name").value();
        std::string type = argNode.attribute("type").value();

        types_t::const_iterator it = types.find(type);
        if (it != types.end())
            arg.type = it->second;
        else
            arg.type = WLD_ARG_UNKNOWN;

        if (arg.type == WLD_ARG_NEWID)
        {
            std::string intf = argNode.attribute("interface").value();
            if (intf.empty())
                DEBUG_LOG("new_id has no interface");

            arg.interface = intf;
        }

//        Logger::getInstance()->log("\t\targ: %28s\ttype: %s\n", arg.name.c_str(), type.c_str());

        msg.args.push_back(arg);
    }

    return true;
}

WldProtocolDefinition::WldProtocolDefinition()
{
    init();
}

const WldInterface *WldProtocolDefinition::getInterface(const std::string &name) const
{
    std::vector<WldInterface>::const_iterator it;

    for (it = interfaceList.begin(); it != interfaceList.end(); it++)
    {
        if ((*it).name == name)
            return &(*it);
    }

    return NULL;
}

size_t WldProtocolDefinition::getArgSize(WldArgType type)
{
    return type_size.find(type)->second;
}

void WldProtocolDefinition::init()
{
    if (initialized)
        return;

    DEBUG_LOG("sizeof WldArgTypeHasher %d", sizeof(WldArgTypeHasher));

    type_size[WLD_ARG_UINT] = sizeof(WldArgVal::u);
    type_size[WLD_ARG_INT] = sizeof(WldArgVal::i);
    type_size[WLD_ARG_FIXED] = sizeof(WldArgVal::f);
    type_size[WLD_ARG_STRING] = sizeof(WldArgVal::s);
    type_size[WLD_ARG_FD] = sizeof(WldArgVal::h);
    type_size[WLD_ARG_ARRAY] = sizeof(WldArgVal::a);
    type_size[WLD_ARG_OBJECT] = sizeof(WldArgVal::o);
    type_size[WLD_ARG_NEWID] = sizeof(WldArgVal::n);

    initialized = true;
}
