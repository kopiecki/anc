/**
 * @file    file_helpers.c
 * @brief   <Provide some short description here>
 */

#include "utils/file_helpers.h"

#include <errno.h>
#include <stdio.h>

#include "logger/logger.h"

int file_helpers_write_buf(const char* name, char delimiter, uint16_t* buffer, size_t size)
{
        FILE* file = fopen(name, "w");
        if (file == NULL)
        {
                LOG_ERROR("Cannot open %s file", name);
                return -ENOENT;
        }

        for (size_t i = 0u; i < size; i++)
                fprintf(file, "%d%c", buffer[i], delimiter);

        fclose(file);
}
