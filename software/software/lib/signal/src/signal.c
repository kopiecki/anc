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

#define SIGNAL_FORMAT_UINT16 true

#define UINT16_BUFFER ((uint16_t*)buffer)
#define DOUBLE_BUFFER ((double*)buffer)

#define SIGNAL_MAX_PHASE 2.0 * M_PI

static inline void signal_sine_calc(
        struct signal* const self,
        void* buffer,
        size_t i)
{
#ifdef SIGNAL_FORMAT_UINT16
        UINT16_BUFFER[i] = INT16_MAX * (1 + sin(self->phase));
#else
        DOUBLE_BUFFER[i]      = self->amplitude * sin(self->phase);
#endif
}

static inline void signal_square_calc(
        struct signal* const self,
        void* buffer,
        size_t i)
{
#ifdef SIGNAL_FORMAT_UINT16
        signal_sine_calc(self, buffer, i);
        UINT16_BUFFER[i] = (UINT16_BUFFER[i] > INT16_MAX) ? UINT16_MAX : 0u;
#else
        CAST_BUFFER_TO_DOUBLE = (signal_sine_calc(self->phase) > 0.0) ?
                                        self->amplitude :
                                        -self->amplitude;
#endif
}

static inline void signal_triangle_calc(
        struct signal* const self,
        void* buffer,
        size_t i)
{
#ifdef SIGNAL_FORMAT_UINT16
        UINT16_BUFFER[i] =
                (1 + ((2 / M_PI) * asin(sin(self->phase)))) * INT16_MAX;
#else
        return (1 + ((2 / M_PI) * asin(sin(self->phase)))) * self->amplitude;
#endif
}

static inline void signal_saw_calc(
        struct signal* const self,
        void* buffer,
        size_t i)
{
#ifdef SIGNAL_FORMAT_UINT16
#else
#endif
}

static inline void signal_noise_calc(
        struct signal* const self,
        void* buffer,
        size_t i)
{
#ifdef SIGNAL_FORMAT_UINT16
        UINT16_BUFFER[i] = rand() & UINT16_MAX;
#endif
}

static void signal_generator_template(
        struct signal* const self,
        void (*calculation)(struct signal* const self, void* buffer, size_t i),
        void* buffer,
        size_t size)
{
        ASSERT(self != NULL);
        ASSERT(buffer != NULL);

        const double angle =
                SIGNAL_MAX_PHASE * self->frequency / self->sampling_frequency;
        for (size_t i = 0u; i < size; i++)
        {
                self->phase += angle;
                calculation(self, buffer, i);

                if (self->phase >= SIGNAL_MAX_PHASE)
                        self->phase -= SIGNAL_MAX_PHASE;
        }
}

static void (*signal_calculators[])(
        struct signal* const self,
        void* buffer,
        size_t i) = {
        signal_sine_calc,
        signal_square_calc,
        signal_triangle_calc,
        signal_saw_calc,
        signal_noise_calc,
};

void signal_generate(struct signal* const self, void* buffer, size_t size)
{
        if (self->type == SIGNAL_TYPE_NOISE)
                srand(time(NULL));

        signal_generator_template(
                self, signal_calculators[self->type], buffer, size);
}
