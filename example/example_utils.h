#ifndef TEST_UTILITIES_H
#define TEST_UTILITIES_H

#include "td_def.h"

#define TESTS_LOGGING

#ifdef TESTS_LOGGING
term_bool start_logging(const char *filename);
void write_log(const char *format, ...);
term_bool stop_logging();
#else
#define start_logging(tmp) 0
#define stop_logging() 0
#define write_log(tmp1, ...)
#endif

#define LOG_INTERVAL 0.1 // In seconds

double get_time();
char *to_string(const char *format, ...);
char *to_timestamp(double time);

enum {
    NO_ARG = 0,
    REQUIRED_ARG,
    OPTIONAL_ARG
};

typedef struct {
    const char *name; // Long option name
    int has_arg;      // NO_ARG, REQUIRED_ARG, OPTIONAL_ARG
    int *flag;        // If not NULL, set *flag to val and return 0
    int val;          // Value to return or store in *flag
} option;

extern char *optarg;  // Argument for current option
extern int optind;    // Index of next argv[] to process
extern int opterr;    // Whether to print error messages
extern int optopt;    // Last known option character

int getopt_long(int argc, char * const argv[], const char *optstring,
                const option *longopts, int *longindex);

int example_tdopt(int argc, char* const argv[]);

#endif
