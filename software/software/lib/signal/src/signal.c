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

#define SIGNAL_MAX_PHASE 2.0 * M_PI

static inline uint16_t signal_sine_calc(double phase)
{
        return INT16_MAX * (1 + sin(phase));
}

static inline uint16_t signal_square_calc(double phase)
{
        return (signal_sine_calc(phase) > INT16_MAX) ? UINT32_MAX : 0u;
}

static inline uint16_t signal_triangle_calc(double phase)
{
        return (1 + ((2 / M_PI) * asin(sin(phase)))) * INT16_MAX;
}

static inline uint16_t signal_saw_calc(double phase)
{
        double y = 0.0;
        for (double i = 0.0; i < 50.0; i++) 
                y += sin(i * phase) / i;

        y *= 2.0 / M_PI;

        return (y * INT16_MAX) + INT16_MAX;
}

static inline uint16_t signal_noise_calc(double phase)
{
        return rand() % UINT32_MAX;
}

static void signal_generator_template(
        struct signal* const self,
        uint16_t (*calculation)(double phase),
        uint16_t* const buffer,
        size_t size)
{
        ASSERT(self != NULL);
        ASSERT(buffer != NULL);

        const double angle =
                SIGNAL_MAX_PHASE * self->frequency / self->sampling_frequency;
        for (size_t i = 0u; i < size; i++)
        {
                self->phase += angle;

                buffer[i] = calculation(self->phase);

                if (self->phase >= SIGNAL_MAX_PHASE)
                        self->phase -= SIGNAL_MAX_PHASE;
        }
}

static uint16_t (*signal_calculators[])(double phase) = {
        signal_sine_calc,
        signal_square_calc,
        signal_triangle_calc,
        signal_saw_calc,
        signal_noise_calc,
};

void signal_generate(struct signal* const self, uint16_t* const buffer, size_t size)
{
        if (self->type == SIGNAL_TYPE_NOISE)
                srand(time(NULL));

        signal_generator_template(
                self, signal_calculators[self->type], buffer, size);
}
