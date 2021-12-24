#ifndef RTSETUP_H
#define RTSETUP_H

#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include "rtutils.h"

struct period_info {
    struct timespec next_period;
    long period_ns;
};

void inc_period(struct period_info *pinfo);
void periodic_task_init(struct period_info *pinfo, long period_ns);
void wait_rest_of_period(struct period_info *pinfo);

#endif
