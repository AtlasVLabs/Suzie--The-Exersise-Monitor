#include "signal_processing.h"
#include "timers.h"
#include <stdio.h>

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

bool detect_phase_transition(float sig, phase_detection_t *params)
{
    if (params == NULL)
    {
        return false;
    }

    // We define a hysteresis limit to avoid "chattering" or bouncing near the thresholds.
    // If the threshold to enter positive is CD_THRESHOLD_UPPER (4.0),
    // we only return to neutral if we fall below CD_HYSTERESIS_UPPER (e.g. 1.0).
    // If the threshold to enter negative is CD_THRESHOLD_LOWER (-4.0),
    // we only return to neutral if we rise above CD_HYSTERESIS_LOWER (e.g. -1.0).
    const float CD_HYSTERESIS_UPPER = 1.0f;
    const float CD_HYSTERESIS_LOWER = -1.0f;

    switch (params->state)
    {
        case PHASE_STATE_NEUTRAL:
            if (sig >= CD_THRESHOLD_UPPER)
            {
                params->state = PHASE_STATE_POSITIVE;
            }
            else if (sig <= CD_THRESHOLD_LOWER)
            {
                params->state = PHASE_STATE_NEGATIVE;
            }
            break;

        case PHASE_STATE_POSITIVE:
            if (sig <= CD_HYSTERESIS_UPPER)
            {
                params->state = PHASE_STATE_NEUTRAL;
                return true; // Transition from active positive movement to neutral detected!
            }
            break;

        case PHASE_STATE_NEGATIVE:
            if (sig >= CD_HYSTERESIS_LOWER)
            {
                params->state = PHASE_STATE_NEUTRAL;
                return true; // Transition from active negative movement to neutral detected!
            }
            break;
    }

    return false;
}