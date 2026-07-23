#pragma once
#include <stdio.h>
#include <stdbool.h>
#include "esp_timer.h"

// alarm callback
typedef void (*timer_callback_t)(void);

typedef struct
{
    int64_t deadline_us;
    bool active;
    timer_callback_t callback;

} alarm_obj;

typedef struct
{
    int64_t start_time_us;
    int64_t duration_us;
    bool active;
} stopwatch_obj;

/// @brief start an alarm event
/// @param delay_us how long it should last for
/// @param callback what function is called after
/// @param alarm an alarm object
void alarm_start(alarm_obj * alarm, int64_t delay, timer_callback_t callback);

/// @brief check up on the alarm and execute callbacks
/// @param alarm an alarm object
void alarm_update(alarm_obj *alarm);

/// @brief reset an alarm event
/// @param alarm an alarm object
void alarm_reset(alarm_obj *alarm);

/// @brief checks if the alarm is running
/// @param alarm an alarm object
/// @return the state of the alarm event
bool alarm_is_active(const alarm_obj *alarm);

/// @brief start a stopwatch event
/// @param stopwatch a stopwatch object
/// @param start_time_us when the event started
void stopwatch_start(stopwatch_obj *stopwatch, int64_t start_time_us);

/// @brief update the duration counter
/// @param stopwatch a stopwatch object
void stopwatch_update(stopwatch_obj *stopwatch);

/// @brief Ends the specified stopwatch event
/// @param stopwatch a stopwatch object
/// @return elapsed time of the event
int16_t stopwatch_end(stopwatch_obj *stopwatch);

/// @brief Returns the elapsed time without affecting the stopwatch event
/// @param stopwatch a stopwatch object
/// @return elapsed time of the event
int64_t stopwatch_elapsed(const stopwatch_obj *stopwatch);