#include "term_priv.h"
#include "td_main.h"

#include <stdio.h>
#include <string.h>

#define _getch(ch) if (((ch) = getchar()) == EOF) return
#define getch_chk(val) if (getchar() != val) return

term_bool key_shift_translate(const term_i8 byte, int* ch, int* mods)
{
    switch(byte)
    {
    case '~': *ch = '`'; break;
    case '!': *ch = '1'; break;
    case '@': *ch = '2'; break;
    case '#': *ch = '3'; break;
    case '$': *ch = '4'; break;
    case '%': *ch = '5'; break;
    case '^': *ch = '6'; break;
    case '&': *ch = '7'; break;
    case '*': *ch = '8'; break;
    case '(': *ch = '9'; break;
    case ')': *ch = '0'; break;
    case '_': *ch = '-'; break;
    case '+': *ch = '='; break;
    case '{': *ch = '['; break;
    case '}': *ch = ']'; break;
    case '|': *ch = '\\'; break;
    case ':': *ch = ';'; break;
    case '\"': *ch = '\''; break;
    case '<': *ch = ','; break;
    case '>': *ch = '.'; break;
    case '?': *ch = '/'; break;
    default:
    {
        if (IN_RANGE(byte, 'A', 'Z')) {
            *ch = byte;
            break;
        }
        return 1;
    }
    }
    *mods |= td_key_shift;
    return 0;
}

// Handle single-byte character input
term_bool shift_translate = term_false;
TD_INLINE term_bool handle_single_byte(const term_i8 byte, int *ch, int *mods)
{
    switch (byte) {
    case '\0':
        *ch = td_key_space;
        *mods |= td_key_ctrl;
        break;
    case 0x08:
        *ch = td_key_backspace;
        *mods |= td_key_ctrl;
        break;
    case 0x09:
        *ch = td_key_tab;
        break;
    case 0x0A:
    case 0x0D:
        *ch = td_key_enter;
        break;
    case 0x1B:
        *ch = td_key_escape;
        break;
    case 0x7F:
        *ch = td_key_backspace;
        break;
    default:
        if (IN_RANGE(byte, 0x01, 0x1D)) {
            *ch = byte + 64;
            *mods |= td_key_ctrl;
            break;
        }
        if(shift_translate) {
            if(!key_shift_translate(byte, ch, mods))
                break;
            if (IN_RANGE(byte, 'a', 'z')) {
                *ch = byte - 32;
                break;
            }
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
TD_INLINE term_bool handle_nav_key(const term_i8 byte, int *ch)
{
    switch (byte) {
    case 'A': *ch = td_key_up; break;
    case 'B': *ch = td_key_down; break;
    case 'C': *ch = td_key_right; break;
    case 'D': *ch = td_key_left; break;
    case 'H': *ch = td_key_home; break;
    case 'F': *ch = td_key_end; break;
    default:
        return 1;
    }
    return 0;
}

TD_INLINE term_bool handle_f5_below(const term_i8 byte, int *ch)
{
    int tmp = 0;
    if (OUT_RANGE
        ((tmp = byte - 'P' + td_key_f1), td_key_f1, td_key_f4))
        return 1;
    *ch = tmp;
    return 0;
}

TD_INLINE term_bool handle_f5_above(const term_i8 first, const term_i8 second, int *ch)
{
    if (first == '1') {
        switch (second) {
        case '5': *ch = td_key_f5; break;
        case '7': *ch = td_key_f6; break;
        case '8': *ch = td_key_f7; break;
        case '9': *ch = td_key_f8; break;
        default: return 1;
        }
    } else if (first == '2') {
        switch (second) {
        case '0': *ch = td_key_f9;  break;
        case '1': *ch = td_key_f10; break;
        case '3': *ch = td_key_f11; break;
        case '4': *ch = td_key_f12; break;
        default: return 1;
        }
    } else
        return 1;
    return 0;
}

TD_INLINE term_bool handle_special_combo(const int byte, int *mods)
{
    switch (byte) {
    case '8': *mods |= (td_key_ctrl | td_key_alt | td_key_shift); break;
    case '7': *mods |= (td_key_ctrl | td_key_alt); break;
    case '6': *mods |= (td_key_ctrl | td_key_shift); break;
    case '5': *mods |= td_key_ctrl; break;
    case '4': *mods |= (td_key_alt | td_key_shift); break;
    case '3': *mods |= td_key_alt; break;
    case '2': *mods |= td_key_shift; break;
    default:
        return 1;
    }
    return 0;
}

TD_INLINE term_bool handle_special_key(int *ch)
{
    switch (*ch) {
    case '2': *ch = td_key_insert; break;
    case '3': *ch = td_key_delete; break;
    case '5': *ch = td_key_page_up; break;
    case '6': *ch = td_key_page_down; break;
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
    if (read(STDIN_FILENO, buf, (size_t)(bytes < BUFFER_SIZE ? bytes : BUFFER_SIZE))
        == -1)
        return;

    if (buf[0] == 0x1B) {       // Escape sequence handling
        switch (bytes) {
        case 1:
            ch = td_key_escape;
            break;
        case 2:                // Alt modifier
            mods |= td_key_alt;
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
    if(func)func(ch, mods, key_press);
}


// // Convert b to have the same type as a
void convert(term_u8 *b_out, const term_u8 *b_in, term_u8 ch_a, term_u8 ch_b, term_u8 *out_b)
{
    term_u8 a_g = IS_GRAYSCALE(ch_a);
    term_u8 b_g = IS_GRAYSCALE(ch_b);
    term_u8 tmp;
    if(!out_b) { out_b = &tmp; }

    if (a_g && !b_g) { // A is grayscale, B is truecolor
        b_out[0] = to_grayscale(b_in); 
        b_out[1] = ch_b - 3 ? b_in[3] : 255;
        *out_b = ch_b - 2;
        return;
    } 
    else if (!a_g && b_g) { // A is truecolor, B is grayscale
        b_out[0] = b_out[1] = b_out[2] = b_in[0]; 
        b_out[3] = ch_b - 1 ? b_in[1] : 255;
        *out_b = ch_b + 2;
        return;
    }

    // If both are already the same format, copy directly
    for (term_u8 i = 0; i < ch_b; i++) {
        b_out[i] = b_in[i];
    }
    *out_b = ch_b;
}


// Color b (foreground) er color a (background)
void alpha_blend(term_u8 *a, const term_u8 *b, const term_u8 ch_a, const term_u8 ch_b)
{
    term_u8 out_a = IS_TRANSPARENT(ch_a);
    term_u8 a_i = ch_a - 1;
    term_u8 a_a = out_a ? a[a_i] : 255;
    term_u16 a_b = IS_TRANSPARENT(ch_b) ? b[ch_b - 1] : 255, iva_b = 255 - a_b;
    if (ch_a < 5)
        a[0] = (term_u8)((a_b * b[0] + iva_b * a[0]) >> 8);
    if (ch_a > 2) {
        a[1] = (term_u8)((a_b * b[1] + iva_b * a[1]) >> 8);
        a[2] = (term_u8)((a_b * b[2] + iva_b * a[2]) >> 8);
    }
    if (out_a)
    
    a[a_i] = (term_u8)(!iva_b ? 255 : a_b + ((iva_b + a_a) >> 8));
}

void fill_buffer(void* dest, const void* src, size_t destsz, size_t srcsz)
{
    if (destsz < srcsz) {
        memcpy(dest, src, destsz);
        return;
    }

    term_u8* ptr = (term_u8*)dest;
    memcpy(ptr, src, srcsz);

    size_t filled = srcsz;
    ptr += srcsz;

    while (filled * 2 <= destsz) {
        memcpy(ptr, dest, filled);
        ptr += filled;
        filled *= 2;
    }

    memcpy(ptr, dest, destsz - filled);
}


void reset_buffer(const void** out_buffer, const term_vec2 size, const term_vec2* out_size, const int type_size)
{
    
}