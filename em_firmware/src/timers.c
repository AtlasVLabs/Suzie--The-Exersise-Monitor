#include "timers.h"

void alarm_start(int64_t delay, timer_callback_t callback, alarm_obj *alarm);
void alarm_update(alarm_obj *alarm);
void alarm_cancel(alarm_obj *alarm);
bool alarm_is_active(const alarm_obj *alarm);
void stopwatch_start(stopwatch_obj *stopwatch, int64_t start_time_us);
int16_t stopwatch_end(stopwatch_obj *stopwatch);
int64_t stopwatch_elapsed(const stopwatch_obj *stopwatch);