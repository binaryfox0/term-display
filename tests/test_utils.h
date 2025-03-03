#ifndef TEST_UTILITIES_H
#define TEST_UTILITIES_H

#include "term_def.h"

#define TESTS_LOGGING

#ifdef TESTS_LOGGING
u8 start_logging(const char* filename);
void write_log(const char* format, ...);
u8 stop_logging();
#else
 #define start_logging() 0
 #define stop_logging() 0
#endif
 #define LOG_INTERVAL 0.1

double get_time();
char* to_string(const char* format, ...);
char* to_timestamp(double time);

#endif
