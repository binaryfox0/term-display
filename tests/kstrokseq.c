#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <termios.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pty.h>
#include <termios.h>
#include <libgen.h>
#include <limits.h>

#define error(fmt, ...) fprintf(stderr, "\x1b[1;31merror\x1b[0m: " fmt "\n", ##__VA_ARGS__)
#define info(fmt, ...) fprintf(stderr, "\x1b[1;34minfo\x1b[0m: " fmt "\n", ##__VA_ARGS__)

#define pack(mod, key, times) ((mod) << 24 | (key) << 8 | (times))

/**
 * @enum td_key_token_t
 * @brief Enumeration for key tokens.
 * 
 * These values represent various key codes for keyboard keys.
 */
typedef enum {
    td_key_space = 32,        /**< Space key */
    td_key_astrophe = 39,     /**< Apostrophe key */
    td_key_minus = 45,        /**< Minus key */
    td_key_0 = 48,            /**< '0' key */
    td_key_1,                 /**< '1' key */
    td_key_2,                 /**< '2' key */
    td_key_3,                 /**< '3' key */
    td_key_4,                 /**< '4' key */
    td_key_5,                 /**< '5' key */
    td_key_6,                 /**< '6' key */
    td_key_7,                 /**< '7' key */
    td_key_8,                 /**< '8' key */
    td_key_9,                 /**< '9' key */
    td_key_semicolon = 59,    /**< Semicolon key */
    td_key_equal = 61,        /**< Equal key */
    td_key_a = 65,            /**< 'A' key */
    td_key_b,                 /**< 'B' key */
    td_key_c,                 /**< 'C' key */
    td_key_d,                 /**< 'D' key */
    td_key_e,                 /**< 'E' key */
    td_key_f,                 /**< 'F' key */
    td_key_g,                 /**< 'G' key */
    td_key_h,                 /**< 'H' key */
    td_key_i,                 /**< 'I' key */
    td_key_j,                 /**< 'J' key */
    td_key_k,                 /**< 'K' key */
    td_key_l,                 /**< 'L' key */
    td_key_m,                 /**< 'M' key */
    td_key_n,                 /**< 'N' key */
    td_key_o,                 /**< 'O' key */
    td_key_p,                 /**< 'P' key */
    td_key_q,                 /**< 'Q' key */
    td_key_r,                 /**< 'R' key */
    td_key_s,                 /**< 'S' key */
    td_key_t,                 /**< 'T' key */
    td_key_u,                 /**< 'U' key */
    td_key_v,                 /**< 'V' key */
    td_key_w,                 /**< 'W' key */
    td_key_x,                 /**< 'X' key */
    td_key_y,                 /**< 'Y' key */
    td_key_z,                 /**< 'Z' key */
    td_key_left_bracket = 91, /**< Left bracket '[' */
    td_key_backslash,         /**< Backslash '\\' */
    td_key_right_bracket,     /**< Right bracket ']' */
    td_key_grave_accent = 96, /**< Grave accent '`' */
    td_key_escape = 256,      /**< Escape key */
    td_key_enter,             /**< Enter key */
    td_key_tab,               /**< Tab key */
    td_key_backspace,         /**< Backspace key */
    td_key_insert,            /**< Insert key */
    td_key_delete,            /**< Delete key */
    td_key_right,             /**< Right arrow key */
    td_key_left,              /**< Left arrow key */
    td_key_down,              /**< Down arrow key */
    td_key_up,                /**< Up arrow key */
    td_key_page_up,           /**< Page up key */
    td_key_page_down,         /**< Page down key */
    td_key_home,              /**< Home key */
    td_key_end,               /**< End key */
    td_key_f1 = 290,          /**< Function key F1 */
    td_key_f2,                /**< Function key F2 */
    td_key_f3,                /**< Function key F3 */
    td_key_f4,                /**< Function key F4 */
    td_key_f5,                /**< Function key F5 */
    td_key_f6,                /**< Function key F6 */
    td_key_f7,                /**< Function key F7 */
    td_key_f8,                /**< Function key F8 */
    td_key_f9,                /**< Function key F9 */
    td_key_f10,               /**< Function key F10 */
    td_key_f11,               /**< Function key F11 */
    td_key_f12,               /**< Function key F12 */
    __td_key_token_end__
} td_key_token_t;


int find_program(char *buf, size_t out_len)
{
    char path[PATH_MAX] = {0};
    ssize_t path_len = 0;
    if((path_len = readlink("/proc/self/exe", path, sizeof(path) - 1)) == -1) {
        error("unable to get the program directory");
        return 1;
    }
    path[path_len] = '\0';
    char* dir = dirname(path);
    char program_path[PATH_MAX] = {0};
    snprintf(program_path, sizeof(program_path), "%s/../example/multiline_text", dir);
    if(access(program_path, F_OK) != 0)
    {
        error("unable to find \"multline_text\" executable path");
        info("please ensure that this program wasn't moved from its original path after building from source");
        return 1;
    }
    if(strlen(program_path) + 1 > out_len)
        return 1;
    memcpy(buf, program_path, strlen(program_path) + 1);
    return 0;
}

int create_process(const char* program_path, int* master_fd, int* pid)
{
    // Create PTY and fork
    *pid = forkpty(master_fd, NULL, NULL, NULL);
    if (*pid < 0) {
        error("unable to create a new pseudo-terminal");
        error("forkpty: %s", strerror(errno));
        return 1;
    }

    if (*pid == 0) {
        execlp(program_path, program_path, "--keystroke-test", NULL);
        
        error("unable to execute \"multline_text\" program");
        error("execlp: %s", strerror(errno));
        exit(127);
    }

    int status;
    int waited_ms = 0;

    while (1) {
        pid_t r = waitpid(*pid, &status, WNOHANG);
        if (r == *pid) {
            if (WIFEXITED(status) && WEXITSTATUS(status) == 127) {
                error("failed to execute \"multline_text\" program");
                return -1;
            }
            error("something went wrong, likely internal error!");
            info("treat as error");
            return 1;
        }

        if (waited_ms >= 500)
            break;

        usleep(1000);
        waited_ms++;
    }
    tcflush(*master_fd, TCIOFLUSH); // discard all bytes that the program had sent
    return 0;
}

typedef struct {
    const char* seq;
    int* expected;
    int expected_count;
    int pass;
} test_entry;
test_entry tests[] = {
    {"thisisatest", (int[]){
        td_key_h, 1, td_key_a, 1, td_key_e, 2, 
        td_key_i, 2, td_key_t, 3, td_key_s, 3
    }, 6},
    {"a\x1bm"}
};

static const int test_count = sizeof(tests) / sizeof(tests[0]);

void run_tests(const char* program_path)
{
    int master_fd = 0, pid = 0;
    if(create_process(program_path, &master_fd, &pid) != 0) {
        error("unable to create the initial process");
        return;
    }
    for(int i = 0; i < test_count; i++)
    {
        test_entry entry = tests[i];
        write(master_fd, entry.seq, strlen(entry.seq));
        usleep(500000); // wait 500ms
        if(kill(pid, 0) != 0) {
            if(create_process(program_path, &master_fd, &pid) != 0) {
                error("test %d has triggered timeout and exited", i);
                error("unable to recover from program timeout, abort!");
                return;
            }
            continue;
        }
        
        char buf[512] = {0};
        int start_idx = 0;

        ssize_t buf_len = read(master_fd, buf, sizeof(buf));
        if(buf_len < 1) {
            error("failed to read output from program, continue");
            continue;
        }
        for(ssize_t i = 0; i < buf_len; i++)
        {
            if(buf[i] == '\n')
            {
                int key, mods, state;
                sscanf(buf + start_idx, "%d, %d, %d", &key, &mods, &state);
                info("key: %d (%c), mod: %d (%c%c%c), state: %d",
                     key, (key >= ' ' && key <= '~' ? key : 'X'),
                     mods, mods & 1 ? 'X' : ' ', mods & 2 ? 'X' : ' ', mods & 4 ? 'X' : ' ', 
                     state);
                
                start_idx = i + 1;
            }
        }
    }

    for(int i = 0; i < test_count; i++)
        info("test %d: %s", i + 1, (tests[i].pass ? "pass" : "failed"));
}

int main() {
    char program_path[PATH_MAX] = {0};
    if(find_program(program_path, sizeof(program_path)) != 0)
       return 1;

    run_tests(program_path);

    return 0;
}
