#include "td_priv.h"
#include "td_main.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define raise_hand(hand, ...) if((hand)) (hand)(__VA_ARGS__)

td_bool tdp_handle_shift(const td_i32 byte, int* ch, int* mods)
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
TD_INLINE td_bool tdp_handle_single_byte(const td_i32 byte, int *ch, int *mods)
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
            if(!tdp_handle_shift(byte, ch, mods))
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
TD_INLINE td_bool handle_nav_key(const td_i32 byte, int *ch)
{
    switch (byte) {
    case 'A': *ch = td_key_up; break;
    case 'B': *ch = td_key_down; break;
    case 'C': *ch = td_key_right; break;
    case 'D': *ch = td_key_left; break;
    case 'H': *ch = td_key_home; break;
    case 'F': *ch = td_key_end; break;
    case '2': *ch = td_key_insert; break;
    case '3': *ch = td_key_delete; break;
    case '5': *ch = td_key_page_up; break;
    case '6': *ch = td_key_page_down; break;
    default:
        return td_false;
    }
    return td_true;
}

TD_INLINE td_bool handle_f5_below(const td_i32 byte, int *ch)
{
    int tmp = 0;
    if (OUT_RANGE
        ((tmp = byte - 'P' + td_key_f1), td_key_f1, td_key_f4))
        return td_false;
    *ch = tmp;
    return td_true;
}

TD_INLINE td_bool handle_f5_above(const td_i32 first, const td_i32 second, int *ch)
{
    if (first == '1') {
        switch (second) {
        case '5': *ch = td_key_f5; break;
        case '7': *ch = td_key_f6; break;
        case '8': *ch = td_key_f7; break;
        case '9': *ch = td_key_f8; break;
        default: return td_false;
        }
    } else if (first == '2') {
        switch (second) {
        case '0': *ch = td_key_f9;  break;
        case '1': *ch = td_key_f10; break;
        case '3': *ch = td_key_f11; break;
        case '4': *ch = td_key_f12; break;
        default: return td_false;
        }
    } else
        return td_false;
    return td_true;
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
        return td_false;
    }
    return td_true;
}

#define BUF_SIZE 256  // physical buffer size
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

    if (rb->count > 0 && index < rb->start_idx) return -1; // too old

    // If index beyond current buffer, refill
    while (index > end_idx) {
        char tmp[BUF_SIZE];

        if(index - end_idx > tdp_kbbyte_available) return -1;
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

    int phys = (index - rb->start_idx) % BUF_SIZE;
    return rb->buffer[phys];
}

int tdp_stoi(tdp_ringbuf* rb, int *idx)
{
    int out = 0;
    int c = 0;
    while((c = tdp_rbuf_get(rb, (*idx)++)) != -1)
    {
        if(!isdigit(c)) {
            (*idx)--;
            break;
        }
        out = out * 10 + (c - '0');
    }
    return out;
}

void tdp_kbpoll(
        const td_key_callback keycb,
        const td_mouse_callback mousecb)
{
    if (!tdp_stdin_ready(0))
        return;
    if ((tdp_kbbyte_available = tdp_stdin_available()) < 1)
        return;

    tdp_ringbuf rb = {0};
    int idx = 0;
    for(;;)
    {
        int probe = idx;
        int ch = 0, mods = 0;

        int b0 = tdp_rbuf_get(&rb, probe);
        if (b0 == -1)
            break;

        if (b0 != 0x1b) {
            probe++;
            if (!tdp_handle_single_byte(b0, &ch, &mods))
                raise_hand(keycb, ch, mods, td_key_press);
            idx = probe;
            continue;
        }

        int b1 = tdp_rbuf_get(&rb, probe + 1);
        if (b1 == -1)
            break; /* wait */

        /* Alt + key */
        if (b1 != '[' && b1 != 'O') {
            int b = tdp_rbuf_get(&rb, probe + 1);
            if (b == -1)
                break;

            probe += 2;
            mods = td_key_alt;
            if (!tdp_handle_single_byte(b, &ch, &mods))
                raise_hand(keycb, ch, mods, td_key_press);
            idx = probe;
            continue;
        }

        int b2 = tdp_rbuf_get(&rb, probe + 2);
        if (b2 == -1)
            break;


        /* ESC [ x */
        if (handle_nav_key(b2, &ch)) {
            probe += 3;
            idx = probe;
            raise_hand(keycb, ch, 0, td_key_press);
            continue;
        }

        /* ESC O P..S  (F1–F4) */
        if (b1 == 'O' && handle_f5_below(b2, &ch)) {
            probe += 3;
            idx = probe;
            raise_hand(keycb, ch, 0, td_key_press);
            continue;
        }

        int b3 = tdp_rbuf_get(&rb, probe + 3);
        if (b3 == -1)
            break;

        /* ESC [ x ~ */
        if (b3 == '~' && handle_f5_below(b2, &ch)) {
            probe += 4;
            idx = probe;
            raise_hand(keycb, ch, 0, td_key_press);
            continue;
        }

        int b4 = tdp_rbuf_get(&rb, probe + 4);
        if (b4 == -1)
            break;

        /* ESC [ xx ~  (F5–F12) */
        if (b4 == '~' && handle_f5_above(b2, b3, &ch)) {
            probe += 5;
            idx = probe;
            raise_hand(keycb, ch, 0, td_key_press);
            continue;
        }

        /* ESC [ 1 ; y x   (mod + nav / F1–F4) */
        if (b2 == '1' && b3 == ';') {
            int b5 = tdp_rbuf_get(&rb, probe + 5);
            if (b5 == -1)
                break;

            if (handle_special_combo(b4, &mods) &&
                (handle_nav_key(b5, &ch) ||
                 handle_f5_below(b5, &ch))) {
                probe += 6;
                idx = probe;
                raise_hand(keycb, ch, mods, td_key_press);
                continue;
            }
        }


        /* ---------- mouse ---------- */
        if (b2 == '<') {
            probe += 3;
            int cb = tdp_stoi(&rb, &probe);
            if (tdp_rbuf_get(&rb, probe++) != ';')
                goto fallback;

            int cx = (tdp_stoi(&rb, &probe) - 1) /
                     tdp_options[td_opt_pixel_width];
            probe++; /* ; */

            int cy = (tdp_stoi(&rb, &probe) - 1) /
                     tdp_options[td_opt_pixel_height];

            int type = tdp_rbuf_get(&rb, probe++);
            if (type == -1)
                break;

            idx = probe;
            raise_hand(mousecb, cx, cy, cb);
            continue;
        }

fallback:
        idx++;
        raise_hand(keycb, td_key_escape, 0, td_key_press);
    }
}

// // Convert b to have the same type as a
void tdp_convert_color(td_u8 *b_out, const td_u8 *b_in, td_i32 ch_a, td_i32 ch_b, td_i32 *out_b)
{
    td_u8 a_g = IS_GRAYSCALE(ch_a);
    td_u8 b_g = IS_GRAYSCALE(ch_b);
    td_i32 tmp;
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
void tdp_blend(td_u8 *a, const td_u8 *b, const td_i32 ch_a, const td_i32 ch_b)
{
    td_u8 out_a = IS_TRANSPARENT(ch_a);
    td_i32 a_i = ch_a - 1;
    td_i32 a_a = out_a ? a[a_i] : 255;
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

void tdp_fill_buffer(void* dest, const void* src, td_u64 destsz, td_u64 srcsz)
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
