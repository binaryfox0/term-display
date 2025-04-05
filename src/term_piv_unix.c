#include "term_priv.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include <signal.h>

term_bool set_handler(int type, void (*handler)(int))
{
#ifdef _POSIX_VERSION
    struct sigaction sa = {.sa_flags = SA_SIGINFO,.sa_handler = handler };
    sigemptyset(&sa.sa_mask);
    return (sigaction(type, &sa, 0) != 0);
#else
    return signal(type, handler) == SIG_ERR;
#endif
}

static struct termios old, cur;
term_bool setup_env(void(*handler)(int))
{
    if (tcgetattr(STDIN_FILENO, &old) == -1)
        return 1;

    cur = old;
    cur.c_lflag &= (term_u32)(~(ICANON | ECHO));

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

term_ivec2 query_terminal_size()
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1)
        return ivec2_init(ws.ws_col, ws.ws_row);
    return ivec2_init(0, 0);
}

term_bool restore_env()
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

term_bool timeout(int ms)
{
    return poll(&pfd, 1, ms);
}

int available()
{
    int out = 0;
    ioctl(STDIN_FILENO, FIONREAD, &out);
    return out;
}