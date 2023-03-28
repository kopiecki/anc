/**
 * @file    file_helpers.c
 * @brief   <Provide some short description here>
 */

#include "utils/file_helpers.h"

#include <errno.h>
#include <stdio.h>

#include "logger/logger.h"
#include "utils/assert.h"

int file_helpers_write_buf(
        const char* name,
        char delimiter,
        const float* const buffer,
        size_t size)
{
        ASSERT(name != NULL);
        ASSERT(buffer != NULL);

        FILE* file = fopen(name, "w");
        if (file == NULL)
        {
                LOG_ERROR("Cannot open %s file", name);
                return -ENOENT;
        }

        for (size_t i = 0u; i < size; i++)
                fprintf(file, "%f%c", buffer[i], delimiter);

        fclose(file);
}

int file_helpers_read_buf(const char* name, float* const buffer)
{
        ASSERT(name != NULL);
        ASSERT(buffer != NULL);

        FILE* file = fopen(name, "r");
        if (file == NULL)
        {
                LOG_ERROR("Cannot open %s file", name);
                return -ENOENT;
        }

        char* line = NULL;
        size_t size, i = 0u;
        while ((getline(&line, &size, file)) > 0)
                sscanf(line, "%f", &buffer[i++]);

        return i;

}
