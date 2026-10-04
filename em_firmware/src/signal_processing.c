#include "signal_processing.h"
#include "timers.h"
#include <stdio.h>

float p_interval;
cycle_sig_direction dir;
bool beenCounted;

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

// void peak_detection(float sig, float p_threshold, peak_det_params_t *params)
// {
//     params->interval_complete = false;

//     if (sig > 0.1f)
//         params->interval_signal_sum += sig;
//     else
//         params->interval_signal_sum = 0.0f;

//     // does the signal pass a threshold to not be noise?
//     if (sig <= ACCEL_SIG_MOD_MIN)
//     {
//         params->isAbove = false;
//         return;
//     }

//     if (sig > p_threshold)
//     {
//         params->isAbove = true;
//     }
//     if (params->isAbove && sig <= p_threshold)
//     {
//         params->p_count++;
//         params->peakFlag = !params->peakFlag;
//         params->isAbove = false;

//         int64_t interval_us;

//         // returns true at the end of an interval
//         if (stopwatch_lap(&params->s_obj, &interval_us))
//         {
//             params->p_interval_s =
//                 interval_us / 1000000.0f;

//             params->interval_complete = true;
//         }

//         params->interval_signal_sum = 0;
//     }
// }

bool peak_detection(float sig, float p_threshold, peak_det_params_t *params)
{
    // greater than the peak threshold?
    if (sig > p_threshold)
    {
        params->isAbove = true;
        return false;
    }
    // transition state
    if (params->isAbove && sig <= p_threshold)
    {
        params->p_count++;
        params->peakFlag = !params->peakFlag;
        params->isAbove = false;
        return true;
    }
    return false; // fail-safe
}


bool transition_detection(float sig)
{
    if (sig >= CD_THRESHOLD_UPPER)
    {
        dir = positive;
        if (beenCounted)
            beenCounted = false;
    }
    else if (sig <= CD_THRESHOLD_LOWER)
    {
        dir = negative;
        if (beenCounted)
            beenCounted = false;
    }
    else
    {
        dir = neutral;
        if (!beenCounted)
        {
            beenCounted = true;
            return beenCounted;
        }
    }

    return false;
}

// bool transition_detection(float sig)
// {
//     if (sig >= CD_THRESHOLD_UPPER)
//     {
//         if (last_extreme == negative)
//         {
//             last_extreme = positive;
//             return true;
//         }
//         // transition to positive from negative
//         last_extreme = positive;
//     }
//     else if (sig <= CD_THRESHOLD_LOWER)
//     {
//         if (last_extreme == positive)
//         {
//             last_extreme = true;
//             return true;
//         }
//         // transition to positive from negative
//         last_extreme = negative;
//     }

//     return false;
// }