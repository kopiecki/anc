/**
 * @file    lms.h
 * @brief   <Provide some short description here>
 */

#ifndef LMS_H_
#define LMS_H_

#include <stddef.h>

#ifndef __FORMAT
#define DSP_FORMAT float
#endif

/**
 * @brief Calculate new filter weights estimate with least means squares
 * algorithm.
 *
 * @param w previous filter weights, updated after calculation
 * @param order filter order (lenght of filter weights array)
 * @param cl_x circural fifo with input samples
 * @param error calculated error, defined as difference from estimated output
 * and real output
 * @param step algorithm step
 * @return
 */
void dsp_lms(
        DSP_FORMAT* const w,
        DSP_FORMAT error,
        DSP_FORMAT step,
        void* const cf_x,
        size_t order,
        size_t cnt);

#endif /* LMS_H_ */
