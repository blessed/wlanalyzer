#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

#ifndef DEBUG_BUILD
void wld_log(const char *format, ...)
{
    return;
}
#else

void wld_log(const char *format, ...)
{
    va_list vargs;
    va_start(vargs, format);

    vprintf(format, vargs);

    va_end(vargs);
}
#endif // DEBUG_BUILD

void debug_print(const char *buf)
{
    int errno_backup = errno;
    write(2, buf, strlen(buf));
    write(2, "\n", 1);
    errno = errno_backup;
}

int check_error(int error)
{
    const char *buf;

    if (error < 0)
    {
        buf = strerror(errno);
        debug_print(buf);

        return -1;
    }

    return 0;
}
