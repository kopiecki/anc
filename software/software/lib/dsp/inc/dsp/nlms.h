/**
 * @file    nlms.h
 * @brief   <Provide some short description here>
 */

#ifndef NLMS_H_
#define NLMS_H_

#include <stddef.h>

/**
 * @brief Calculate new filter weights estimate with normalized least means
 * squares algorithm.
 *
 * @param w previous filter weights, updated after calculation
 * @param order filter order (lenght of filter weights array)
 * @param cl_x circural fifo with input samples
 * @param error calculated error, defined as difference from estimated output
 * and real output
 * @param step algorithm step
 * @return
 */
void dsp_nlms(
        float* restrict const w,
        float error,
        float step,
        struct circ_fifo* const cf_x,
        size_t order,
        size_t cnt);

#endif /* NLMS_H_ */
