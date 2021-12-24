#ifndef _RTUTILS_H_
#define _RTUTILS_H_

#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#define NS_PER_S   1e9

/* Get the clock time in seconds. */
double get_clock_time_seconds();
void   tare_clock_time();

#endif