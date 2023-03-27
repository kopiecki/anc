/**
 * @file    signal.h
 * @brief   <Provide some short description here>
 */

#ifndef SIGNAL_H_
#define SIGNAL_H_

#include <stddef.h>
#include <stdint.h>

#ifndef SIGNAL_FORMAT
#define SIGNAL_FORMAT uint16_t
#endif

enum signal_type
{
        SIGNAL_TYPE_SINE = 0u,
        SIGNAL_TYPE_SQUARE,
        SIGNAL_TYPE_TRIANGLE,
        SIGNAL_TYPE_SAW,
        SIGNAL_TYPE_NOISE,
};

struct signal
{
        double frequency, sampling_frequency, phase;
#if SIGNAL_FORMAT == double
        double amplitude;
#endif
        enum signal_type type;
};

void signal_generate(struct signal* const self, void* buffer, size_t size);

#endif /* SIGNAL_H_ */
