#include "signal_processing.h"

void ema_machinef(ema_params *params)
{
    params->output = params->output + params->alpha * (params->source - params->output);
}

void accel_processing(accel_proc_params *params)
{
    // Seperate noise from motion
    params->accel_filt = params->accel_filt + params->accel_alpha * (params->a_abs - params->accel_filt);
    params->noise = params->a_abs - params->accel_filt;

    // modifying the signal a bit
    if (params->accel_filt > 0)
        params->accel_filt_mod = sqrtf(params->accel_filt) + .5; // add a baseline and sqrt the signal to compress the signal. We just need to count peaks here
    else
        params->accel_filt_mod = 0;

    // clamping to stop with noise
    if (params->accel_filt <= params->cutoff)
        params->accel_filt_mod = 0; // clamping down our modded signal based off the filtered signal

    // define the envelope for peak detection
    if (params->accel_filt_mod > params->p_env)
        params->p_env = params->accel_filt_mod; // instant rise
    else
        params->p_env = params->p_env + params->p_alpha_release * (params->accel_filt_mod - params->p_env); // slow fall

    // threshold (95% of the envelope)
    params->p_threshold = params->p_env * .95;

    if (params->accel_filt_mod > 0.8f)
    {
        if (params->accel_filt_mod > params->p_threshold)
        {
            params->isAbove = true;
        }
        if (params->isAbove && params->accel_filt_mod <= params->p_threshold)
        {
            params->p_count++;
            params->isAbove = false;
        }

        // new signal start? reset rep decay timer
        params->initDecay = false;
        params->rep_decay_start_time = 0;
    }
    else
    {
        params->isAbove = false;
        // end of a signal clump
        if (params->accel_filt_mod == 0)
        {
            // turn p_count into reps
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
                params->rep_decay_start_time = params->now_ms;
                params->initDecay = true; // we have started the decay
            }

            // check decay timer
            if (params->now_ms - params->rep_decay_start_time >= REP_DECAY_MS)
            {
                params->rep_count = floorf(params->rep_count);
            }
        }
    }

    // Find the average energy / intensity
    params->effort_avg = params->accel_filt * 0.05 + (1 - 0.05) * params->effort_avg;
}

void lowpass_filter(lp_filt_params *params)
{
    ema_params ep = {
        .alpha = params->alpha,
        .source = params->sig_source,
        .output = params->sig_filt};
    ema_machinef(&ep);
    params->noise = params->sig_source - params->sig_filt;
}

float accel_signal_mod(asm_params *params)
{
    // modifying the signal a bit
    if (params->sig > 0)
        params->sig_mod = sqrtf(params->sig) + .5; // add a baseline and sqrt the signal to compress the signal. We just need to count peaks here
    else
        params->sig_mod = 0;

    // clamping to stop with noise
    if (params->sig <= params->cutoff)
        params->sig_mod = 0; // clamping down our modded signal based off the filtered signal

    return params->sig_mod;
}

float env_creation_irsf(envc_params *params)
{
    // define the envelope for peak detection
    if (params->sig > params->env)
        params->env = params->sig; // instant rise
    else
        params->env = params->env + params->fall_alpha * (params->sig - params->env); // slow fall

    return params->env;
}

void peak_detection(peak_det_params *params)
{
    float p_threshold = params->p_env * params->env_fraction;
    // does the signal pass a threshold to not be noise?
    if (params->sig > ACCEL_SIG_MOD_MIN)
    {
        if (params->sig > p_threshold)
        {
            params->isAbove = true;
        }
        if (params->isAbove && params->sig <= p_threshold)
        {
            params->p_count++;
        }
    }
}