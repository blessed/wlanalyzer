#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include "logger.h"

#define WLA_SOCKETNAME "wayland-debug"

void debug_print(const char *buf);
int check_error(int error);

void wld_log(const char *format, ...);

#ifdef DEBUG_BUILD

#define WLD_DEBUG_LOG(format, ...) \
    printf("%s:%d->" format "\n", __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)

#else

#define WLD_DEBUG_LOG(format, ...)
#define WLD_DEBUG_LOG_DO(format, ...)

#endif // DEBUG_BUILD

#endif // COMMON_H
