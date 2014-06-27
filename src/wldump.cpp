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

using namespace std;

int validate_cmdline(int argc, char **argv)
{
    if (argc < 2)
        return -1;

    return 0;
}

void usage()
{
    fprintf(stderr, "Usage: wlanalyzer <binary> [args]\n");
}

void verify_runtime()
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
                                   "Probably no compositor is running\n");
        exit(EXIT_FAILURE);
    }
}

void modify_environment()
{
    setenv("WAYLAND_DISPLAY", WLA_SOCKETNAME, 1);
}

int main(int argc, char *argv[])
{
    int ret = validate_cmdline(argc, argv);
    if (ret)
    {
        Logger::getInstance()->log("Invalid command line\n");
        usage();
        exit(EXIT_FAILURE);
    }

    verify_runtime();
    WlaProxyServer proxy;
    proxy.init(WLA_SOCKETNAME);

    if (fork() == 0)
    {
        modify_environment();
        ret = execvp(argv[1], argv + 1);
        if (ret)
        {
            perror(NULL);
            Logger::getInstance()->log("failed to launch command %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }
    }

    proxy.openServer();

    wait(NULL);

    return 0;
}
