/**
 * @file    main.c
 * @brief   <Provide some short description here>
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dsp/fir.h"
#include "dsp/lms.h"
#include "signal/signal.h"
#include "sound_card/sound_card.h"
#include "utils/circ_fifo.h"
#include "utils/file_helpers.h"
#include "utils/logger.h"

#define F_HZ         8000u
#define NUM_CHANNELS 1u

#define P_DEV "hw:0,0"
#define C_DEV "hw:0,0"

#define PERIOD_US 8500
#define BUFF_US   2 * PERIOD_US
#define S_SIZE    sizeof(uint32_t)

enum playback_buffers {
        NOISE_BUFF     = 0,
        ANTINOISE_BUFF = 1,
};

enum capture_buffers {
        ERROR_BUFF = 0,
        REF_BUFF   = 1,
};

static sound_card_t *p_dev, *c_dev;
static uint32_t *p_buff, *c_buff, *ref_buff, *error_buff;
static size_t period_size;

#define FIFO_SIZE 8192u
static struct circ_fifo x, fx;

#define PRIMARY_ORDER 512u
static float primary_w[PRIMARY_ORDER];

#define SECONDARY_ORDER 256u
static float secondary_w[SECONDARY_ORDER];

static struct signal white_noise = {
        .amplitude          = UINT16_MAX,
        .frequency          = 0.0,
        .sampling_frequency = F_HZ,
        .type               = SIGNAL_TYPE_NOISE,
        .phase              = 0,
};
static struct signal sine_150_hz_noise = {
        .amplitude          = UINT16_MAX,
        .frequency          = 150.0,
        .sampling_frequency = F_HZ,
        .type               = SIGNAL_TYPE_SINE,
        .phase              = 0,
};

static int create_capture_dev(void)
{
        const struct sound_card_config capture_device_cfg = {
                .access         = SOUND_CARD_ACCESS_INTERLEAVED,
                .channels       = NUM_CHANNELS,
                .format         = SOUND_CARD_FORMAT_U24_LE,
                .name           = C_DEV,
                .rate           = F_HZ,
                .stream         = SOUND_CARD_STREAM_CAPTURE,
                .buffer_time_us = BUFF_US,
                .period_time_us = PERIOD_US,
                .poll           = false,
                .blocking       = false,
        };

        c_dev = sound_card_construct(&capture_device_cfg);
        if (!c_dev)
                return -ENOTSUP;

        return 0;
}

static int create_playback_dev(void)
{
        const struct sound_card_config playback_device_cfg = {
                .access         = SOUND_CARD_ACCESS_INTERLEAVED,
                .channels       = NUM_CHANNELS,
                .format         = SOUND_CARD_FORMAT_U24_LE,
                .name           = P_DEV,
                .rate           = F_HZ,
                .stream         = SOUND_CARD_STREAM_PLAYBACK,
                .buffer_time_us = BUFF_US,
                .period_time_us = PERIOD_US,
                .poll           = false,
                .blocking       = false,
        };

        p_dev = sound_card_construct(&playback_device_cfg);
        if (!p_dev)
                return -ENOTSUP;

        return 0;
}

static int create_devs(void)
{
        int error = create_playback_dev();
        if (error < 0)
                return error;

        error = create_capture_dev();
        if (error < 0)
                return error;

        return 0;
}

static int create_buffers(void)
{
        period_size = sound_card_get_period_size_bytes(p_dev) / S_SIZE;

        p_buff = calloc(period_size * NUM_CHANNELS, S_SIZE);
        if (!p_buff)
                return -ENOMEM;

        c_buff = calloc(period_size * NUM_CHANNELS, S_SIZE);
        if (!c_buff)
                return -ENOMEM;

        ref_buff = calloc(F_HZ * 8, S_SIZE);
        if (!ref_buff)
                return -ENOMEM;

        error_buff = calloc(F_HZ * 8, S_SIZE);
        if (!error_buff)
                return -ENOMEM;

        int error = circ_fifo_construct(&x, NULL, FIFO_SIZE);
        if (error < 0)
                return error;

        error = circ_fifo_construct(&fx, NULL, FIFO_SIZE);
        if (error < 0)
                return error;

        LOG_INFO("Created buffers");

        return 0;
}

static void encode_buffer()
{
        for (size_t i = 0u; i < period_size; i += NUM_CHANNELS) {
                const uint16_t noise_sample =
                        signal_generate(&white_noise, NULL, 0);

                p_buff[i + NOISE_BUFF] = noise_sample;
        }
}

static void decode_buffer()
{
        for (size_t i = 0u; i < period_size; i += NUM_CHANNELS) {
                error_buff[i] = c_buff[i];
        }
}

static int loop(void)
{
        int error = 0;

        size_t i = 0;
        while (i < (F_HZ * 4)) {

                for (size_t j = 0u; j < period_size; j++) {
                        const uint16_t noise_sample =
                                signal_generate(&white_noise, NULL, 0);

                        p_buff[j] = noise_sample;
                        ref_buff[j + i] = noise_sample;
                }

                error = sound_card_playback(p_dev, p_buff, period_size);
                if (error < 0)
                        return error;

                error = sound_card_capture(c_dev, c_buff, period_size);
                if (error < 0)
                        return error;

                size_t k = 0;
                for (size_t j = 0; j < period_size; j++) {
                        uint32_t noise_sample = c_buff[j] << 4u;
                        noise_sample          = (noise_sample >> 8u);

                        error_buff[j + i] = noise_sample; 
                }

                LOG_INFO("Captured full buffer, i = %d", i);
                i += period_size;
        }

        file_helpers_write_buf("error.txt", '\n', error_buff, F_HZ * 4, false);
        file_helpers_write_buf("noise.txt", '\n', ref_buff, F_HZ * 4, false);

}

int main(int argc, char* argv[])
{
        (void)argc;
        (void)argv;

        int error = create_devs();
        if (error < 0)
                return error;

        error = create_buffers();
        if (error < 0)
                return error;

        error = loop();
        if (error < 0)
                return error;

        return 0;
}
