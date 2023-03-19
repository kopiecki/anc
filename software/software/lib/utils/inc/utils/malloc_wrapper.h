/**
 * @file    malloc_wrapper.h
 * @brief   <Provide some short description here>
 */

#ifndef MALLOC_WRAPPER_H_
#define MALLOC_WRAPPER_H_

#include <stddef.h>

void* malloc_wrapper(size_t size);
void* calloc_wrapper(size_t el, size_t size);

#endif /* MALLOC_WRAPPER_H_ */
