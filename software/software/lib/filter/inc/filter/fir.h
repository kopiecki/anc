/**
 * @file    fir.h
 * @brief   <Provide some short description here>
 */

#ifndef FIR_H_
#define FIR_H_

#include "utils/circ_lifo.h"

float fir_filter(struct circ_lifo* const cl, const float* const imp_res, size_t imp_res_size);

#endif /* FIR_H_ */
