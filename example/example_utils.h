#ifndef TEST_UTILITIES_H
#define TEST_UTILITIES_H

#include "td_def.h"

#define TESTS_LOGGING

#ifdef TESTS_LOGGING
td_bool start_logging(const char *filename);
void write_log(const char *format, ...);
td_bool stop_logging();
#else
#define start_logging(tmp) 0
#define stop_logging() 0
#define write_log(tmp1, ...)
#endif

#define LOG_INTERVAL 0.1 // In seconds

double get_time();
char *to_string(const char *format, ...);
char *to_timestamp(double time);

extern int maximum_fps;
#endif
