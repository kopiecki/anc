/**
 * @file    file_helpers.c
 * @brief   <Provide some short description here>
 */

#include "utils/file_helpers.h"

#include <errno.h>
#include <stdio.h>

int file_helpers_write_buf(
        const char* name,
        char delimiter,
        const float* const buffer,
        size_t size,
        bool append)
{
        FILE* file = fopen(name, (append) ? "a" : "w");
        if (file == NULL) {
                return -ENOENT;
        }

        for (size_t i = 0u; i < size; i++)
                fprintf(file, "%f%c", buffer[i], delimiter);
        if (delimiter != '\n')
                fprintf(file, "%c", '\n');

        return fclose(file);
}

int file_helpers_read_buf(const char* name, float* const buffer)
{
        FILE* file = fopen(name, "r");
        if (file == NULL)
                return -ENOENT;

        char* line = NULL;
        size_t size, i = 0u;
        while ((getline(&line, &size, file)) > 0)
                sscanf(line, "%f", &buffer[i++]);

        return (int)i;
}
