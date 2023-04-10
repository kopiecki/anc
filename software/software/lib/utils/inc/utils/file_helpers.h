/**
 * @file    file_helpers.h
 * @brief   <Provide some short description here>
 */

#ifndef FILE_HELPERS_H_
#define FILE_HELPERS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef FILE_BUFFER_FORMAT
#define FILE_BUFFER_FORMAT float
#endif

int file_helpers_write_buf(
        const char* name,
        char delimiter,
        const FILE_BUFFER_FORMAT* const buffer,
        size_t size,
        bool append);
int file_helpers_read_buf(const char* name, FILE_BUFFER_FORMAT* const buffer);

#endif /* FILE_HELPERS_H_ */
