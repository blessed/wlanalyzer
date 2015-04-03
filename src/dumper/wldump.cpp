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


#include <fcntl.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../wlanalyzer_base/common.h"
#include "../wlanalyzer_base/proxy.h"
#include "../wlanalyzer_base/logger.h"
#include "../wlanalyzer_base/xml/protocol_parser.h"

#include "../wlanalyzer_base/dummy_sink.h"
#include "../wlanalyzer_base/raw_composite_sink.h"
#include "../wlanalyzer_base/raw_fd_sink.h"

using namespace std;
using namespace WlAnalyzer;


struct options_t
{
    options_t() : coreProtocol(""), analyze(false), exec(NULL) {}

    std::string coreProtocol;
    std::vector<std::string> extensions;
    bool analyze;
    std::string port_number; // used when the dumper is launched in server mode
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
    fprintf(stderr, "wldump is a wayland protocol dumper\n"
            "Usage:\twldump [OPTIONS] -- <wayland_client>\n\n"
            "Options:\n"
            "\t-c <file_path> - set the core protocol specification file\n"
            "\t-e <file_paths> - provide extensions of the protocol file. "
            "Use only with -c option\n"
            "\t-n <port number> - launch in server mode\n"
            "\t-h - this help screen\n");
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
            opt->analyze = true;
        }
        else if (!strcmp(argv[i], "-n"))
        {
            i++;
            if (i == argc)
            {
                Logger::getInstance()->log("port number not specified\n");
                exit(EXIT_FAILURE);
            }

            opt->port_number = argv[i];
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

                if (i >= argc - 1)
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
                usage();
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
        usage();
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
    // set sink
    shared_ptr<RawCompositeSink> composite(new RawCompositeSink());
    proxy.setSink(composite);
    shared_ptr<RawMessageSink> dummysink(new DummySink());
    composite->AddSink(dummysink);
    shared_ptr<RawMessageSink> file1sink(new RawFdSink(open("file1.wldump", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)));
    composite->AddSink(file1sink);
    shared_ptr<RawMessageSink> file2sink(new RawFdSink(open("file2.wldump", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)));
    composite->AddSink(file2sink);

    if (options.coreProtocol.size())
    {
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

        WldIODumper *dumper = new WldIODumper;
        dumper->open("dump");
        proxy.setDumper(dumper);

//        WldNetDumper *netDump = new WldNetDumper;
//        if (netDump->open("5000"))
//            DEBUG_LOG("Failed to open port 5000");
//        proxy.setDumper(netDump);

        WlaBinParser *parser = new WlaBinParser;
        parser->openResource("dump");
        parser->attachAnalyzer(analyzer);
        proxy.setParser(parser);
    }
    else if (options.port_number.size())
    {
        WldNetDumper *netDump = new WldNetDumper;
        if (netDump->open(options.port_number))
            DEBUG_LOG("Failed to open port %s", options.port_number.c_str());
        proxy.setDumper(netDump);
    }

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

    proxy.startServer();

    kill(ppid, SIGTERM);

    wait(NULL);

    return 0;
}
