/**
 * @file    test_anc.c
 * @brief   <Provide some short description here>
 */

#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "dsp/fir.h"
#include "dsp/lms.h"
#include "signal/signal.h"
#include "sound_card/sound_card.h"
#include "utils/circ_fifo.h"
#include "utils/common.h"
#include "utils/file_helpers.h"
#include "utils/logger.h"

#define I_CNT    100000u
#define LMS_STEP 0.001f
#define DEC_FACTOR

#define CAPTURE_FILE_LOG_INPUT "capture_input.csv"
#define CAPTURE_FILE_LOG_ERROR "capture_error.csv"

#define SC_PLAYBACK_NAME   "plughw:1,0"
#define SC_CAPTURE_NAME    "plughw:1,0"
#define SAMPLING_FREQUENCY 441000
#define NUM_OF_SAMPLES     SAMPLING_FREQUENCY

#define BUFFER_TIME_US 100000
#define PERIOD_TIME_US 50000
#define CHANNELS       2

static sound_card_t* sc_playback;
static sound_card_t* sc_capture;

static struct sound_card_task playback_task;
static struct sound_card_task capture_task;

static float* noise_buffer;
static float* antinoise_buffer;

static float* input_buffer;
static float* error_buffer;

#define FIFO_SIZE 8192u
static struct circ_fifo x;
static struct circ_fifo fx;

#define PRIMARY_ORDER 512u
static float primary_w[PRIMARY_ORDER];

#define SECONDARY_ORDER 256u
static float secondary_w[SECONDARY_ORDER];

size_t period_size;

enum playback_buffers {
        NOISE_BUFFER     = 0,
        ANTINOISE_BUFFER = 1,
};

enum capture_buffers {
        ERROR_BUFFER = 0,
        INPUT_BUFFER = 1,
};

static int create_cards(void)
{
        const struct sound_card_config sc_playback_config = {
                .access         = SOUND_CARD_ACCESS_SEPARATE,
                .channels       = CHANNELS,
                .name           = SC_PLAYBACK_NAME,
                .rate           = SAMPLING_FREQUENCY,
                .stream         = SOUND_CARD_STREAM_PLAYBACK,
                .buffer_time_us = BUFFER_TIME_US,
                .period_time_us = PERIOD_TIME_US,
                .poll           = false,
                .blocking       = false,
        };

        const struct sound_card_config sc_capture_config = {
                .access         = SOUND_CARD_ACCESS_SEPARATE,
                .channels       = CHANNELS,
                .name           = SC_CAPTURE_NAME,
                .rate           = SAMPLING_FREQUENCY,
                .stream         = SOUND_CARD_STREAM_CAPTURE,
                .buffer_time_us = BUFFER_TIME_US,
                .period_time_us = PERIOD_TIME_US,
                .poll           = false,
                .blocking       = false,
        };

        sc_playback = sound_card_construct(&sc_playback_config);
        if (sc_playback == NULL)
                return -ENOTSUP;

        sc_capture = sound_card_construct(&sc_capture_config);
        if (sc_capture == NULL)
                return -ENOTSUP;

        sound_card_synchronize(sc_capture, sc_playback);

        LOG_INFO("Created sound cards");

        return 0;
}

static int create_buffers(void)
{
        const size_t period_size_bytes_playback =
                (size_t)sound_card_get_period_size_bytes(sc_playback);
        const size_t period_size_bytes_capture =
                (size_t)sound_card_get_period_size_bytes(sc_capture);

        period_size = period_size_bytes_playback / (sizeof(float) * CHANNELS);

        noise_buffer = calloc(
                period_size_bytes_playback / sizeof(float), sizeof(float));
        if (noise_buffer == NULL)
                return -ENOMEM;
        antinoise_buffer = calloc(
                period_size_bytes_playback / sizeof(float), sizeof(float));
        if (antinoise_buffer == NULL)
                return -ENOMEM;

        input_buffer =
                calloc(period_size_bytes_capture / sizeof(float), sizeof(float));
        if (input_buffer == NULL)
                return -ENOMEM;
        error_buffer =
                calloc(period_size_bytes_capture / sizeof(float), sizeof(float));
        if (error_buffer == NULL)
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

static void create_playback_task(void)
{
        playback_task.sample_buffers[NOISE_BUFFER]     = noise_buffer;
        playback_task.sample_buffers[ANTINOISE_BUFFER] = antinoise_buffer;
        playback_task.size                             = period_size;

        LOG_INFO("Created playback task");
}

static void create_capture_task(void)
{
        capture_task.sample_buffers[INPUT_BUFFER] = input_buffer;
        capture_task.sample_buffers[ERROR_BUFFER] = error_buffer;
        capture_task.size                         = period_size;

        LOG_INFO("Created capture task");
}

static void cleanup(void)
{
        if (sc_capture != NULL)
                sound_card_destruct(sc_capture);
        if (sc_playback != NULL)
                sound_card_destruct(sc_playback);
        if (input_buffer != NULL)
                free(input_buffer);
        if (error_buffer != NULL)
                free(error_buffer);
        if (noise_buffer != NULL)
                free(noise_buffer);
        if (antinoise_buffer != NULL)
                free(antinoise_buffer);
}

static int fill_buffer(void)
{
        int error         = 0;
        uint8_t threshold = 2;
        while (threshold--) {
                error = sound_card_playback(sc_playback, &capture_task);
                if (error < 0)
                        return error;
        }

        return error;
}

static void fill_x_fifo(void)
{
        for (size_t i = 0; i < period_size; i++)
                circ_fifo_write(&x, input_buffer[i]);
}

static void fill_anitnoise(void)
{
        for (size_t i = 0; i < period_size; i++)
                antinoise_buffer[period_size - i - 1] =
                        dsp_fir(&x, primary_w, PRIMARY_ORDER, i);
}

static void filter_x(void)
{
        for (size_t i = 0; i < period_size; i++) {
                float _fx = dsp_fir(&x, secondary_w, SECONDARY_ORDER, i);
                circ_fifo_write(&fx, _fx);
        }
}

static void update_primary(void)
{
        for (size_t i = 0; i < period_size; i++) {
                dsp_lms(primary_w, error_buffer[i], LMS_STEP, &fx, PRIMARY_ORDER, i);
        }
}

static void fxlms(void)
{
        fill_x_fifo();
        fill_anitnoise();
        filter_x();
        update_primary();
}

static int loop(size_t i_cnt)
{
        int error = 0;
        while (i_cnt--) {
                error = sound_card_capture(sc_capture, &capture_task);
                if (error < 0)
                        return error;

                fxlms();

                error = sound_card_playback(sc_playback, &capture_task);
                if (error < 0)
                        return error;
        }

        return error;
}

static int loopback(void)
{
        int error = create_cards();
        if (error < 0)
                return error;

        error = create_buffers();
        if (error < 0)
                return error;

        create_playback_task();
        create_capture_task();

        fill_buffer();
        loop(I_CNT);

        return 0;
}

int main(void)
{
        LOG_INFO("Running sound test");
        int error = loopback();
        if (error < 0)
                LOG_ERROR("Sound test failed");
        else
                LOG_INFO("Sound test ended with success");

        cleanup();

        return error;
}
