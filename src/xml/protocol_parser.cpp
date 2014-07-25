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

bool WldProtocolScanner::openProtocolFile(const std::string &path)
{
    xml_parse_result result = doc.load_file(path.c_str());

    DEBUG_LOG("result %s", result.description());

    return result;
}

WldProtocolDefinition *WldProtocolScanner::getProtocolDefinition()
{
    WldProtocolDefinition *prot = new WldProtocolDefinition;

    xml_node protocol = doc.child("protocol");
    xml_attribute attr = protocol.first_attribute();

    for (xml_node interfaceNode = protocol.child("interface"); interfaceNode;
         interfaceNode = interfaceNode.next_sibling())
    {
        WldInterface interface;

        interface.name = interfaceNode.attribute("name").value();
        interface.version = interfaceNode.attribute("version").as_int();

        if (!scanInterface(interfaceNode, interface))
        {
            delete prot;
            return NULL;
        }

        prot->addInterface(interface);
    }

    DEBUG_LOG("Got protocol node %s, attr %s=%s", protocol.name(), attr.name(), attr.value());

    return prot;
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
        if (type == "uint")
            arg.type = WLD_ARG_UINT;
        else if (type == "int")
            arg.type = WLD_ARG_INT;
        else if (type == "fixed")
            arg.type = WLD_ARG_FIXED;
        else if (type == "string")
            arg.type = WLD_ARG_STRING;
        else if (type == "object")
            arg.type = WLD_ARG_OBJECT;
        else if (type == "new_id")
            arg.type = WLD_ARG_NEWID;
        else if (type == "array")
            arg.type = WLD_ARG_ARRAY;
        else if (type == "fd")
            arg.type = WLD_ARG_FD;
        else
            arg.type = WLD_ARG_UNKNOWN;

        msg.args.push_back(arg);
    }

    return true;
}
