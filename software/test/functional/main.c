/**
 * @file    main.c
 * @brief   <Provide some short description here>
 */

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "logger/logger.h"
#include "test_filter/test_filter.h"
#include "test_signal/test_signal.h"
#include "utils/common.h"

static int signal_test(void)
{
        LOG_INFO("Running signal test");

        int error = test_signal_dump_sine();
        if (error < 0)
                return error;

        error = test_signal_dump_triangle();
        if (error < 0)
                return error;

        error = test_signal_dump_noise();
        if (error < 0)
                return error;

        error = test_signal_dump_sum();
        if (error < 0)
                return error;

        LOG_INFO("Signal test ended with success");

        return 0;
}

static int filter_test(void)
{
        LOG_INFO("Running filter test");

        LOG_INFO("Filter test ended with success");
}

static int audio_test(void)
{
        LOG_INFO("Running audio test");
}

static int (*tests[])(void) = {
        signal_test,
        filter_test,
        audio_test,
};

static const char* TEST_TYPES[] = {
        "signal",
        "filter",
        "audio",
};

int main(int argc, char* argv[])
{
        if (argc != 2)
                return -EINVAL;

        for (uint8_t i = 0; i < ARRAY_SIZE(TEST_TYPES); i++)
                if (!strcmp(TEST_TYPES[i], argv[1]))
                        return tests[i]();
}
