/**
 * @file    fir.c
 * @brief   <Provide some short description here>
 */

#include "filter/fir.h"

#include "logger/logger.h"
#include "utils/assert.h"

float fir_filter(struct circ_lifo* const cl, const float* const imp_res,
		 size_t imp_res_size)
{
	ASSERT(cl != NULL);
	ASSERT(imp_res != NULL);

	float y = 0.0f;
	for (size_t i = 0u; i < imp_res_size; i++)
		y += imp_res[i] * circ_lifo_peek(cl, i);
	cl->i = (cl->i + 1u) & cl->mask;

	return y;
}
