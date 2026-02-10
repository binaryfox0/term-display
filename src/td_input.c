#include "td_input.h"

#include <ctype.h>

#include "td_def.h"
#include "td_main.h"
#include "td_priv.h"

#define tdp_raise(name, ...) \
    if(tdp_cb.name) (tdp_cb.name)(__VA_ARGS__)

#define tdp_define_setter(name) \
    void __td_cat(__td_cat(td_set_, name), _callback)(const __td_cat(__td_cat(td_, name), _callback) callback) { \
        tdp_cb.name = callback; \
    }

typedef struct tdp_keymap_entry
{
    td_u16 key;
    td_u8 mod;
} tdp_keymap_entry;

typedef struct {
    td_key_callback key;
    td_mouse_button_callback mouse_button;
    td_cursor_pos_callback cursor_pos;
} tdp_input_callback;

static const tdp_keymap_entry tdp_keymap[0xFF] = {
    [0x00] = { td_key_space,         td_mod_ctrl },
    [0x01] = { td_key_a,             td_mod_ctrl },
    [0x02] = { td_key_b,             td_mod_ctrl },
    [0x03] = { td_key_c,             td_mod_ctrl },
    [0x04] = { td_key_d,             td_mod_ctrl },
    [0x05] = { td_key_e,             td_mod_ctrl },
    [0x06] = { td_key_f,             td_mod_ctrl },
    [0x07] = { td_key_g,             td_mod_ctrl },
    [0x08] = { td_key_backspace,     td_mod_ctrl },
    [0x09] = { td_key_tab,           0},
    [0x0A] = { td_key_enter,         0 },
    [0x0B] = { td_key_k,             td_mod_ctrl },
    [0x0C] = { td_key_l,             td_mod_ctrl },
    [0x0D] = { td_key_m,             td_mod_ctrl },
    [0x0E] = { td_key_n,             td_mod_ctrl },
    [0x0F] = { td_key_o,             td_mod_ctrl },
    [0x10] = { td_key_p,             td_mod_ctrl },
    [0x11] = { td_key_q,             td_mod_ctrl },
    [0x12] = { td_key_r,             td_mod_ctrl },
    [0x13] = { td_key_s,             td_mod_ctrl },
    [0x14] = { td_key_t,             td_mod_ctrl },
    [0x15] = { td_key_u,             td_mod_ctrl },
    [0x16] = { td_key_v,             td_mod_ctrl },
    [0x17] = { td_key_w,             td_mod_ctrl },
    [0x18] = { td_key_x,             td_mod_ctrl },
    [0x19] = { td_key_y,             td_mod_ctrl },
    [0x1A] = { td_key_z,             td_mod_ctrl },
    [0x1B] = { td_key_escape,        0 },
    [0x1C] = { td_key_backslash,     td_mod_ctrl },
    [0x1D] = { td_key_right_bracket, td_mod_ctrl },
    [0x1E] = { td_key_6,             td_mod_ctrl | td_mod_shift },
    [0x1F] = { td_key_minus,         td_mod_ctrl | td_mod_shift },
    [' ']  = { td_key_space,         0 },
    ['!']  = { td_key_1,             td_mod_shift },
    ['\"'] = { td_key_astrophe,      td_mod_shift },
    ['#']  = { td_key_3,             td_mod_shift },
    ['$']  = { td_key_4,             td_mod_shift },
    ['%']  = { td_key_5,             td_mod_shift },
    ['&']  = { td_key_7,             td_mod_shift },
    ['\''] = { td_key_astrophe,      0 },
    ['(']  = { td_key_9,             td_mod_shift },
    [')']  = { td_key_0,             td_mod_shift },
    ['*']  = { td_key_8,             td_mod_shift },
    ['+']  = { td_key_equal,         td_mod_shift },
    [',']  = { td_key_comma,         0 },
    ['-']  = { td_key_minus,         0 },
    ['.']  = { td_key_period,        0 },
    ['/']  = { td_key_slash,         0 },
    ['0']  = { td_key_0,             0 },
    ['1']  = { td_key_1,             0 },
    ['2']  = { td_key_2,             0 },
    ['3']  = { td_key_3,             0 },
    ['4']  = { td_key_4,             0 },
    ['5']  = { td_key_5,             0 },
    ['6']  = { td_key_6,             0 },
    ['7']  = { td_key_7,             0 },
    ['8']  = { td_key_8,             0 },
    ['9']  = { td_key_9,             0 },
    [':']  = { td_key_semicolon,     td_mod_shift },
    [';']  = { td_key_semicolon,     0 },
    ['<']  = { td_key_comma,         td_mod_shift },
    ['=']  = { td_key_equal,         0 },
    ['>']  = { td_key_period,        td_mod_shift },
    ['?']  = { td_key_slash,         td_mod_shift },
    ['@']  = { td_key_2,             td_mod_shift },
    ['A']  = { td_key_a,             td_mod_shift },
    ['B']  = { td_key_b,             td_mod_shift },
    ['C']  = { td_key_c,             td_mod_shift },
    ['D']  = { td_key_d,             td_mod_shift },
    ['E']  = { td_key_e,             td_mod_shift },
    ['F']  = { td_key_f,             td_mod_shift },
    ['G']  = { td_key_g,             td_mod_shift },
    ['H']  = { td_key_h,             td_mod_shift },
    ['I']  = { td_key_i,             td_mod_shift },
    ['J']  = { td_key_j,             td_mod_shift },
    ['K']  = { td_key_k,             td_mod_shift },
    ['L']  = { td_key_l,             td_mod_shift },
    ['M']  = { td_key_m,             td_mod_shift },
    ['N']  = { td_key_n,             td_mod_shift },
    ['O']  = { td_key_o,             td_mod_shift },
    ['P']  = { td_key_p,             td_mod_shift },
    ['Q']  = { td_key_q,             td_mod_shift },
    ['R']  = { td_key_r,             td_mod_shift },
    ['S']  = { td_key_s,             td_mod_shift },
    ['T']  = { td_key_t,             td_mod_shift },
    ['U']  = { td_key_u,             td_mod_shift },
    ['V']  = { td_key_v,             td_mod_shift },
    ['W']  = { td_key_w,             td_mod_shift },
    ['X']  = { td_key_x,             td_mod_shift },
    ['Y']  = { td_key_y,             td_mod_shift },
    ['Z']  = { td_key_z,             td_mod_shift },
    ['[']  = { td_key_left_bracket,  0 },
    ['\\'] = { td_key_backslash,     0 },
    [']']  = { td_key_right_bracket, 0 },
    ['^']  = { td_key_6,             td_mod_shift },
    ['_']  = { td_key_minus,         td_mod_shift },
    ['`']  = { td_key_grave_accent,  0 },
    ['a']  = { td_key_a,             0 },
    ['b']  = { td_key_b,             0 },
    ['c']  = { td_key_c,             0 },
    ['d']  = { td_key_d,             0 },
    ['e']  = { td_key_e,             0 },
    ['f']  = { td_key_f,             0 },
    ['g']  = { td_key_g,             0 },
    ['h']  = { td_key_h,             0 },
    ['i']  = { td_key_i,             0 },
    ['j']  = { td_key_j,             0 },
    ['k']  = { td_key_k,             0 },
    ['l']  = { td_key_l,             0 },
    ['m']  = { td_key_m,             0 },
    ['n']  = { td_key_n,             0 },
    ['o']  = { td_key_o,             0 },
    ['p']  = { td_key_p,             0 },
    ['q']  = { td_key_q,             0 },
    ['r']  = { td_key_r,             0 },
    ['s']  = { td_key_s,             0 },
    ['t']  = { td_key_t,             0 },
    ['u']  = { td_key_u,             0 },
    ['v']  = { td_key_v,             0 },
    ['w']  = { td_key_w,             0 },
    ['x']  = { td_key_x,             0 },
    ['y']  = { td_key_y,             0 },
    ['z']  = { td_key_z,             0 },
    ['{']  = { td_key_left_bracket,  td_mod_shift },
    ['|']  = { td_key_backslash,     td_mod_shift },
    ['}']  = { td_key_right_bracket, td_mod_shift },
    ['~']  = { td_key_grave_accent,  td_mod_shift },
    [0x7F] = { td_key_backspace,     0 }
};

static tdp_input_callback tdp_cb = {0};

td_bool tdp_shift_translate = td_true;
td_bool tdp_handle_single_byte(
        const td_i32 byte, td_key_token_t *ch, 
        td_key_mod_t *mods
)
{
    tdp_keymap_entry ke = tdp_keymap[byte];
    if(ke.key != 0)
    {
        if(!tdp_shift_translate &&
            ke.mod & td_mod_shift)
        {
            ke.key = (td_u8)byte;
            ke.mod &= (td_u8)~td_mod_shift;
        }
        *ch = ke.key;
        *mods = ke.mod;
        return td_true;
    }
    return td_false;
}

// Handle navigation keys (Arrow keys, Home, End)
TD_INLINE td_bool tdp_handle_nav_key(const td_i32 byte, td_key_token_t *ch)
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

TD_INLINE td_bool tdp_handle_f5_below(const td_i32 byte, td_key_token_t *ch)
{
    td_i32 tmp = 0;
    if (OUT_RANGE
        ((tmp = byte - 'P' + td_key_f1), td_key_f1, td_key_f4))
        return td_false;
    *ch = (td_key_token_t)tmp;
    return td_true;
}

TD_INLINE td_bool tdp_handle_f5_above(
        const td_i32 first, const td_i32 second, td_key_token_t *ch)
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

TD_INLINE td_bool tdp_handle_combo(const int byte, td_key_mod_t *mods)
{
    switch (byte) {
        case '8': *mods |= (td_mod_ctrl | td_mod_alt | td_mod_shift); break;
        case '7': *mods |= (td_mod_ctrl | td_mod_alt); break;
        case '6': *mods |= (td_mod_ctrl | td_mod_shift); break;
        case '5': *mods |= td_mod_ctrl; break;
        case '4': *mods |= (td_mod_alt | td_mod_shift); break;
        case '3': *mods |= td_mod_alt; break;
        case '2': *mods |= td_mod_shift; break;
        default: return td_false;
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
        int n = (int)_pread(STDIN_FILENO, rb->buffer, 
                (size_t)tdp_min(tdp_kbbyte_available, BUF_SIZE));
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
        char tmp[BUF_SIZE] = {0};

        if(index - end_idx > tdp_kbbyte_available) return -1;
        int n = (int)read(STDIN_FILENO, tmp, 
                (size_t)tdp_min(tdp_kbbyte_available, BUF_SIZE));
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

void tdp_kbpoll(void)
{
     if (!tdp_stdin_ready(0))
         return;
     if ((tdp_kbbyte_available = tdp_stdin_available()) < 1)
         return;

    tdp_ringbuf input_buf = {0};
    td_i32 input_ptr = 0;
    for(;;)
    {
        int probe = input_ptr;
        td_key_token_t ch = 0;
        td_key_mod_t mods = 0;

        int b0 = tdp_rbuf_get(&input_buf, probe);
        if (b0 == -1)
            break;

        if (b0 != 0x1b) {
            probe++;
            if(tdp_handle_single_byte(b0, &ch, &mods))
                tdp_raise(key, ch, td_key_press, mods);
            input_ptr = probe;
            continue;
        }

        int b1 = tdp_rbuf_get(&input_buf, probe + 1);
        if (b1 == -1) {
            goto fallback;
        }

        /* Alt + key */
        if (b1 != '[' && b1 != 'O') {
            int b = tdp_rbuf_get(&input_buf, probe + 1);
            if (b == -1)
                break;

            probe += 2;
            if (tdp_handle_single_byte(b, &ch, &mods))
                tdp_raise(key, ch, td_key_press, mods | td_mod_alt);
            input_ptr = probe;
            continue;
        }

        int b2 = tdp_rbuf_get(&input_buf, probe + 2);
        int b3 = tdp_rbuf_get(&input_buf, probe + 3);
        if (b2 == -1)
            break;

        /* ESC [ x */
        if (b3 != '~' && tdp_handle_nav_key(b2, &ch)) {
            probe += 3;
            input_ptr = probe;
            tdp_raise(key, ch, td_key_press, td_mod_none);
            continue;
        }

        /* ESC O P..S  (F1–F4) */
        if (b1 == 'O' && tdp_handle_f5_below(b2, &ch)) {
            probe += 3;
            input_ptr = probe;
            tdp_raise(key, ch, td_key_press, td_mod_none);
            continue;
        }

        if (b3 == -1)
            break;

//         /* ESC [ x ~ */
//         if (b3 == '~' && handle_f5_below(b2, &ch)) {
//             probe += 4;
//             input_ptr = probe;
//             tdp_raise(key ch, 0, td_key_press);
//             continue;
//         }

        /* ESC [ x ~ */
        if (b3 == '~' && tdp_handle_nav_key(b2, &ch)) 
        {
            probe += 4;
            input_ptr = probe;
            tdp_raise(key, ch, td_key_press, td_mod_none);
            continue;
        }

        int b4 = tdp_rbuf_get(&input_buf, probe + 4);
        if (b4 == -1)
            break;

        /* ESC [ xx ~  (F5–F12) */
        if (b4 == '~' && tdp_handle_f5_above(b2, b3, &ch)) {
            probe += 5;
            input_ptr = probe;
            tdp_raise(key, ch,td_key_press, td_mod_none);
            continue;
        }

        /* ESC [ 1 ; y x   (mod + nav / F1–F4) */
        if (b2 == '1' && b3 == ';') {
            int b5 = tdp_rbuf_get(&input_buf, probe + 5);
            if (b5 == -1)
                break;

            if (tdp_handle_combo(b4, &mods) &&
                (tdp_handle_nav_key(b5, &ch) ||
                 tdp_handle_f5_below(b5, &ch))) {
                probe += 6;
                input_ptr = probe;
                tdp_raise(key, ch, td_key_press, mods);
                continue;
            }
        }


        if (b2 == '<') {
            probe += 3;
            int b = tdp_stoi(&input_buf, &probe);
            if (tdp_rbuf_get(&input_buf, probe++) != ';')
                goto fallback;

            int x = (tdp_stoi(&input_buf, &probe) - 1) /
                     tdp_options[td_opt_pixel_width];
            probe++; /* ; */

            int y = (tdp_stoi(&input_buf, &probe) - 1) /
                     tdp_options[td_opt_pixel_height];

            int type = tdp_rbuf_get(&input_buf, probe++);
            if (type == -1)
                break;

            input_ptr = probe;
            tdp_raise(cursor_pos, x, y);
            continue;
        }

fallback:
        input_ptr++;
        tdp_raise(key, td_key_escape, td_key_press, td_mod_none);
    }
}

tdp_define_setter(key)
tdp_define_setter(mouse_button)
tdp_define_setter(cursor_pos)
