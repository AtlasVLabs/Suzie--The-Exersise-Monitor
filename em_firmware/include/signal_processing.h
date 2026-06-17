#pragma once

typedef struct
{
    float source;
    float alpha;
    float output;
} ema_params;

typedef struct
{
    float accel_filt;
    float noise;
    float cutoff;
    float accel_filt_mod;
    float p_alpha_release;
    float p_env;

} accel_proc_params;

void ema_machinef(ema_params *params);

void accel_processing();