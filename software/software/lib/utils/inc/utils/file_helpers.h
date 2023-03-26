/**
 * @file    file_helpers.h
 * @brief   <Provide some short description here>
 */

#ifndef FILE_HELPERS_H_
#define FILE_HELPERS_H_

#include <stddef.h>
#include <stdint.h>

int file_helpers_write_buf(
        const char* name,
        char delimiter,
        uint16_t* buffer,
        size_t size);

#endif /* FILE_HELPERS_H_ */
