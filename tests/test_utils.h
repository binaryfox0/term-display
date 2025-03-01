#ifndef TEST_UTILITIES_H
#define TEST_UTILITIES_H

#include "term_def.h"

#define LOG_INTERVAL 0.1

double get_time();
char* to_string(const char* format, ...);
char* to_timestamp(double time);

#endif
