/**
 * @file    file_helpers.h
 * @brief   File helpers
 */

#ifndef FILE_HELPERS_H_
#define FILE_HELPERS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Write buffer to file
 * 
 * @param name          File name
 * @param delimiter     Delimiter 
 * @param buffer        Input buffer
 * @param size          Input buffer size
 * @param append        Append after delimiter
 * @return int          Error code, 0 if success
 */
int file_helpers_write_buf(
        const char* name,
        char delimiter,
        const float* const buffer,
        size_t size,
        bool append);

/**
 * @brief Read buffer from file
 * 
 * @param name          File name
 * @param buffer        Output buffer
 * @return int          Error code, 0 if success
 */
int file_helpers_read_buf(const char* name, float* const buffer);

#endif /* FILE_HELPERS_H_ */
