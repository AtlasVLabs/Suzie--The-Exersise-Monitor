#pragma once
#include <stdbool.h>
#include "math.h"
#define REP_DECAY_MS 6700 // 6.7 SECOND
#define ACCEL_SIG_MOD_MIN 0.8f
//----------------------------

typedef struct
{
    float sig_filt;
    float noise;
} lp_filt_output_t;

typedef struct
{

    bool isAbove;
    int p_count;
    bool initDecay;
    float rep_decay_start_time;
    float rep_count;

} peak_det_params_t;

void ema_machine(float, float, float *);

void lowpass_filter(lp_filt_output_t *, float, float);

void accel_signal_mod(float *, float, float);

void env_creation_irsf(float *, float, float);

void peak_detection(float, float, float, peak_det_params_t *);