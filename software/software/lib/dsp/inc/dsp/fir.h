/**
 * @file    fir.h
 * @brief   <Provide some short description here>
 */

#ifndef FIR_H_
#define FIR_H_

#include <stddef.h>

#include "utils/circ_fifo.h"

/**
 * @brief Calculate FIR filter output.
 * @param cl circural fifo with input samples
 * @param w filter weights (coefficients, impulse response)
 * @param order filter order (lenght of filter weights arrray)
 * @return Filter output
 */
float dsp_fir(
        struct circ_fifo* const cl,
        const float* restrict const w,
        size_t order,
        size_t cnt);

#endif /* FIR_H_ */
