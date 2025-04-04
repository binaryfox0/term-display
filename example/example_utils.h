#ifndef TEST_UTILITIES_H
#define TEST_UTILITIES_H

#include "term_def.h"

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

#define LOG_INTERVAL 1 // In seconds

double get_time();
char *to_string(const char *format, ...);
char *to_timestamp(double time);

#endif
