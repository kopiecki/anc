/**
 * @file    file_helpers.c
 * @brief   <Provide some short description here>
 */

#include "utils/file_helpers.h"

#include <errno.h>
#include <stdio.h>

#include "utils/assert.h"
#include "utils/logger.h"

int file_helpers_write_buf(
        const char* name,
        char delimiter,
        const FILE_BUFFER_FORMAT* const buffer,
        size_t size,
        bool append)
{
        ASSERT(name != NULL);
        ASSERT(buffer != NULL);

        FILE* file = fopen(name, (append) ? "a" : "w");
        if (file == NULL)
        {
                LOG_ERROR("Cannot open %s file", name);
                return -ENOENT;
        }

        for (size_t i = 0u; i < size; i++)
                fprintf(file, "%f%c", buffer[i], delimiter);
        if (delimiter != '\n')
                fprintf(file, "%c", '\n');

        fclose(file);
}

int file_helpers_read_buf(const char* name, FILE_BUFFER_FORMAT* const buffer)
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
                sscanf(line, "%lf", &buffer[i++]);
        return i;
}
