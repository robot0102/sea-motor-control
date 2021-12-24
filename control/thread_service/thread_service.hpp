#include "rtsetup.h"

// Main functions:
int 
start_periodic_thread(void*(*periodic_func) (void *), void* func_arg, int priority);