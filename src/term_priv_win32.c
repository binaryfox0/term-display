#include "term_priv.h"

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x0002
#endif

HANDLE h_in = 0, h_out = 0;
DWORD old_in_mode = 0, old_out_mode = 0;
BOOL (*handle)(DWORD) = 0;

term_bool setup_env(BOOL (*handler)(DWORD))
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
    handle = handler;
    return enable_stop_sig();
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

term_bool disable_stop_sig() {
    return !SetConsoleCtrlHandler(NULL, TRUE);
}

term_bool enable_stop_sig() {
    return !SetConsoleCtrlHandler(handle, TRUE);
}