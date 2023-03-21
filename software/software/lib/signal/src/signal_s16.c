/**
 * @file    signal_s16.c
 * @brief   <Provide some short description here>
 */

#include "signal/signal_s16.h"

#include <math.h>
#include <stddef.h>

#include "utils/assert.h"

#define SIGNAL_MAX_PHASE M_PI * 2.0f

static void signal_s16_generate_sine(struct signal* const this)
{
        ASSERT(this != NULL);

        double step = SIGNAL_MAX_PHASE * this->parameters.frequency
                      / this->parameters.sampling_frequency;
        for (uint32_t i = 0u; i < this->samples.size; i++)
        {
                this->samples.buffer[i] =
                        this->parameters.amplitude
                        * sin(this->phase + this->parameters.phase_offset);
        }

        this->phase += step;
        if (this->phase >= SIGNAL_MAX_PHASE)
                this->phase += SIGNAL_MAX_PHASE;
}

static void signal_s16_generate_square(struct signal* const this) {}

static const void (*signal_s16_generators[SIGNAL_S16_TYPE_GUARD])(
        struct signal_s16* const this) = {
        signal_s16_generate_sine,
        signal_s16_generate_square,
};

void signal_s16_generate(struct signal_s16* const this, enum signal_s16_type type)
{
        ASSERT(this != NULL);
        ASSERT(type < SIGNAL_S16_TYPE_GUARD);

        signal_s16_generators[type](this);
}
