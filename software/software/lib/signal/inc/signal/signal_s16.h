/**
 * @file    signal_s16.h
 * @brief   <Provide some short description here>
 */

#ifndef SIGNAL_S16_H_
#define SIGNAL_S16_H_

#include "utils/common.h"

enum signal_s16_type
{
        SIGNAL_S16_SINE,
        SIGNAL_S16_SQUARE,
        SIGNAL_S16_SAW,
        SIGNAL_S16_TYPE_GUARD,
};

struct signal_s16_parameters
{
        int16_t amplitude, frequency, sampling_frequency, phase_offset;
};

struct signal
{
        struct signal_s16_parameters parameters;
        struct buffer_and_size_s16 samples;
        float phase;
};

void signal_s16_generate(struct signal_s16* const this, enum signal_s16_type type);

#endif /* SIGNAL_H_ */
