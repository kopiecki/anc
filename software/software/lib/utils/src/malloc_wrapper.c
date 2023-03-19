/**
 * @file    malloc_wrapper.c
 * @brief   <Provide some short description here>
 */

#include "utils/malloc_wrapper.h"

#include <stdlib.h>

#include "logger/logger.h"

void* malloc_wrapper(size_t size)
{
        void* p = malloc(size);
        if (p == NULL)
        {
                LOG_ERROR("Cannot allocate memory, exiting...");
                exit(EXIT_FAILURE);
        }

        return p;
}

void* calloc_wrapper(size_t el, size_t size)
{
        void* p = calloc(el, size);
        if (p == NULL)
        {
                LOG_DEBUG("Cannot allocate memory, exiting...");
                exit(EXIT_FAILURE);
        }

        return p;
}
