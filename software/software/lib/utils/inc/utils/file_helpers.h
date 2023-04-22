/**
 * @file    file_helpers.h
 * @brief   <Provide some short description here>
 */

#ifndef FILE_HELPERS_H_
#define FILE_HELPERS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int file_helpers_write_buf(
        const char* name,
        char delimiter,
        const float* const buffer,
        size_t size,
        bool append);
int file_helpers_read_buf(const char* name, float* const buffer);

#endif /* FILE_HELPERS_H_ */
