/**
 * @file    nlms.h
 * @brief   NLMS algorithm header.
 */

#ifndef NLMS_H_
#define NLMS_H_

#include <stddef.h>

/**
 * @brief Calculate new filter weights estimate with normalized least means
 * squares algorithm.
 *
 * @param w     Previous filter weights, updated after calculation
 * @param order Filter order (lenght of filter weights array)
 * @param cl_x  Circural fifo with input samples
 * @param error Calculated error, defined as difference from estimated output
 *              and real output
 * @param step  Algorithm step
 * @param cnt   Sample offset
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
