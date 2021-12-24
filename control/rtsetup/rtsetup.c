#include "rtsetup.h"

void 
inc_period(struct period_info *pinfo)
{
    pinfo->next_period.tv_nsec += pinfo->period_ns;

    while (pinfo->next_period.tv_nsec >= NS_PER_S) {
		/* timespec nsec overflow */
		pinfo->next_period.tv_sec++;
		pinfo->next_period.tv_nsec -= NS_PER_S;
    }
}

void 
periodic_task_init(struct period_info *pinfo, long period_ns)
{
    pinfo->period_ns = period_ns;

    clock_gettime(CLOCK_MONOTONIC, &(pinfo->next_period));
}

void 
wait_rest_of_period(struct period_info *pinfo)
{
    inc_period(pinfo);

    /* for simplicity, ignoring possibilities of signal wakes */
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &pinfo->next_period, NULL);
}
