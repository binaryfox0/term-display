#include "term_priv.h"
#include "term_display.h"

#include <stdio.h>

#if defined(TERMINAL_UNIX)
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include <signal.h>

u8 set_handler(int type, void (*handler)(int))
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
u8 setup_env(void *stop_handler)
{
    if (tcgetattr(STDIN_FILENO, &old) == -1)
        return 1;

    cur = old;
    cur.c_lflag &= ~(ICANON | ECHO);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &cur) == -1)
        return 1;

    // Seperate if for stopping it further continuing
    void (*handler)(int) =(void(*)(int)) stop_handler;
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

u8 restore_env()
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

u8 timeout(int ms)
{
    return poll(&pfd, 1, ms);
}

static inline int available()
{
    int out = 0;
    ioctl(STDIN_FILENO, FIONREAD, &out);
    return out;
}
#elif defined(TERMINAL_WINDOWS)
#include <windows.h>
HANDLE h_in = 0, h_out = 0;
DWORD old_in_mode = 0, old_out_mode = 0;
u8 setup_env(void *stop_handler)
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
    return !SetConsoleCtrlHandler((BOOL(*)(DWORD)) stop_handler, 1);
}

term_ivec2 query_terminal_size()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return ivec2_init(csbi.dwSize.X, csbi.dwSize.Y);
    return ivec2_init(0, 0);
}

u8 restore_env()
{
    return SetConsoleMode(h_in, old_in_mode)
        || SetConsoleMode(h_out, old_out_mode);
}

u8 timeout(int ms)
{
    return WaitForSingleObject(h_in, ms) == WAIT_OBJECT_0;
}

static inline int available()
{
    DWORD out = 0;
    PeekNamedPipe(h_in, 0, 0, 0, &out, 0);
    return (int) out;
}
#else
#error "Unsupported OS"
#endif

#define _getch(ch) if (((ch) = getchar()) == EOF) return
#define getch_chk(val) if (getchar() != val) return

// Handle single-byte character input
static inline u8 handle_single_byte(const i8 byte, int *ch, int *mods)
{
    switch (byte) {
    case '\0':
        *ch = term_key_space;
        *mods |= key_ctrl;
        break;
    case 0x08:
        *ch = term_key_backspace;
        *mods |= key_ctrl;
        break;
    case 0x09:
        *ch = term_key_tab;
        break;
    case 0x0A:
    case 0x0D:
        *ch = term_key_enter;
        break;
    case 0x1B:
        *ch = term_key_escape;
        break;
    case '\"':
        *ch = term_key_space;
        *mods |= key_shift;
        break;
    case ':':
        *ch = term_key_semicolon;
        *mods |= key_shift;
        break;
    case '>':
    case '?':
    case '<':
        *ch -= 16;
        *mods |= key_shift;
        break;
    case '+':
        *ch = term_key_equal;
        *mods |= key_shift;
        break;
    case '@':
        *ch = term_key_2;
        *mods |= key_shift;
        break;
    case '^':
        *ch = term_key_6;
        *mods |= key_shift;
        break;
    case '_':
        *ch = term_key_minus;
        *mods |= key_shift;
        break;
    case 0x7F:
        *ch = term_key_backspace;
        break;
    default:
        if (IN_RANGE(byte, 0x01, 0x1D)) {
            *ch = byte + 64;
            *mods |= key_ctrl;
            break;
        }
        if (IN_RANGE(byte, 'A', 'Z')) {
            *ch = byte;
            *mods |= key_shift;
            break;
        }
        if (IN_RANGE(byte, 'a', 'z')) {
            *ch = byte - 32;
            break;
        }
        if (IN_RANGE(byte, ' ', '~')) {
            *ch = byte;
            break;
        }                       // Remaining characters
        return 1;
    }
    return 0;
}

// Handle navigation keys (Arrow keys, Home, End)
static inline u8 handle_nav_key(const i8 byte, int *ch)
{
    switch (byte) {
    case 'A':
        *ch = term_key_up;
        break;
    case 'B':
        *ch = term_key_down;
        break;
    case 'C':
        *ch = term_key_right;
        break;
    case 'D':
        *ch = term_key_left;
        break;
    case 'H':
        *ch = term_key_home;
        break;
    case 'F':
        *ch = term_key_end;
        break;
    default:
        return 1;
    }
    return 0;
}

static inline u8 handle_f5_below(const i8 byte, int *ch)
{
    int tmp = 0;
    if (OUT_RANGE
        ((tmp = byte - 'P' + term_key_f1), term_key_f1, term_key_f4))
        return 1;
    *ch = tmp;
    return 0;
}

static inline u8 handle_f5_above(const i8 first, const i8 second, int *ch)
{
    if (first == '1') {
        switch (second) {
        case '5':
            *ch = term_key_f5;
            break;
        case '7':
            *ch = term_key_f6;
            break;
        case '8':
            *ch = term_key_f7;
            break;
        case '9':
            *ch = term_key_f8;
            break;
        default:
            return 1;
        }
    } else if (first == '2') {
        switch (second) {
        case '0':
            *ch = term_key_f9;
            break;
        case '1':
            *ch = term_key_f10;
            break;
        case '3':
            *ch = term_key_f11;
            break;
        case '4':
            *ch = term_key_f12;
            break;
        default:
            return 1;
        }
    } else
        return 1;
    return 0;
}

static inline u8 handle_special_combo(const int byte, int *mods)
{
    switch (byte) {
    case '8':
        *mods |= (key_ctrl | key_alt | key_shift);
        break;
    case '7':
        *mods |= (key_ctrl | key_alt);
        break;
    case '6':
        *mods |= (key_ctrl | key_shift);
        break;
    case '5':
        *mods |= key_ctrl;
        break;
    case '4':
        *mods |= (key_alt | key_shift);
        break;
    case '3':
        *mods |= key_alt;
        break;
    case '2':
        *mods |= key_shift;
        break;
    default:
        return 1;
    }
    return 0;
}

static inline u8 handle_special_key(int *ch)
{
    switch (*ch) {
    case '2':
        *ch = term_key_insert;
        break;
    case '3':
        *ch = term_key_delete;
        break;
    case '5':
        *ch = term_key_page_up;
        break;
    case '6':
        *ch = term_key_page_down;
        break;
    default:
        return 1;
    }
    return 0;
}

#define BUFFER_SIZE 12
void kbpoll_events(key_callback_func func)
{
    if (!timeout(0))
        return;
    int ch = 0, mods = 0, bytes = 0;
    if ((bytes = available()) < 1)
        return;

    char buf[BUFFER_SIZE] = { 0 };
    if (read(STDIN_FILENO, buf, bytes < BUFFER_SIZE ? bytes : BUFFER_SIZE)
        == -1)
        return;

    if (buf[0] == 0x1B) {       // Escape sequence handling
        switch (bytes) {
        case 1:
            ch = term_key_escape;
            break;
        case 2:                // Alt modifier
            mods |= key_alt;
            handle_single_byte(buf[1], &ch, &mods);
            break;
        case 3:                // Navigation keys & F1-F4
            if (buf[1] == '[') {
                if (handle_nav_key(buf[2], &ch))
                    return;
            } else if (buf[1] == 'O') {
                if (handle_f5_below(buf[2], &ch))
                    return;
            } else
                return;
            break;
        case 4:                // Page Up / Page Down / Insert / Delete
            if (buf[1] == '[' && buf[3] == '~') {
                if (handle_nav_key(buf[2], &ch))
                    return;
            } else if (buf[1] == 'O') { // Ctrl/Shift/Alt + (F1 - F4) (Konsole case)
                if (handle_special_combo(buf[2], &mods)
                    || handle_f5_below(buf[3], &ch))
                    return;
            } else
                return;
            break;
        case 5:                // Function keys F5 - F12
            if (buf[1] != '[' || buf[4] != '~')
                return;
            if (handle_f5_above(buf[2], buf[3], &ch))
                return;
            break;
        case 6:                // Ctrl/Shift/Alt + (F1 - F4) (Vscode terminal case) and Navigation keys
            if (buf[1] != '[' || buf[2] != '1' || buf[3] != ';')
                return;
            if (handle_special_combo(buf[4], &mods))
                return;
            if (!handle_f5_below(buf[5], &ch) ||
                !handle_nav_key(buf[5], &ch)
                )
                return;
            break;
        case 7:
            if (buf[1] != '[' || buf[4] != ';' || buf[6] != '~')
                return;
            if (handle_single_byte(buf[0], &ch, &mods))
                return;
            if (handle_special_combo(buf[5], &mods)
                || handle_f5_above(buf[2], buf[3], &ch))
                return;
            break;
        default:
            return;
        }
    } else                      // Single-byte characters
    if (handle_single_byte(buf[0], &ch, &mods))
        return;
    func(ch, mods, key_press);
}


// Convert b to have the same type as a
void convert(u8 *b_out, const u8 *b_in, u8 ch_a, u8 *ch_b)
{
    u8 a_g = IS_GRAYSCALE(ch_a), b_g = IS_GRAYSCALE(*ch_b);
    if (a_g && !b_g) {
        b_out[0] = to_grayscale(b_in);
        b_out[1] = *ch_b - 3 ? b_out[3] : 255;
        *ch_b = ch_a;
        return;
    } else if (!a_g && b_g) {
        b_out[3] = *ch_b - 1 ? b_in[1] : 255;
        b_out[0] = b_out[1] = b_out[2] = b_in[0];
        *ch_b = ch_a;
        return;
    }
    for (u8 i = 0; i < *ch_b; i++)
        b_out[i] = b_in[i];;
}


void alpha_blend(u8 *a, const u8 *b, u8 ch_a, u8 ch_b)
{
    u8 out_a = IS_TRANSPARENT(ch_a);
    u8 a_i = ch_a - 1;
    u8 a_a = out_a ? a[a_i] : 255;
    u16 a_b = IS_TRANSPARENT(ch_b) ? b[ch_b - 1] : 255, iva_b = 255 - a_b;
    if (ch_a < 5)
        a[0] = (a_b * b[0] + iva_b * a[0]) >> 8;
    if (ch_a > 2) {
        a[1] = (a_b * b[1] + iva_b * a[1]) >> 8;
        a[2] = (a_b * b[2] + iva_b * a[2]) >> 8;
    }
    if (out_a)
        a[a_i] = !iva_b ? 255 : a_b + ((iva_b + a_a) >> 8);
}
