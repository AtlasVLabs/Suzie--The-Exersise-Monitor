#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "max30102.h"
#include "bmi160.h"
#include "i2c_bus.h"
#include "imu_service.h"
#include "signal_processing.h"
#include "timers.h"

#define USER_LED GPIO_NUM_21
#define SET_TIMEOUT_US 5000000LL

// REP OBJECT ------------------------------
// Stores the finalized metrics for a single completed rep.
typedef struct
{
    // What rep is this
    int rep_index;
    // Maximum angle of the primary angle reached during the rep.
    float angle_max;
    // Minimum angle of the primary angle reached during the rep.
    float angle_min;
    // Range of motion for the rep.
    // Calculated as angle_max - angle_min.
    float ROM;
    // Total accumulated movement effort for the rep.
    float effort;
    // Total duration of the rep, in microseconds.
    int64_t duration_us;
} rep_obj;

typedef struct
{
    int set_index;
    int rep_count;
    int64_t set_duration_us;

    float avg_ROM;
    float avg_effort;
    float avg_rep_duration_us;

    float rec_effort;
    float rec_ROM;
    float rec_rep_duration_us;

} set_obj;

// variables---------------------------------------------
// Low-pass filter smoothing factor for acceleration data.
float accel_alpha = 0.08f;
// Integrated gyroscope angle around the X-axis, in radians.
float gx_angle;
// Integrated gyroscope angle around the Y-axis, in radians.
float gy_angle;
// Integrated gyroscope angle around the Z-axis, in radians.
float gz_angle;
// Maximum X-axis angle recorded during the current rep.
float gx_angle_max;
// Minimum X-axis angle recorded during the current rep.
float gx_angle_min;
// Accumulated movement effort for the current rep.
float rep_effort = 0.0f;
// Alpha value for recent metrics
float set_ema_alpha = 0.30f;
// Number of detected movement phases in the current set.
// Two phases represent one complete rep.
int phase_count = 0;
// Index used for phase-related data storage or processing.
int phase_index;
// Number of sets done per workout
int set_count;
// Number of initial sensor samples ignored to allow sensor
// and filter values to stabilize.
int startup_samples = 20;
// Duration of the completed set, in microseconds.
int64_t setDuration_us = 0;
// Time elapsed between consecutive sensor samples, in microseconds.
int64_t sample_interval_us;
// ESP timer timestamp marking the beginning of the current set,
// in microseconds.
int64_t set_start_time;
// Duration of the most recently completed rep, in microseconds.
int64_t last_rep_duration_us;
// Stores the most recent accelerometer sample read from the IMU.
accel_samples_t accel_sample = {0};
// Stores the most recent gyroscope sample read from the IMU.
gyro_samples_t gyro_sample = {0};
// Stores the state and output of the acceleration low-pass filter.
lp_filt_output_t accel_filt_out = {0};
// Stores the state and output of the X-axis gyroscope low-pass filter.
lp_filt_output_t gyro_x_filt_out = {0};
// Stores the state and output of the Y-axis gyroscope low-pass filter.
// Reserved for future multi-axis processing.
lp_filt_output_t gyro_y_filt_out = {0};
// Stores the state and output of the Z-axis gyroscope low-pass filter.
// Reserved for future multi-axis processing.
lp_filt_output_t gyro_z_filt_out = {0};
// Stopwatch used to measure the interval between consecutive sensor samples.
stopwatch_obj sample_watch = {0};
// Inactivity alarm used to determine when the current workout set has ended.
alarm_obj set_alarm_obj = {0};
// Stopwatch used to measure the duration of the current rep.
stopwatch_obj rep_watch = {0};
// Stores the metrics for the most recently completed rep.
rep_obj curr_rep_obj = {0};
// Stores the metrics for the most recently completed set.
set_obj curr_set_obj = {0};
// True while a workout set is currently active.
// Becomes false after the set inactivity timeout expires.
bool set_active = false;
// Callback executed when the set inactivity alarm expires.
void end_of_set_event();

void update_set_metrics(set_obj *set, rep_obj *rep);

void finalize_set(set_obj *set);

void app_main()
{
    // inits-------------------------------------------------

    gpio_config_t led_config = {
        .pin_bit_mask = 1ULL << USER_LED,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&led_config);
    gpio_set_level(USER_LED, 0);
    vTaskDelay(pdMS_TO_TICKS(5000));
    fflush(stdout);
    i2c_bus_init();
    fflush(stdout);

    BMI160_init(get_master_bus_handle());
    bmi160_config_t bmiconfig = {
        .apm = bmi160_accel_pwr_normal,
        .gpm = bmi160_gyro_pwr_normal};
    BMI160_config(&bmiconfig);
    fflush(stdout);

    while (1)
    {
        if (startup_samples > 0)
        {
            startup_samples--;
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        // READING ACCEL VALUES
        read_accel(&accel_sample);
        // Reading gyro values
        read_gyro(&gyro_sample);
        // timer for each sample read
        bool valid_dt =
            stopwatch_lap(&sample_watch, &sample_interval_us);

        //-------------------------------------------------------------
        // Gyro stuff
        //-------------------------------------------------------------

        // remove the bias (hardcoded)
        float gx_corrected = gyro_sample.gx_rad_s;
        float gy_corrected = gyro_sample.gy_rad_s;
        float gz_corrected = gyro_sample.gz_rad_s;

        // Seperate noise from motion
        lowpass_filter(&accel_filt_out, accel_alpha, accel_sample.a_abs);

        // integration
        if (valid_dt)
        {
            float dt_s = sample_interval_us / 1000000.0f;

            if (dt_s > 0.0f && dt_s < 0.1f)
            {
                // integrating effort only when a rep is active
                if (rep_watch.active)
                    rep_effort += accel_filt_out.sig_filt * dt_s;
                // getting the angles
                if (gx_corrected > 0.5 || gx_corrected < -0.5)
                {
                    gx_angle += gx_corrected * dt_s;
                    gy_angle += gy_corrected * dt_s;
                    gz_angle += gz_corrected * dt_s;
                }
            }
        }
        // phase transition detection
        bool transition_detected = transition_detection(gx_corrected);
        // set alarm update logic
        if (alarm_is_active(&set_alarm_obj))
        {
            // update the counter
            alarm_update(&set_alarm_obj);
            // end of a set?
            if (!alarm_is_active(&set_alarm_obj))
            {
                set_active = false;
            }
        }

        // reps
        if (transition_detected)
        {

            // start of set
            if (!set_active)
            {
                set_active = true;
                set_start_time = esp_timer_get_time();
                phase_count = 0;
                curr_set_obj.rep_count = 0;

                // start an inactivity alarm
                if (!set_alarm_obj.active)
                    alarm_start(&set_alarm_obj, SET_TIMEOUT_US, end_of_set_event);
            }
            // reset alarm with each peak??
            if (alarm_is_active(&set_alarm_obj))
            {
                alarm_reset(&set_alarm_obj, SET_TIMEOUT_US);
            }

            phase_count++; // a rep is divided into 2 phases
            if (phase_count % 2 == 0 && phase_count > 0)
            {
                if (rep_watch.active) // end of a rep
                {
                    last_rep_duration_us = stopwatch_end(&rep_watch);
                    curr_set_obj.rep_count++;

                    // finalize rep
                    curr_rep_obj = (rep_obj){
                        .rep_index = curr_set_obj.rep_count,
                        .angle_max = gx_angle_max,
                        .angle_min = gx_angle_min,
                        .duration_us = last_rep_duration_us,
                        .ROM = gx_angle_max - gx_angle_min,
                        .effort = rep_effort};

                    update_set_metrics(&curr_set_obj, &curr_rep_obj);
                }
                stopwatch_start(&rep_watch);
                // reset variables at the start
                gx_angle_max = gx_angle;
                gx_angle_min = gx_angle;
                rep_effort = 0;
            }
        }

        // look for the max and min angle
        if (gx_angle_max < gx_angle)
            gx_angle_max = gx_angle;
        if (gx_angle_min > gx_angle)
            gx_angle_min = gx_angle;
        //-----------------------------------------------------
        // debuging purposes

        // printf(">gx:%f\n", gx_corrected);
        // printf(">angle_max:%f\n", curr_rep_obj.angle_max);
        // printf(">angle_min:%f\n", curr_rep_obj.angle_min);
        // printf(">duration_us:%lld\n", (long long)curr_rep_obj.duration_us);
        // printf(">ROM:%f\n", curr_rep_obj.ROM);
        // printf(">effort:%f\n", curr_rep_obj.effort);

        printf(">rep_index:%d\n", curr_rep_obj.rep_index);

        printf(">ROM:%f\n", curr_rep_obj.ROM);
        printf(">avg_ROM:%f\n", curr_set_obj.avg_ROM);
        printf(">rec_ROM:%f\n", curr_set_obj.rec_ROM);

        printf(">effort:%f\n", curr_rep_obj.effort);
        printf(">avg_effort:%f\n", curr_set_obj.avg_effort);
        printf(">rec_effort:%f\n", curr_set_obj.rec_effort);

        printf(">rep_duration_us:%lld\n",
               (long long)curr_rep_obj.duration_us);

        printf(">avg_rep_duration_us:%f\n",
               curr_set_obj.avg_rep_duration_us);

        printf(">rec_rep_duration_us:%f\n",
               curr_set_obj.rec_rep_duration_us);

        printf(">ROM_recent_vs_avg:%f\n",
               curr_set_obj.rec_ROM -
                   curr_set_obj.avg_ROM);

        printf(">effort_recent_vs_avg:%f\n",
               curr_set_obj.rec_effort -
                   curr_set_obj.avg_effort);

        printf(">duration_recent_vs_avg:%f\n",
               curr_set_obj.rec_rep_duration_us -
                   curr_set_obj.avg_rep_duration_us);

        fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void end_of_set_event()
{
    // reset rep metrics
    last_rep_duration_us = 0;
    gx_angle_max = 0;
    gx_angle_min = 0;
    gx_angle = 0;
    rep_effort = 0;

    set_count++;
    finalize_set(&curr_set_obj);

    // reset current rep obj
    curr_rep_obj = (rep_obj){0};
    // reset the current set obj
    curr_set_obj = (set_obj){0};

    // reset rep watch
    stopwatch_end(&rep_watch);
}

void update_set_metrics(set_obj *set, rep_obj *rep)
{
    int n = set->rep_count;
    set->set_index = set_count;

    // update the avg metrics
    set->avg_ROM +=
        (rep->ROM - set->avg_ROM) /
        set->rep_count;
    ;

    set->avg_effort +=
        (rep->effort - set->avg_effort) /
        set->rep_count;
    ;

    set->avg_rep_duration_us +=
        ((float)rep->duration_us -
         set->avg_rep_duration_us) /
        set->rep_count;
    ;

    // update the recent metrics
    if (set->rep_count == 1)
    {
        set->rec_ROM = rep->ROM;
        set->rec_effort = rep->effort;
        set->rec_rep_duration_us =
            (float)rep->duration_us;
    }
    else
    {
        ema_machine(
            rep->ROM,
            set_ema_alpha,
            &set->rec_ROM);

        ema_machine(
            rep->effort,
            set_ema_alpha,
            &set->rec_effort);

        ema_machine(
            (float)rep->duration_us,
            set_ema_alpha,
            &set->rec_rep_duration_us);
    }
}

void finalize_set(set_obj *set)
{
    set->set_duration_us =
        esp_timer_get_time() - set_start_time - SET_TIMEOUT_US;

    // anything else that only becomes known
    // once the set is definitely over
}