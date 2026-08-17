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
    if (!alarm || alarm->active == false)
        return;

    int64_t now_us = esp_timer_get_time();
    // deadline has passed
    if (now_us >= alarm->deadline_us)
    {
        timer_callback_t callback = alarm->callback;
        alarm->active = false;

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
    stopwatch->duration_us = 0;
}

int64_t stopwatch_end(stopwatch_obj *stopwatch)
{
    int64_t duration;
    if (!stopwatch || stopwatch->active == false)
        return -1;
    duration = stopwatch_elapsed(stopwatch);
    *stopwatch = (stopwatch_obj){0};
    return duration;
}

int64_t stopwatch_elapsed(stopwatch_obj *stopwatch)
{
    if (!stopwatch || stopwatch->active == false)
        return -1;
    stopwatch->duration_us = esp_timer_get_time() - stopwatch->start_time_us;
    return stopwatch->duration_us;
}

bool stopwatch_lap(stopwatch_obj *obj, int64_t *interval_us)
{
    if (!obj || !interval_us)
        return false;

    int64_t now_us = esp_timer_get_time();

    if (!obj->active)
    {
        obj->active = true;
        obj->start_time_us = now_us;
        obj->duration_us = 0;
        return false;
    }

    *interval_us = now_us - obj->start_time_us;

    obj->start_time_us = now_us;
    obj->duration_us = *interval_us;

    return true;
}