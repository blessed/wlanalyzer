#include <unistd.h>
#include <string.h>
#include <errno.h>

static void debug_print(const char *buf)
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
