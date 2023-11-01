/**
 * @file    lms.h
 * @brief   LMS algorithm header.
 */

#ifndef LMS_H_
#define LMS_H_

#include <stddef.h>

#include "utils/circ_fifo.h"

/**
 * @brief Calculate new filter weights estimate with least means squares
 * algorithm.
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
void dsp_lms(
        float* restrict const w,
        float error,
        float step,
        struct circ_fifo* const cf_x,
        size_t order,
        size_t cnt);

#endif /* LMS_H_ */
