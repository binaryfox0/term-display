#include "td_input.h"

#include <ctype.h>

#include "td_rbuf.h"
#include "td_window_priv.h"
#include "td_term.h"

#define tdp_raise(name, ...) \
    if(TDP_CALLBACK_NAME(name)) \
        TDP_CALLBACK_NAME(name)(window, __VA_ARGS__)

typedef struct tdp_keymap_entry
{
    td_u16 key;
    td_u8 mod;
} tdp_keymap_entry;

static const tdp_keymap_entry tdp_keymap[(1 << (sizeof(td_u8) * 8))] = 
{
    [0x00] = { TD_KEY_SPACE,         TD_MOD_CTRL },
    [0x01] = { TD_KEY_A,             TD_MOD_CTRL },
    [0x02] = { TD_KEY_B,             TD_MOD_CTRL },
    [0x03] = { TD_KEY_C,             TD_MOD_CTRL },
    [0x04] = { TD_KEY_D,             TD_MOD_CTRL },
    [0x05] = { TD_KEY_E,             TD_MOD_CTRL },
    [0x06] = { TD_KEY_F,             TD_MOD_CTRL },
    [0x07] = { TD_KEY_G,             TD_MOD_CTRL },
    [0x08] = { TD_KEY_BACKSPACE,     TD_MOD_CTRL },
    [0x09] = { TD_KEY_TAB,           TD_MOD_NONE },
    [0x0A] = { TD_KEY_ENTER,         TD_MOD_NONE },
    [0x0B] = { TD_KEY_K,             TD_MOD_CTRL },
    [0x0C] = { TD_KEY_L,             TD_MOD_CTRL },
    [0x0D] = { TD_KEY_M,             TD_MOD_CTRL },
    [0x0E] = { TD_KEY_N,             TD_MOD_CTRL },
    [0x0F] = { TD_KEY_O,             TD_MOD_CTRL },
    [0x10] = { TD_KEY_P,             TD_MOD_CTRL },
    [0x11] = { TD_KEY_Q,             TD_MOD_CTRL },
    [0x12] = { TD_KEY_R,             TD_MOD_CTRL },
    [0x13] = { TD_KEY_S,             TD_MOD_CTRL },
    [0x14] = { TD_KEY_T,             TD_MOD_CTRL },
    [0x15] = { TD_KEY_U,             TD_MOD_CTRL },
    [0x16] = { TD_KEY_V,             TD_MOD_CTRL },
    [0x17] = { TD_KEY_W,             TD_MOD_CTRL },
    [0x18] = { TD_KEY_X,             TD_MOD_CTRL },
    [0x19] = { TD_KEY_Y,             TD_MOD_CTRL },
    [0x1A] = { TD_KEY_Z,             TD_MOD_CTRL },
    [0x1B] = { TD_KEY_ESCAPE,        TD_MOD_NONE },
    [0x1C] = { TD_KEY_BACKSLASH,     TD_MOD_CTRL },
    [0x1D] = { TD_KEY_RIGHT_BRACKET, TD_MOD_CTRL },
    [0x1E] = { TD_KEY_6,             TD_MOD_CTRL | TD_MOD_SHIFT },
    [0x1F] = { TD_KEY_MINUS,         TD_MOD_CTRL | TD_MOD_SHIFT },
    [' ']  = { TD_KEY_SPACE,         TD_MOD_NONE },
    ['!']  = { TD_KEY_1,             TD_MOD_SHIFT },
    ['\"'] = { TD_KEY_ASTROPHE,      TD_MOD_SHIFT },
    ['#']  = { TD_KEY_3,             TD_MOD_SHIFT },
    ['$']  = { TD_KEY_4,             TD_MOD_SHIFT },
    ['%']  = { TD_KEY_5,             TD_MOD_SHIFT },
    ['&']  = { TD_KEY_7,             TD_MOD_SHIFT },
    ['\''] = { TD_KEY_ASTROPHE,      TD_MOD_NONE },
    ['(']  = { TD_KEY_9,             TD_MOD_SHIFT },
    [')']  = { TD_KEY_0,             TD_MOD_SHIFT },
    ['*']  = { TD_KEY_8,             TD_MOD_SHIFT },
    ['+']  = { TD_KEY_EQUAL,         TD_MOD_SHIFT },
    [',']  = { TD_KEY_COMMA,         TD_MOD_NONE },
    ['-']  = { TD_KEY_MINUS,         TD_MOD_NONE },
    ['.']  = { TD_KEY_PERIOD,        TD_MOD_NONE },
    ['/']  = { TD_KEY_SLASH,         TD_MOD_NONE },
    ['0']  = { TD_KEY_0,             TD_MOD_NONE },
    ['1']  = { TD_KEY_1,             TD_MOD_NONE },
    ['2']  = { TD_KEY_2,             TD_MOD_NONE },
    ['3']  = { TD_KEY_3,             TD_MOD_NONE },
    ['4']  = { TD_KEY_4,             TD_MOD_NONE },
    ['5']  = { TD_KEY_5,             TD_MOD_NONE },
    ['6']  = { TD_KEY_6,             TD_MOD_NONE },
    ['7']  = { TD_KEY_7,             TD_MOD_NONE },
    ['8']  = { TD_KEY_8,             TD_MOD_NONE },
    ['9']  = { TD_KEY_9,             TD_MOD_NONE },
    [':']  = { TD_KEY_SEMICOLON,     TD_MOD_SHIFT },
    [';']  = { TD_KEY_SEMICOLON,     TD_MOD_NONE },
    ['<']  = { TD_KEY_COMMA,         TD_MOD_SHIFT },
    ['=']  = { TD_KEY_EQUAL,         TD_MOD_NONE },
    ['>']  = { TD_KEY_PERIOD,        TD_MOD_SHIFT },
    ['?']  = { TD_KEY_SLASH,         TD_MOD_SHIFT },
    ['@']  = { TD_KEY_2,             TD_MOD_SHIFT },
    ['A']  = { TD_KEY_A,             TD_MOD_SHIFT },
    ['B']  = { TD_KEY_B,             TD_MOD_SHIFT },
    ['C']  = { TD_KEY_C,             TD_MOD_SHIFT },
    ['D']  = { TD_KEY_D,             TD_MOD_SHIFT },
    ['E']  = { TD_KEY_E,             TD_MOD_SHIFT },
    ['F']  = { TD_KEY_F,             TD_MOD_SHIFT },
    ['G']  = { TD_KEY_G,             TD_MOD_SHIFT },
    ['H']  = { TD_KEY_H,             TD_MOD_SHIFT },
    ['I']  = { TD_KEY_I,             TD_MOD_SHIFT },
    ['J']  = { TD_KEY_J,             TD_MOD_SHIFT },
    ['K']  = { TD_KEY_K,             TD_MOD_SHIFT },
    ['L']  = { TD_KEY_L,             TD_MOD_SHIFT },
    ['M']  = { TD_KEY_M,             TD_MOD_SHIFT },
    ['N']  = { TD_KEY_N,             TD_MOD_SHIFT },
    ['O']  = { TD_KEY_O,             TD_MOD_SHIFT },
    ['P']  = { TD_KEY_P,             TD_MOD_SHIFT },
    ['Q']  = { TD_KEY_Q,             TD_MOD_SHIFT },
    ['R']  = { TD_KEY_R,             TD_MOD_SHIFT },
    ['S']  = { TD_KEY_S,             TD_MOD_SHIFT },
    ['T']  = { TD_KEY_T,             TD_MOD_SHIFT },
    ['U']  = { TD_KEY_U,             TD_MOD_SHIFT },
    ['V']  = { TD_KEY_V,             TD_MOD_SHIFT },
    ['W']  = { TD_KEY_W,             TD_MOD_SHIFT },
    ['X']  = { TD_KEY_X,             TD_MOD_SHIFT },
    ['Y']  = { TD_KEY_Y,             TD_MOD_SHIFT },
    ['Z']  = { TD_KEY_Z,             TD_MOD_SHIFT },
    ['[']  = { TD_KEY_LEFT_BRACKET,  TD_MOD_NONE },
    ['\\'] = { TD_KEY_BACKSLASH,     TD_MOD_NONE },
    [']']  = { TD_KEY_RIGHT_BRACKET, TD_MOD_NONE },
    ['^']  = { TD_KEY_6,             TD_MOD_SHIFT },
    ['_']  = { TD_KEY_MINUS,         TD_MOD_SHIFT },
    ['`']  = { TD_KEY_GRAVE_ACCENT,  TD_MOD_NONE },
    ['a']  = { TD_KEY_A,             TD_MOD_NONE },
    ['b']  = { TD_KEY_B,             TD_MOD_NONE },
    ['c']  = { TD_KEY_C,             TD_MOD_NONE },
    ['d']  = { TD_KEY_D,             TD_MOD_NONE },
    ['e']  = { TD_KEY_E,             TD_MOD_NONE },
    ['f']  = { TD_KEY_F,             TD_MOD_NONE },
    ['g']  = { TD_KEY_G,             TD_MOD_NONE },
    ['h']  = { TD_KEY_H,             TD_MOD_NONE },
    ['i']  = { TD_KEY_I,             TD_MOD_NONE },
    ['j']  = { TD_KEY_J,             TD_MOD_NONE },
    ['k']  = { TD_KEY_K,             TD_MOD_NONE },
    ['l']  = { TD_KEY_L,             TD_MOD_NONE },
    ['m']  = { TD_KEY_M,             TD_MOD_NONE },
    ['n']  = { TD_KEY_N,             TD_MOD_NONE },
    ['o']  = { TD_KEY_O,             TD_MOD_NONE },
    ['p']  = { TD_KEY_P,             TD_MOD_NONE },
    ['q']  = { TD_KEY_Q,             TD_MOD_NONE },
    ['r']  = { TD_KEY_R,             TD_MOD_NONE },
    ['s']  = { TD_KEY_S,             TD_MOD_NONE },
    ['t']  = { TD_KEY_T,             TD_MOD_NONE },
    ['u']  = { TD_KEY_U,             TD_MOD_NONE },
    ['v']  = { TD_KEY_V,             TD_MOD_NONE },
    ['w']  = { TD_KEY_W,             TD_MOD_NONE },
    ['x']  = { TD_KEY_X,             TD_MOD_NONE },
    ['y']  = { TD_KEY_Y,             TD_MOD_NONE },
    ['z']  = { TD_KEY_Z,             TD_MOD_NONE },
    ['{']  = { TD_KEY_LEFT_BRACKET,  TD_MOD_SHIFT },
    ['|']  = { TD_KEY_BACKSLASH,     TD_MOD_SHIFT },
    ['}']  = { TD_KEY_RIGHT_BRACKET, TD_MOD_SHIFT },
    ['~']  = { TD_KEY_GRAVE_ACCENT,  TD_MOD_SHIFT },
    [0x7F] = { TD_KEY_BACKSPACE,     TD_MOD_NONE }
};

td_bool tdp_shift_translate = TD_TRUE;
static td_bool tdp_handle_single_byte(
        const td_i32 byte, td_key_token_t *ch, 
        td_key_mod_t *mods
)
{
    tdp_keymap_entry ke = tdp_keymap[byte];
    if(ke.key != 0)
    {
        if(!tdp_shift_translate &&
            ke.mod & TD_MOD_SHIFT)
        {
            ke.key = (td_u8)byte;
            ke.mod &= (td_u8)~TD_MOD_SHIFT;
        }
        *ch = ke.key;
        *mods = ke.mod;
        return TD_TRUE;
    }
    return TD_FALSE;
}

// Handle navigation keys (Arrow keys, Home, End)
TD_INLINE td_bool tdp_handle_nav_key(const td_i32 byte, td_key_token_t *ch)
{
    switch (byte)      
    {
        case 'A': *ch = TD_KEY_UP; break;
        case 'B': *ch = TD_KEY_DOWN; break;
        case 'C': *ch = TD_KEY_RIGHT; break;
        case 'D': *ch = TD_KEY_LEFT; break;
        case 'H': *ch = TD_KEY_HOME; break;
        case 'F': *ch = TD_KEY_END; break;
        case '2': *ch = TD_KEY_INSERT; break;
        case '3': *ch = TD_KEY_DELETE; break;
        case '5': *ch = TD_KEY_PAGE_UP; break;
        case '6': *ch = TD_KEY_PAGE_DOWN; break;
        default:
            return TD_FALSE;
    }
    return TD_TRUE;
}

TD_INLINE td_bool tdp_handle_f5_below(const td_i32 byte, td_key_token_t *ch)
{
    td_i32 tmp = 0;
    if (TDP_OUT_RANGE
        ((tmp = byte - 'P' + TD_KEY_F1), TD_KEY_F1, TD_KEY_F4))
        return TD_FALSE;
    *ch = (td_key_token_t)tmp;
    return TD_TRUE;
}

TD_INLINE td_bool tdp_handle_f5_above(
        const td_i32 first, const td_i32 second, td_key_token_t *ch)
{
    if (first == '1') 
    {
        switch (second) 
        {
            case '5': *ch = TD_KEY_F5; break;
            case '7': *ch = TD_KEY_F6; break;
            case '8': *ch = TD_KEY_F7; break;
            case '9': *ch = TD_KEY_F8; break;
            default: return TD_FALSE;
        }
    } else if (first == '2') {
        switch (second) 
        {
            case '0': *ch = TD_KEY_F9;  break;
            case '1': *ch = TD_KEY_F10; break;
            case '3': *ch = TD_KEY_F11; break;
            case '4': *ch = TD_KEY_F12; break;
            default: return TD_FALSE;
        }
    } else {
        return TD_FALSE;
    }
    return TD_TRUE;
}

TD_INLINE td_bool tdp_handle_combo(const int byte, td_key_mod_t *mods)
{
    switch (byte) {
        case '8': *mods |= (TD_MOD_CTRL | TD_MOD_ALT | TD_MOD_SHIFT); break;
        case '7': *mods |= (TD_MOD_CTRL | TD_MOD_ALT); break;
        case '6': *mods |= (TD_MOD_CTRL | TD_MOD_SHIFT); break;
        case '5': *mods |= TD_MOD_CTRL; break;
        case '4': *mods |= (TD_MOD_ALT | TD_MOD_SHIFT); break;
        case '3': *mods |= TD_MOD_ALT; break;
        case '2': *mods |= TD_MOD_SHIFT; break;
        default: return TD_FALSE;
    }
    return TD_TRUE;
}


static int tdp_stoi(tdp_rbuf_t* rb, int *idx)
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

static int tdp_stdin_read_callback(
        void *userdata,
        char *buf,
        int size)
{
    int *available = (int*)userdata;
    int want = 0;
    int n = 0;
    if (*available <= 0)
        return 0;

    want = size;
    if (want > *available)
        want = *available;

    n = (int)tdp_tty_read(buf, (size_t)want);
    if (n > 0)
        *available -= n;

    return n;
}

void tdp_poll_input(
        td_window_t *window,
        TDP_DEFINE_CALLBACK_VAR(key),
        TDP_DEFINE_CALLBACK_VAR(mouse_button),
        TDP_DEFINE_CALLBACK_VAR(cursor_pos))
{
    td_i32 available = 0;
    tdp_rbuf_t input_buf = {0};
    td_i32 input_ptr = 0;
     if (!tdp_term_stdin_ready(0))
         return;
     available = tdp_term_stdin_available();
     if(available < 0)
         return;

     input_buf.read_cb = tdp_stdin_read_callback;
     input_buf.userdata = &available;
(void)mouse_button_callback;
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
                tdp_raise(key, ch, TD_ACTION_PRESS, mods);
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
                tdp_raise(key, ch, TD_ACTION_PRESS, mods | TD_MOD_ALT);
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
            tdp_raise(key, ch, TD_ACTION_PRESS, TD_MOD_NONE);
            continue;
        }

        /* ESC O P..S  (F1–F4) */
        if (b1 == 'O' && tdp_handle_f5_below(b2, &ch)) {
            probe += 3;
            input_ptr = probe;
            tdp_raise(key, ch, TD_ACTION_PRESS, TD_MOD_NONE);
            continue;
        }

        if (b3 == -1)
            break;

//         /* ESC [ x ~ */
//         if (b3 == '~' && handle_f5_below(b2, &ch)) {
//             probe += 4;
//             input_ptr = probe;
//             tdp_raise(key ch, 0, TD_ACTION_PRESS);
//             continue;
//         }

        /* ESC [ x ~ */
        if (b3 == '~' && tdp_handle_nav_key(b2, &ch)) 
        {
            probe += 4;
            input_ptr = probe;
            tdp_raise(key, ch, TD_ACTION_PRESS, TD_MOD_NONE);
            continue;
        }

        int b4 = tdp_rbuf_get(&input_buf, probe + 4);
        if (b4 == -1)
            break;

        /* ESC [ xx ~  (F5–F12) */
        if (b4 == '~' && tdp_handle_f5_above(b2, b3, &ch)) {
            probe += 5;
            input_ptr = probe;
            tdp_raise(key, ch,TD_ACTION_PRESS, TD_MOD_NONE);
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
                tdp_raise(key, ch, TD_ACTION_PRESS, mods);
                continue;
            }
        }


        if (b2 == '<') 
        {
            probe += 3;
            int b = tdp_stoi(&input_buf, &probe);
            if (tdp_rbuf_get(&input_buf, probe++) != ';')
                goto fallback;

            int x = (tdp_stoi(&input_buf, &probe) - 1) /
                     window->pix_size.x;
            probe++; /* ; */

            int y = (tdp_stoi(&input_buf, &probe) - 1) /
                     window->pix_size.y;

            int type = tdp_rbuf_get(&input_buf, probe++);
            if (type == -1)
                break;

            input_ptr = probe;
            tdp_raise(cursor_pos, x, y);
            (void)b;
            continue;
        }

fallback:
        input_ptr++;
        tdp_raise(key, TD_KEY_ESCAPE, TD_ACTION_PRESS, TD_MOD_NONE);
    }
}

