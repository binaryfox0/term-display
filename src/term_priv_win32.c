#include "term_priv.h"

#include <windows.h>

HANDLE h_in = 0, h_out = 0;
DWORD old_in_mode = 0, old_out_mode = 0;
term_bool setup_env(int (*handler)(unsigned int))
{
    if ((h_in = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE)
        return 1;
    if (!GetConsoleMode(h_in, &old_in_mode))
        return 1;
    if (!SetConsoleMode
        (h_in,
         old_in_mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT |
                         ENABLE_VIRTUAL_TERMINAL_INPUT)))
        return 1;
    if ((h_out = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE)
        return 1;
    if (!GetConsoleMode(h_out, &old_out_mode))
        return 1;
    if (!SetConsoleMode
        (h_out, old_out_mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING))
        return 1;
    return !SetConsoleCtrlHandler(handler, 1);
}

term_ivec2 query_terminal_size()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return ivec2_init(csbi.dwSize.X, csbi.dwSize.Y);
    return ivec2_init(0, 0);
}

term_bool restore_env()
{
    return SetConsoleMode(h_in, old_in_mode)
        || SetConsoleMode(h_out, old_out_mode)
        || SetConsoleCtrlHandler(NULL, FALSE);
}

term_bool timeout(int ms)
{
    return WaitForSingleObject(h_in, ms) == WAIT_OBJECT_0;
}

int available()
{
    DWORD out = 0;
    PeekNamedPipe(h_in, 0, 0, 0, &out, 0);
    return (int) out;
}