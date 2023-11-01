/**
 * @file    logger.c
 * @brief   Basic logger implementation
 */

#include "utils/logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void logger_unused(const char* format, ...)
{
        va_list args;
        va_start(args, format);
        va_end(args);
}

void logger_log(const char* format, ...)
{
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
}

void logger_raw(const char* format, ...)
{
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
}
