#pragma once
#include <stdbool.h>
#include "math.h"
#include "timers.h"

#define NOISE_THRESHOLD 0.35f
#define CD_THRESHOLD_UPPER 4
#define CD_THRESHOLD_LOWER -4

//----------------------------
// float gyro_alpha = 0.6f; //an agressive filter for the gyroscope
//----------------------------

typedef struct
{
    float sig_filt;
    float noise;
} lp_filt_output_t;

typedef struct
{
    bool isAbove;
    float p_interval_s;
    int p_count;
    bool peakFlag;

    bool interval_complete; // new result is available
} peak_det_params_t;

typedef enum
{
    neutral,
    positive,
    negative
} cycle_sig_direction;



// lp_filt_output_t gx_filt = {0};
// lp_filt_output_t gy_filt = {0};
// lp_filt_output_t gz_filt = {0};

void ema_machine(float, float, float *);

void lowpass_filter(lp_filt_output_t *lpo, float alpha, float sig_source);

void accel_signal_mod(float *, float, float);

void env_creation_irsf(float *, float, float);

bool cycle_detection(float sig);