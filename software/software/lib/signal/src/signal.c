/**
 * @file    signal.c
 * @brief   <Provide some short description here>
 */

#include "signal/signal.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "utils/assert.h"
#include "utils/common.h"
#include "utils/logger.h"

#define SIGNAL_MAX_PHASE (float)(2.0 * M_PI)

static inline float signal_normalize(float y, float min, float max)
{
        return 2 * ((y - min) / (max - min)) - 1;
}

static inline float signal_sine_calc(struct signal* const self)
{
        return self->amplitude * (float)sin(self->phase);
}

static inline float signal_square_calc(struct signal* const self)
{
        return (sin(self->phase) > 0.0) ? self->amplitude : -self->amplitude;
}

static inline float signal_triangle_calc(struct signal* const self)
{
        return (1 + ((2 / (float)M_PI) * (float)asin(sin(self->phase))))
               * self->amplitude;
}

static inline float signal_saw_calc(struct signal* const self)
{
        UNUSED_PARAMETER(self);

        return 0.0f;
}

static inline float signal_noise_calc(struct signal* const self)
{
        return (2 * ((float)rand() / (float)RAND_MAX) - 1) * self->amplitude;
}

static void signal_generator_template(
        struct signal* const self,
        float (*calculation)(struct signal* const self),
        float* const buffer,
        size_t size)
{
        ASSERT(self != NULL);
        ASSERT(buffer != NULL);

        const float angle =
                SIGNAL_MAX_PHASE * self->frequency / self->sampling_frequency;
        for (size_t i = 0u; i < size; i++) {
                self->phase += angle;
                buffer[i] = calculation(self);

                if (self->phase >= SIGNAL_MAX_PHASE)
                        self->phase -= SIGNAL_MAX_PHASE;
        }
}

static float (*signal_calculators[])(struct signal* const self) = {
        signal_sine_calc,
        signal_square_calc,
        signal_triangle_calc,
        signal_saw_calc,
        signal_noise_calc,
};

void signal_generate(struct signal* const self, float* const buffer, size_t size)
{
        signal_generator_template(
                self, signal_calculators[self->type], buffer, size);
}

void signal_generate_sum(
        struct signal* signals[], uint8_t num_of_signals, float* buffer, size_t size)
{
        float max = 1;
        float min = -1;

        for (size_t i = 0u; i < size; i++) {
                float y = 0;
                for (uint8_t j = 0u; j < num_of_signals; ++j) {
                        float temp = 0;
                        signal_generator_template(
                                signals[j],
                                signal_calculators[signals[j]->type],
                                &temp,
                                1u);
                        y += temp;
                }

                if (y > max)
                        max = y;
                else if (y < min)
                        min = y;

                buffer[i] = y;
        }

        for (size_t i = 0u; i < size; i++)
                buffer[i] = signal_normalize(buffer[i], min, max);
}

void signal_initialize_noise(void)
{
        srand((unsigned int)time(NULL));
}
