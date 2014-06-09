/*
 * Copyright © 2014 Samsung Electronics
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <string>
#include "interceptor.h"


WldInterceptor::WldInterceptor()
{
}

WldInterceptor::~WldInterceptor()
{
}

WldInterceptor::InterceptorErr WldInterceptor::configure()
{
    const char *server_addr;
    const char *runtime_dir;

    runtime_dir = getenv("XDG_RUNTIME_DIR");
    if (!runtime_dir)
    {
        return ConfigureErr;
    }

    server_addr = getenv("WAYLAND_DISPLAY");
    if (!server_addr)
        server_addr = "wayland-0";

    std::string socket_path;
    socket_path = std::string(runtime_dir) + std::string(server_addr);

    return NoError;
}

int WldInterceptor::start()
{
}
