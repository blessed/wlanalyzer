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
        Logger::getInstance()->log("XDG_RUNTIME_DIR is not set in the environment."
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
        }
    }

    proxy.openServer();

    wait(NULL);

    return 0;
}
