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

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x0002
#endif

HANDLE h_in = 0, h_out = 0;
DWORD old_in_mode = 0, old_out_mode = 0;
BOOL (*handle)(DWORD) = 0;

td_bool setup_env(BOOL (*handler)(DWORD))
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

td_ivec2 query_terminal_size()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return td_ivec2_init(csbi.dwSize.X, csbi.dwSize.Y);
    return td_ivec2_init(0, 0);
}

td_bool restore_env()
{
    return SetConsoleMode(h_in, old_in_mode)
        || SetConsoleMode(h_out, old_out_mode)
        || SetConsoleCtrlHandler(NULL, FALSE);
}

td_bool timeout(int ms)
{
    return WaitForSingleObject(h_in, ms) == WAIT_OBJECT_0;
}

int available()
{
    DWORD out = 0;
    PeekNamedPipe(h_in, 0, 0, 0, &out, 0);
    return (int) out;
}

td_bool disable_stop_sig() {
    return !SetConsoleCtrlHandler(NULL, TRUE);
}

td_bool enable_stop_sig() {
    return !SetConsoleCtrlHandler(handle, TRUE);
}