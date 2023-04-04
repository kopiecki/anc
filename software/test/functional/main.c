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
#include "test_sound/test_sound.h"
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

        LOG_INFO("Signal test ended");

        return 0;
}

static int filter_test(void)
{
        LOG_INFO("Running filter test");

        test_filter_fir();

        LOG_INFO("Filter test ended");
}

static int sound_test(void)
{
        LOG_INFO("Running audio test");
        test_sound_poll_playback_loop();
        LOG_INFO("Sound test ended");
}

static int (*tests[])(void) = {
        signal_test,
        filter_test,
        sound_test,
};

static const char* TEST_TYPES[] = {
        "signal",
        "filter",
        "sound",
};

int main(int argc, char* argv[])
{
        if (argc != 2)
                return -EINVAL;

        for (uint8_t i = 0; i < ARRAY_SIZE(TEST_TYPES); i++)
                if (!strcmp(TEST_TYPES[i], argv[1]))
                        return tests[i]();
}
