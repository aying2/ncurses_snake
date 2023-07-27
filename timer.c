#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

struct Timer
{
    time_t start_time;
    time_t pause_time;
    time_t unpause_time;
    bool started;
    bool paused;
};

Timer *
timer_new(void)
{
    Timer *timer = malloc(sizeof *timer);

    timer->start_time = 0;
    timer->pause_time = 0;
    timer->unpause_time = 0;
    timer->started = false;
    timer->paused = false;

    return timer;
}

void
timer_destroy(Timer *timer)
{
    free(timer);
}

void
timer_start(Timer *timer)
{
    if (timer->started == false)
    {
        timer->start_time = time(NULL);
        timer->started = true;
    }
    else
    {
        fprintf(stderr, "bad timer start");
    }
}

void
timer_restart(Timer *timer)
{
    if (timer->started == true)
    {
        timer->start_time = time(NULL);
        timer->paused = false;
        timer->pause_time = 0;
        timer->unpause_time = 0;
    }
    else
    {
        fprintf(stderr, "bad timer restart");
    }
}

void
timer_pause(Timer *timer)
{
    if (timer->started == false)
    {
        fprintf(stderr, "bad timer pause: not started");
    }
    else if (timer->paused == true)
    {
        fprintf(stderr, "bad timer pause: already paused");
    }
    else
    {
        timer->paused = true;
        timer->pause_time = time(NULL);
    }
}

void
timer_unpause(Timer *timer)
{
    if (timer->started == false)
    {
        fprintf(stderr, "bad timer unpause: not started");
    }
    else if (timer->paused == false)
    {
        fprintf(stderr, "bad timer unpause: already unpaused");
    }
    else
    {
        timer->paused = false;
        timer->unpause_time = time(NULL);
    }
}

bool
timer_paused(Timer *timer)
{
    return timer->paused;
}

time_t
timer_get_time(Timer *timer)
{
    if (timer->started == false)
    {
        return 0;
    }
    else if (timer->paused == true)
    {
        return timer->pause_time - timer->start_time;
    }
    else
    {
        return time(NULL) - timer->start_time - (timer->unpause_time - timer->pause_time);
    }
}
