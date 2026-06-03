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

#include "td_term.h"

#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "td_utils.h"

static struct termios old = {0}, cur = {0};
static struct pollfd pfd = {.events = POLLIN,.fd = STDIN_FILENO };

TD_INLINE void tdp_set_sighand(
        const int signal, const tdp_sighand_t handle)
{
#ifdef _POSIX_VERSION
    struct sigaction sa = {.sa_flags = SA_SIGINFO,.sa_handler = handle };
    sigemptyset(&sa.sa_mask);
    sigaction(signal, &sa, 0);
#else
    signal(type, handler);
#endif
}

td_error_t tdp_term_init(void)
{
    if (tcgetattr(STDIN_FILENO, &old) == -1)
        return TD_ERR_GENERIC;

    cur = old;
    cur.c_lflag &= (td_u32)(~(ICANON | ECHO));

    if (tcsetattr(STDIN_FILENO, TCSANOW, &cur) == -1)
        return TD_ERR_GENERIC;

    return TD_ERR_OK;
}

void tdp_term_set_stop_handle(const tdp_sighand_t handle)
{
    tdp_set_sighand(SIGINT, handle ? handle : SIG_DFL);
    tdp_set_sighand(SIGTSTP, handle ? handle : SIG_DFL);
    tdp_set_sighand(SIGQUIT, handle ? handle : SIG_DFL);
}

td_ivec2 tdp_term_get_size(void)
{
    struct winsize ws = {0};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1)
        return (td_ivec2){.x=ws.ws_col, .y=ws.ws_row};
    return (td_ivec2){0};
}

void tdp_term_clear(void)
{
    fflush(stdout);
    tdp_tty_write(
        "\x1b[0m"   // Reset all text formatting (colors, styles, attributes)
        "\x1b[3J"   // Clear entire scrollback buffer (if supported by terminal)
        "\x1b[H"    // Move cursor to top-left corner (home position)
        "\x1b[2J"   // Clear entire visible screen
    );
}

td_bool tdp_term_stdin_ready(const int ms) 
{
    return (td_bool)(poll(&pfd, 1, ms) == 1);
}

int tdp_term_stdin_available(void)
{
    int out = 0;
    ioctl(STDIN_FILENO, FIONREAD, &out);
    return out;
}

td_error_t tdp_term_toggle_stop(const td_bool enable)
{
    if(enable)
        cur.c_lflag |= ISIG;
    else
        cur.c_lflag &= (td_u32)(~ISIG);
    return tcsetattr(STDIN_FILENO, TCSANOW, &cur) != -1 ?
        TD_ERR_OK : TD_ERR_GENERIC;
}

void tdp_term_exit(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    tdp_term_set_stop_handle(0);
}

