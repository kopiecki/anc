/**
 * @file    signal.h
 * @brief   Signal generators collection header.
 */

#ifndef SIGNAL_H_
#define SIGNAL_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Signal types.
 */
enum signal_type {
        SIGNAL_TYPE_SINE = 0u,
        SIGNAL_TYPE_SQUARE,
        SIGNAL_TYPE_TRIANGLE,
        SIGNAL_TYPE_SAW,
        SIGNAL_TYPE_NOISE,
};

/**
 * @brief Signal state holder.
 */
struct signal {
        float amplitude;
        float frequency, sampling_frequency, phase;
        enum signal_type type;
};

/**
 * @brief Initialize random noise signal seed (rand() implemenation).
 */
void signal_initialize_noise(void);

/**
 * @brief Generate specifed signal.
 * 
 * @param self          Signal state holder.
 * @param buffer        Output buffer, NULL if only 1 sample is generated.
 * @param size          Output buffer size, 1 if 1 sample is generated.
 * @return float        Output sample, none if buffer is filled.
 */
float signal_generate(
        struct signal* const self, uint16_t* const buffer, size_t size);

/**
 * @brief Generate sum of specified signals.
 * 
 * @param signals               Array of signals
 * @param num_of_signals        Number of signals
 * @param buffer                Output buffer, NULL if only 1 sample is generated.
 * @param size                  Output buffer size, 1 if only 1 sample.
 * @return float                Output sample, none if buffer is filled.
 */
float signal_generate_sum(
        struct signal* signals[],
        uint8_t num_of_signals,
        float* const buffer,
        size_t size);

#endif /* SIGNAL_H_ */
