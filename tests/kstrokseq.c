#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <termios.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pty.h>
#include <termios.h>
#include <libgen.h>

#define error(fmt, ...) fprintf(stderr, "\x1b[1;31merror\x1b[0m: " fmt "\n", ##__VA_ARGS__)
#define info(fmt, ...) fprintf(stderr, "\x1b[1;34minfo\x1b[0m: " fmt "\n", ##__VA_ARGS__)

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
    int pass;
    int expected[];
} test_entry;
test_entry tests[] = {
    {"thisisatest"},
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
        usleep(500000);
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
