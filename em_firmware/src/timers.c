#include "timers.h"

void alarm_start(alarm_obj *alarm, int64_t delay_us, timer_callback_t callback)
{
    if (!alarm)
        return;

    // populate the alarm object
    alarm->active = true;
    alarm->callback = callback;
    alarm->deadline_us = esp_timer_get_time() + delay_us;
}

void alarm_update(alarm_obj *alarm)
{
    if (!alarm || alarm->active == true)
        return;

    int64_t now_us = esp_timer_get_time();
    // deadline has passed
    if (now_us >= alarm->deadline_us)
    {
        timer_callback_t callback = alarm->callback;

        if (callback != NULL)
            callback();
    }
}

void alarm_reset(alarm_obj *alarm)
{
    if (!alarm)
        return;
    *alarm = (alarm_obj){0};
}

bool alarm_is_active(const alarm_obj *alarm)
{
    if (!alarm)
        return;
    return alarm->active;
}

void stopwatch_start(stopwatch_obj *stopwatch, int64_t start_time_us)
{
    if (!stopwatch || stopwatch->active == true)
        return;

    // populate the stopwatch
    stopwatch->active = true;
    stopwatch->start_time_us = esp_timer_get_time();
    stopwatch->duration_us = 0;
}

void stopwatch_update(stopwatch_obj *stopwatch)
{
    if (!stopwatch || stopwatch->active == false)
        return;
    stopwatch->duration_us = stopwatch->start_time_us - esp_timer_get_time();
}

int16_t stopwatch_end(stopwatch_obj *stopwatch)
{
    if (!stopwatch || stopwatch->active == false)
        return;
    *stopwatch = (stopwatch_obj){0};
}

int64_t stopwatch_elapsed(const stopwatch_obj *stopwatch)
{
    if(!stopwatch || stopwatch->active == false)
        return;
    return stopwatch->duration_us;
}