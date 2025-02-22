#include "test_utils.h"

#if defined(_WIN64) || defined(_WIN32)
 #include <windows.h>
 double get_time() {
  LARGE_INTEGER frequency, counter;
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&counter);
  return (double)counter.QuadPart / frequency.QuadPart;
 }
#else
 #include <time.h>
 double get_time() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + ts.tv_nsec / 1e9;
 }
#endif
