/**
 * @file    logger.c
 * @brief   <Provide some short description here>
 */

#include "logger/logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static const char* logger_get_timestamp()
{
	time_t raw_time	     = 0;
	struct tm* time_info = NULL;

	time(&raw_time);
	char* time_str		       = ctime(&raw_time);
	time_str[strlen(time_str) - 1] = '\0';

	return time_str;
}

void logger_unused(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	va_end(args);
}

void logger_log(const char* format, ...)
{
	fprintf(stderr, "%s ", logger_get_timestamp());

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
