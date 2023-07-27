#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include <stdbool.h>

typedef struct Timer Timer;

Timer *
timer_new(void);

void
timer_destroy(Timer *timer);

void
timer_start(Timer *timer);

void
timer_restart(Timer *timer);

void
timer_pause(Timer *timer);

void
timer_unpause(Timer *timer);

bool
timer_paused(Timer *timer);

time_t
timer_get_time(Timer *timer);

#endif // !DEBUG
