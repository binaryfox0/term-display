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

#include "td_def.h"
#include "td_priv.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#define _POSIX_C_SOURCE 200809L
#include <signal.h>

static struct termios old, cur;

td_bool tdp_set_sighand(int type, void (*handler)(int))
{
#ifdef _POSIX_VERSION
    struct sigaction sa = {.sa_flags = SA_SIGINFO,.sa_handler = handler };
    sigemptyset(&sa.sa_mask);
    return (sigaction(type, &sa, 0) != 0);
#else
    return signal(type, handler) == SIG_ERR;
#endif
}

td_bool tdp_setup_env(const tdp_sighand handle)
{
    if (tcgetattr(STDIN_FILENO, &old) == -1)
        return td_false;

    cur = old;
    cur.c_lflag &= (td_u32)(~(ICANON | ECHO));

    if (tcsetattr(STDIN_FILENO, TCSANOW, &cur) == -1)
        return td_false;

    int sigs[] = { SIGINT,  SIGQUIT};
    for(unsigned long i = 0; i < sizeof(sigs) / sizeof(sigs[0]); i++) {
        if(tdp_set_sighand(sigs[i], handle)) {
            tdp_restore_env();
            return td_false;
        }
    }

    return td_true;
}

td_ivec2 tdp_get_termsz(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1)
        return (td_ivec2){.x=ws.ws_col, .y=ws.ws_row};
    return (td_ivec2){0};
}

void tdp_restore_env(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &old);

    tdp_set_sighand(SIGINT, SIG_DFL);
    tdp_set_sighand(SIGQUIT, SIG_DFL);
}

static struct pollfd pfd = {.events = POLLIN,.fd = STDIN_FILENO };
td_bool tdp_stdin_ready(const int ms) {
    return (td_bool)(poll(&pfd, 1, ms) == 1);
}

int tdp_stdin_available(void)
{
    int out = 0;
    ioctl(STDIN_FILENO, FIONREAD, &out);
    return out;
}

td_bool tdp_enable_stop_sig(const td_bool enable) {
    if(enable == td_true)
        cur.c_lflag |= ISIG;
    else
        cur.c_lflag &= (td_u32)(~ISIG);
    return tcsetattr(STDIN_FILENO, TCSANOW, &cur) != -1;
}
