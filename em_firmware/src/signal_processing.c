#include "signal_processing.h"

void ema_machinef(ema_params *params)
{
    params->output = params->output + params->alpha * (params->source - params->output);
}