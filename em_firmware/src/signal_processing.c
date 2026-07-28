#include "signal_processing.h"
#include "timers.h"
#include <stdio.h>

float p_interval;

void ema_machine(float source, float alpha, float *output)
{
    *output = *output + alpha * (source - *output);
}

void lowpass_filter(lp_filt_output_t *lpo, float alpha, float sig_source)
{

    ema_machine(sig_source, alpha, &lpo->sig_filt);
    lpo->noise = sig_source - lpo->sig_filt;
}

void accel_signal_mod(float *sig_mod, float sig, float cutoff)
{
    // modifying the signal a bit
    if (sig > 0)
        *sig_mod = sqrtf(sig) + .5; // add a baseline and sqrt the signal to compress the signal. We just need to count peaks here
    else
        *sig_mod = 0;

    // clamping to stop with noise
    if (sig <= cutoff)
        *sig_mod = 0; // clamping down our modded signal based off the filtered signal
}

void env_creation_irsf(float *env, float sig, float fall_alpha)
{
    // define the envelope for peak detection
    if (sig > *env)
        *env = sig; // instant rise
    else
        // env = env + fall_alpha * (sig - env); // slow fall
        ema_machine(sig, fall_alpha, env); // slow fall
}

void peak_detection(float sig, float p_threshold, peak_det_params_t *params)
{
    // does the signal pass a threshold to not be noise?
    if (sig > ACCEL_SIG_MOD_MIN)
    {
        if (sig > p_threshold)
        {
            params->isAbove = true;
        }
        if (params->isAbove && sig <= p_threshold)
        {
            params->p_count++;
            params->peakFlag = !params->peakFlag;
            params->isAbove = false;

            int64_t interval_us;

            if (stopwatch_lap(&params->s_obj, &interval_us))
            {
                params->p_interval_s =
                    interval_us / 1000000.0f;
            }
        }
    }
    else
    {
        params->isAbove = false;
    }
}