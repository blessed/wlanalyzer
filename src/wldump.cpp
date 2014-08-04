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


#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "common.h"
#include "proxy.h"
#include "logger.h"
#include "xml/protocol_parser.h"

using namespace std;

struct options_t
{
    std::string coreProtocol;
    std::vector<std::string> extensions;
    char **exec;
};

static int validate_cmdline(int argc, char **argv)
{
    if (argc < 2)
        return -1;

    return 0;
}

static void usage()
{
    fprintf(stderr, "Usage: wldump -c <core protocol file> -p <executable> [-e protocol extension files]\n");
}

static void verify_runtime()
{
    const char *runtimeDir = getenv("XDG_RUNTIME_DIR");
    if (!runtimeDir)
    {
        Logger::getInstance()->log("XDG_RUNTIME_DIR is not set in the environment. "
                                   "Set it first!\n");
        exit(EXIT_FAILURE);
    }

    string debugSocketPath = string(runtimeDir) + "/" + WLA_SOCKETNAME;
    if (!access(debugSocketPath.c_str(), F_OK))
    {
        Logger::getInstance()->log("debug socket %s already exists.\n"
                                   "Removing\n", debugSocketPath.c_str());

        unlink(debugSocketPath.c_str());
    }

    const char *socket_name = getenv("WAYLAND_DISPLAY");
    if (!socket_name)
    {
        Logger::getInstance()->log("WAYLAND_DISPLAY is not set. "
                                   "Probably no compositor is running.\n");
        exit(EXIT_FAILURE);
    }
}

static void modify_environment()
{
    setenv("WAYLAND_DISPLAY", WLA_SOCKETNAME, 1);
}

static void get_multiple_args(int argc, char **argv, options_t *opt)
{
    int index = optind - 1;
    while (index < argc)
    {
        char *argval = argv[index];
        if (argval[0] != '-')
        {
            std::string val = argv[index];
            opt->extensions.push_back(val);
        }
        else
        {
            optind = index - 1;
            break;
        }

        index++;
    }
}

static int parse_cmdline(int argc, char **argv, options_t *opt)
{
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-c"))
        {
            i++;
            if (i == argc)
            {
                Logger::getInstance()->log("Core protocol specification file not specified\n");
                exit(EXIT_FAILURE);
            }

            opt->coreProtocol = argv[i];
        }
        else if (!strcmp(argv[i], "-e"))
        {
            i++;
            if (i == argc)
            {
                Logger::getInstance()->log("No extensions specified\n");
                exit(EXIT_FAILURE);
            }

            while (argv[i][0] != '-')
            {
                opt->extensions.push_back(argv[i]);
                i++;

                if (i == argc - 1)
                {
                    break;
                }
            }

            i--;
        }
        else if (!strcmp(argv[i], "--"))
        {
            i++;
            if (i == argc)
            {
                Logger::getInstance()->log("Program not specified\n");
                exit(EXIT_FAILURE);
            }
            opt->exec = &argv[i];
        }
        else
        {
            Logger::getInstance()->log("Unknown option %s\n", argv[i]);
            usage();
            exit(EXIT_FAILURE);
        }
    }

    if (!opt->exec)
    {
        Logger::getInstance()->log("No program specified\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int ppid = 0;

    int ret = validate_cmdline(argc, argv);
    if (ret)
    {
        Logger::getInstance()->log("Invalid command line\n");
        usage();
        exit(EXIT_FAILURE);
    }

    options_t options;
    if (parse_cmdline(argc, argv, &options))
    {
        Logger::getInstance()->log("Invalid command line\n");
        usage();
        exit(EXIT_FAILURE);
    }

    verify_runtime();
    WlaProxyServer proxy;
    proxy.init(WLA_SOCKETNAME);

    WldProtocolAnalyzer *analyzer = new WldProtocolAnalyzer;
    analyzer->coreProtocol(options.coreProtocol);
    if (!options.extensions.empty())
    {
        std::vector<std::string>::const_iterator it = options.extensions.begin();
        for (; it != options.extensions.end(); it++)
        {
            DEBUG_LOG("extensions %s", it->c_str());
            analyzer->addProtocolSpec(*it);
        }
    }
    proxy.setAnalyzer(analyzer);

    if ((ppid = fork()) == 0)
    {
        modify_environment();
        ret = execvp(options.exec[0], options.exec);
        if (ret)
        {
            perror(NULL);
            Logger::getInstance()->log("failed to launch command %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }
    }

//    WldProtocolScanner scanner;
//    if (scanner.openProtocolFile("wayland.xml"))
//    {
//        WldProtocolDefinition *protocol = scanner.getProtocolDefinition();

//        delete protocol;
//    }

    proxy.startServer();

    kill(ppid, SIGTERM);

    wait(NULL);

    return 0;
}
