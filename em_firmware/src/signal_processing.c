#include "signal_processing.h"
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

void peak_detection(float sig, float p_threshold, float now_ms, peak_det_params_t *params)
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
            params->isAbove = false;
        }
    }
    else
    {
        params->isAbove = false;
        // end of signal clump
        if (sig == 0)
        {
            // turn peak count to reps
            // idea: TAKE ALL THE PEAKS IN A CLUMP AND TURN THOSE INTO REPS
            if (params->p_count > 0)
            {
                params->rep_count += ((float)params->p_count - 1.0f) / 2.0f;
                if (params->rep_count < 0)
                    params->rep_count = 0;

                // reset p_count
                params->p_count = 0;
            }

            if (!params->initDecay)
            {
                // rep decay for half reps
                params->rep_decay_start_time = now_ms;
                params->initDecay = true; // we have started the decay
            }
            // check decay timer
            if (now_ms - params->rep_decay_start_time >= REP_DECAY_MS)
            {
                params->rep_count = floorf(params->rep_count);
            }
        }
    }
    // gather clump time
    // todo: calculate local clump rep intensity

    // printf(">sig:%f\n", sig);
    // printf(">rep_count:%f\n", params->rep_count);
    // printf(">p_count:%d\n", params->p_count);
    // printf(">test1:%d\n", params->isAbove);
}