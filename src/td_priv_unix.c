/*
MIT License

Copyright (c) 2025 binaryfox0 (Duy Pham Duc)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "td_priv.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include <signal.h>

static struct termios old, cur;

td_bool set_handler(int type, void (*handler)(int))
{
#ifdef _POSIX_VERSION
    struct sigaction sa = {.sa_flags = SA_SIGINFO,.sa_handler = handler };
    sigemptyset(&sa.sa_mask);
    return (sigaction(type, &sa, 0) != 0);
#else
    return signal(type, handler) == SIG_ERR;
#endif
}

td_bool setup_env(void(*handler)(int))
{
    if (tcgetattr(STDIN_FILENO, &old) == -1)
        return 1;

    cur = old;
    cur.c_lflag &= (td_u32)(~(ICANON | ECHO));

    if (tcsetattr(STDIN_FILENO, TCSANOW, &cur) == -1)
        return 1;

    // Seperate if for stopping it further continuing
    if (set_handler(SIGINT, handler))
        return 1;
    if (set_handler(SIGTERM, handler))
        return 1;
    if (set_handler(SIGQUIT, handler))
        return 1;

    return 0;
}

td_ivec2 query_terminal_size(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1)
        return td_ivec2_init(ws.ws_col, ws.ws_row);
    return td_ivec2_init(0, 0);
}

td_bool restore_env(void)
{
    if (tcsetattr(STDIN_FILENO, TCSANOW, &old) == -1)
        return 1;

    if (set_handler(SIGINT, SIG_DFL))
        return 1;
    if (set_handler(SIGTERM, SIG_DFL))
        return 1;
    if (set_handler(SIGQUIT, SIG_DFL))
        return 1;
    return 0;
}

struct pollfd pfd = {.events = POLLIN,.fd = STDIN_FILENO };

td_bool timeout(int ms)
{
    return poll(&pfd, 1, ms);
}

int available()
{
    int out = 0;
    ioctl(STDIN_FILENO, FIONREAD, &out);
    return out;
}

td_bool disable_stop_sig() {
    cur.c_lflag &= (td_u32)(~ISIG);
    return tcsetattr(STDIN_FILENO, TCSANOW, &cur) == -1;
}

td_bool enable_stop_sig() {
    cur.c_lflag |= ISIG;
    return tcsetattr(STDIN_FILENO, TCSANOW, &cur) == -1;
}