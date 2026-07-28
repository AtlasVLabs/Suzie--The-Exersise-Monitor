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
        return false;
    return alarm->active;
}

void stopwatch_start(stopwatch_obj *stopwatch)
{
    if (!stopwatch || stopwatch->active == true)
        return;

    // populate the stopwatch
    stopwatch->active = true;
    stopwatch->start_time_us = esp_timer_get_time();
}

void stopwatch_update(stopwatch_obj *stopwatch)
{
    if (!stopwatch || stopwatch->active == false)
        return;
    stopwatch->duration_us = esp_timer_get_time() - stopwatch->start_time_us;
}

int16_t stopwatch_end(stopwatch_obj *stopwatch)
{
    int64_t duration;
    if (!stopwatch || stopwatch->active == false)
        return -1;
    duration = stopwatch_elapsed(stopwatch);
    *stopwatch = (stopwatch_obj){0};
    return duration;
}

int64_t stopwatch_elapsed(const stopwatch_obj *stopwatch)
{
    if (!stopwatch || stopwatch->active == false)
        return -1;
    return stopwatch->duration_us;
}

bool stopwatch_lap(stopwatch_obj *obj, int64_t *interval_us)
{
    int64_t now_us = esp_timer_get_time();

    // First peak: begin timing.
    if (!obj->active)
    {
        stopwatch_start(obj);
        return false;
    }

    // Later peaks: measure and restart.
    stopwatch_update(obj);
    stopwatch_start(obj);

    *interval_us = obj->duration_us;

    return true;
}