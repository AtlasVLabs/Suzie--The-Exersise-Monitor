#pragma once
#include <stdbool.h>
#include "math.h"
#define REP_DECAY_MS 6700 // 6.7 SECOND
#define ACCEL_SIG_MOD_MIN 0.8f

typedef struct
{
    float source;
    float alpha;
    float output;
} ema_params;

typedef struct
{
    float alpha;
    float sig_source;
    float sig_filt;
    float noise;
} lp_filt_params;

typedef struct
{
    float sig;
    float sig_mod;
    float cutoff;
} asm_params;

typedef struct
{
    float sig;
    bool isAbove;
    float env_fraction;
    float p_env;
    int p_count

} peak_det_params;

typedef struct
{
    float sig;
    float env;
    float rise_alpha;
    float fall_alpha;
} envc_params;

typedef struct
{
    float accel_filt;
    double accel_alpha;
    float noise;
    float cutoff;
    float accel_filt_mod;
    float p_alpha_release;
    float p_env;
    bool isAbove;
    float p_threshold;
    int p_count;
    bool initDecay;
    float rep_decay_start_time;
    float rep_count;
    float now_ms;
    float a_abs;
    float effort_avg;

} accel_proc_params;

void ema_machinef(ema_params *);

void accel_processing(accel_proc_params *);

void lowpass_filter(lp_filt_params *);

float accel_signal_mod(asm_params *);