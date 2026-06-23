#pragma once
#include <stdbool.h>
#include "math.h"
#define REP_DECAY_MS 6700 // 6.7 SECOND
#define ACCEL_SIG_MOD_MIN 0.8f
//----------------------------

typedef struct
{
    float *sig_filt;
    float *noise;
} lp_filt_output;

typedef struct
{

    bool *isAbove;
    int *p_count;
    bool *initDecay;
    float *rep_decay_start_time;
    float *rep_count;

} peak_det_params;

void ema_machine(float, float, float *);

void lowpass_filter(float, float, lp_filt_output *);

float accel_signal_mod(float, float);

float env_creation_irsf(float, float);

void peak_detection();