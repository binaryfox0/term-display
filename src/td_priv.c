#include "td_priv.h"
#include "td_main.h"

#include <stdio.h>
#include <string.h>

#define _getch(ch) if (((ch) = getchar()) == EOF) return
#define raise_hand(hand, ...) if((hand)) (hand)(__VA_ARGS__)

td_bool key_shift_translate(const td_i32 byte, int* ch, int* mods)
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
td_bool tdp_shift_translate = td_true;
TD_INLINE td_bool handle_single_byte(const td_i32 byte, int *ch, int *mods)
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
        if(tdp_shift_translate) {
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
TD_INLINE td_bool handle_nav_key(const td_i8 byte, int *ch)
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

TD_INLINE td_bool handle_f5_below(const td_i8 byte, int *ch)
{
    int tmp = 0;
    if (OUT_RANGE
        ((tmp = byte - 'P' + td_key_f1), td_key_f1, td_key_f4))
        return 1;
    *ch = tmp;
    return 0;
}

TD_INLINE td_bool handle_f5_above(const td_i8 first, const td_i8 second, int *ch)
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

TD_INLINE td_bool handle_special_combo(const int byte, int *mods)
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

TD_INLINE td_bool handle_special_key(int *ch)
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

#define BUF_SIZE 16  // physical buffer size
typedef struct {
    char buffer[BUF_SIZE];        // physical storage
    int start_idx;          // logical index of buffer[0]
    int count;              // number of valid elements in buffer
} tdp_ringbuf;

static int tdp_kbbyte_available = 0;
// Access by logical index
int tdp_rbuf_get(tdp_ringbuf* rb, int index) {
    // If buffer empty, fill first chunk
    if (rb->count == 0) {
        int n = (int)read(STDIN_FILENO, rb->buffer, min((size_t)tdp_kbbyte_available, BUF_SIZE));
        if (n == 0) return -1;  // no data
        
        rb->start_idx = index;
        rb->count = n;
        tdp_kbbyte_available -= n;

        return rb->buffer[0];
    }

    int end_idx = rb->start_idx + rb->count - 1;

    if (index < rb->start_idx) return -1; // too old

    // If index beyond current buffer, refill
    while (index > end_idx) {
        char tmp[BUF_SIZE];

        int n = (int)read(STDIN_FILENO, tmp, min((size_t)tdp_kbbyte_available, BUF_SIZE));
        if (n == 0) return -1;  // no more data

        // Copy into circular buffer
        for (int i = 0; i < n; i++) {
            int pos = (rb->start_idx + rb->count + i) % BUF_SIZE;
            rb->buffer[pos] = tmp[i];
        }

        // Update count and start_idx if buffer exceeded
        if (rb->count + n > BUF_SIZE) {
            rb->start_idx += (rb->count + n - BUF_SIZE);
            rb->count = BUF_SIZE;
        } else {
            rb->count += n;
        }

        end_idx = rb->start_idx + rb->count - 1;
        tdp_kbbyte_available -= n;
    }

    int phys = index % BUF_SIZE;
    return rb->buffer[phys];
}

#define BUFFER_SIZE 12
void tdp_kbpoll(key_callback_func func)
{
    if (!tdp_stdin_ready(0))
        return;
    if ((tdp_kbbyte_available = tdp_stdin_available()) < 1)
        return;

    tdp_ringbuf rb = {0};
    int idx = 0;
    for(;;)
    {
        int ch = 0, mods = 0;
        int b0 = tdp_rbuf_get(&rb, idx + 0);
        if(b0 == -1) break;
        if(b0 != 0x1B) {
            idx++;
            if(handle_single_byte(b0, &ch, &mods))
                continue;
            raise_hand(func, ch, mods, td_key_press);
            continue;
        }

        int b1 = tdp_rbuf_get(&rb, idx + 1);
        if(b1 == -1) break;
        if(b1 != '[' && b1 != '0')
        {
            idx += 2;
            mods |= td_key_alt;
            if(handle_single_byte(b1, &ch, &mods))
                continue;
            raise_hand(func, ch, mods, td_key_press);
            continue;
        }
    }

    /*
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
            if (buf[1] != '[') {
                if(buf[2] != '1' || buf[3] != ';')
                    return;
                if (handle_special_combo(buf[4], &mods))
                    return;
                if (!handle_f5_below(buf[5], &ch) ||
                    !handle_nav_key(buf[5], &ch)
                    )
                    return;
            }
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
    } else {                      // Single-byte characters
        if (handle_single_byte(buf[0], &ch, &mods))
            return;
    }
    if(func)
        func(ch, mods, td_key_press);
    */
}


// // Convert b to have the same type as a
void convert(td_u8 *b_out, const td_u8 *b_in, td_u8 ch_a, td_u8 ch_b, td_u8 *out_b)
{
    td_u8 a_g = IS_GRAYSCALE(ch_a);
    td_u8 b_g = IS_GRAYSCALE(ch_b);
    td_u8 tmp;
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
    for (td_u8 i = 0; i < ch_b; i++) {
        b_out[i] = b_in[i];
    }
    *out_b = ch_b;
}


// Color b (foreground) er color a (background)
void alpha_blend(td_u8 *a, const td_u8 *b, const td_u8 ch_a, const td_u8 ch_b)
{
    td_u8 out_a = IS_TRANSPARENT(ch_a);
    td_u8 a_i = ch_a - 1;
    td_u8 a_a = out_a ? a[a_i] : 255;
    td_u16 a_b = IS_TRANSPARENT(ch_b) ? b[ch_b - 1] : 255, iva_b = 255 - a_b;
    if (ch_a < 5)
        a[0] = (td_u8)((a_b * b[0] + iva_b * a[0]) >> 8);
    if (ch_a > 2) {
        a[1] = (td_u8)((a_b * b[1] + iva_b * a[1]) >> 8);
        a[2] = (td_u8)((a_b * b[2] + iva_b * a[2]) >> 8);
    }
    if (out_a)
    
    a[a_i] = (td_u8)(!iva_b ? 255 : a_b + ((iva_b + a_a) >> 8));
}

void fill_buffer(void* dest, const void* src, size_t destsz, size_t srcsz)
{
    if(!destsz || !srcsz || !dest || !src) return;
    if (destsz < srcsz) {
        memcpy(dest, src, destsz);
        return;
    }

    td_u8* ptr = (td_u8*)dest;
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
