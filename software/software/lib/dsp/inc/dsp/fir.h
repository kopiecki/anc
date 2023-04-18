/**
 * @file    fir.h
 * @brief   <Provide some short description here>
 */

#ifndef FIR_H_
#define FIR_H_

#include <stddef.h>

#ifndef DSP_FORMAT
#define DSP_FORMAT float
#endif

/**
 * @brief Calculate FIR filter output.
 * @param cl circural fifo with input samples
 * @param w filter weights (coefficients, impulse response)
 * @param order filter order (lenght of filter weights arrray)
 * @return Filter output
 */
float dsp_fir(void* const cl, const DSP_FORMAT* const w, size_t order, size_t cnt);

#endif /* FIR_H_ */
