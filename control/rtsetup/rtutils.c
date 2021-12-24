#include "rtutils.h"

static struct timespec start_time;

// Get the clock time in seconds:
double get_clock_time_seconds() {
    // Struct to hold the time:
    static struct timespec curtime;
    double time_in_seconds = 0.0;
    
    clock_gettime(CLOCK_MONOTONIC, &curtime);

    time_in_seconds = (curtime.tv_sec - start_time.tv_sec)*1.0;
    time_in_seconds += (curtime.tv_nsec - start_time.tv_nsec)/NS_PER_S*1.0;

    return time_in_seconds;
}

void tare_clock_time() {
    // Set the start time to the current time:
    clock_gettime(CLOCK_MONOTONIC, &start_time);
}