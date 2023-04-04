/**
 * @file    sound_card.h
 * @brief   Sound card ADT. In fact, this is just asoundlib fascade with
 * addidional logging to make programming with ALSA easier and more
 * transparent.
 */

#ifndef SOUND_CARD_H_
#define SOUND_CARD_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SOUND_CARD_MAX_CHANNEL   2u
#define SOUND_CARD_FORMAT_UINT16 2
#define SOUND_CARD_FORMAT_FLOAT  14

#ifndef SOUND_CARD_FORMAT
#define SOUND_CARD_FORMAT SOUND_CARD_FORMAT_FLOAT
#define SAMPLE_FORMAT     float
#endif

/* Opaque sound card handle */
typedef struct sound_card_s sound_card_t;

enum sound_card_stream
{
        SOUND_CARD_STREAM_PLAYBACK = 0,
        SOUND_CARD_STREAM_CAPTURE,
};

enum sound_card_access
{
        SOUND_CARD_ACCESS_INTERLEAVED = 3,
        SOUND_CARD_ACCESS_SEPARATE    = 4,
};

struct sound_card_config
{
        const char* name;

        enum sound_card_stream stream;
        enum sound_card_access access;

        uint32_t rate;
        uint8_t channels;

        uint32_t buffer_time_us;
        uint32_t period_time_us;

        bool poll;
};

struct sound_card_task
{
        float* sample_buffers[SOUND_CARD_MAX_CHANNEL];
        size_t size;
        uint16_t num_of_periods;
};

sound_card_t* const sound_card_construct(
        const struct sound_card_config* const config);
void sound_card_destruct(sound_card_t* const this);

size_t sound_card_get_period_size_bytes(sound_card_t* const this);

int sound_card_capture(sound_card_t* const this, struct sound_card_task* const task);
int sound_card_playback(sound_card_t* const this, struct sound_card_task* const task);

int sound_card_synchronize(sound_card_t* const playback, sound_card_t* const capture);

int sound_card_poll_playback_loop(
        sound_card_t* const playback, struct sound_card_task* const task);

int sound_card_poll_is_ready(sound_card_t* const self, bool* const ready);

#endif /* SOUND_CARD_H_ */
