#include <signal.h>
#include <time.h>

#include "tick.h"

static void timer_signal_handler(int signum, siginfo_t *info, void *context)
{
    (void)signum; (void)info; (void)context;
    tick_cb();
}

void tick_init(void)
{
    timer_t timeout_timer;

    /* set up handling signal SIGRTMIN */
    struct sigaction sigact;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_sigaction = timer_signal_handler;
    sigact.sa_flags = SA_SIGINFO;
    sigaction(SIGRTMIN, &sigact, NULL);

    /* create timer to trigger SIGRTMIN */
    struct sigevent sigevt;
    sigevt.sigev_notify = SIGEV_SIGNAL;
    sigevt.sigev_signo = SIGRTMIN;
    sigevt.sigev_value.sival_ptr = NULL;
    timer_create(CLOCK_REALTIME, &sigevt, &timeout_timer);

    /* set first expiry and subsequent interval time */
    struct itimerspec alarm;
    alarm.it_value.tv_sec     = 0;
    alarm.it_value.tv_nsec    = 1'000'000'000 / TICK_HZ;
    alarm.it_interval.tv_sec  = 0;
    alarm.it_interval.tv_nsec = 1'000'000'000 / TICK_HZ;

    /* starting timer */
    timer_settime(timeout_timer, 0, &alarm, NULL);
}
