/**
 * @file    signal.h
 * @brief   <Provide some short description here>
 */

#ifndef SIGNAL_H_
#define SIGNAL_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum signal_type {
        SIGNAL_TYPE_SINE = 0u,
        SIGNAL_TYPE_SQUARE,
        SIGNAL_TYPE_TRIANGLE,
        SIGNAL_TYPE_SAW,
        SIGNAL_TYPE_NOISE,
};

struct signal {
        float amplitude;
        float frequency, sampling_frequency, phase;
        enum signal_type type;
};

void signal_initialize_noise(void);
uint16_t signal_generate(
        struct signal* const self, uint16_t* const buffer, size_t size);

void signal_generate_sum(
        struct signal* signals[],
        uint8_t num_of_signals,
        float* const buffer,
        size_t size);

#endif /* SIGNAL_H_ */
