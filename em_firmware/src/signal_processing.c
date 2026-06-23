#include "signal_processing.h"

void ema_machine(float source, float alpha, float *output)
{
    *output = *output + alpha * (source - *output);
}

void lowpass_filter(float alpha, float sig_source, lp_filt_output *out)
{
    ema_machine(sig_source, alpha, out->sig_filt);
    *out->noise = sig_source - *out->sig_filt;
}

float accel_signal_mod(float sig, float cutoff)
{
    float sig_mod = 0;
    // modifying the signal a bit
    if (sig > 0)
        sig_mod = sqrtf(sig) + .5; // add a baseline and sqrt the signal to compress the signal. We just need to count peaks here
    else
        sig_mod = 0;

    // clamping to stop with noise
    if (sig <= cutoff)
        sig_mod = 0; // clamping down our modded signal based off the filtered signal

    return sig_mod;
}

float env_creation_irsf(float sig, float fall_alpha)
{
    float env = 0;
    // define the envelope for peak detection
    if (sig > env)
        env = sig; // instant rise
    else
        // env = env + fall_alpha * (sig - env); // slow fall
        ema_machine(sig, fall_alpha, &env); // slow fall
    return env;
}

void peak_detection(float sig, float p_threshold, float now_ms, peak_det_params *params)
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
        *params->isAbove = false;
        // end of signal clump
        if (sig == 0)
        {
            // turn peak count to reps
            // idea: TAKE ALL THE PEAKS IN A CLUMP AND TURN THOSE INTO REPS
            if (params->p_count > 0)
            {
                *params->rep_count += ((float)*params->p_count - 1.0f) / 2.0f;
                if (*params->rep_count < 0)
                    *params->rep_count = 0;

                // reset p_count
                params->p_count = 0;
            }

            if (!params->initDecay)
            {
                // rep decay for half reps
                *params->rep_decay_start_time = now_ms;
                params->initDecay = true; // we have started the decay
            }
            // check decay timer
            if (now_ms - *params->rep_decay_start_time >= REP_DECAY_MS)
            {
                *params->rep_count = floorf(*params->rep_count);
            }
        }
    }
    // gather clump time
    // todo: calculate local clump rep intensity
}