/**
 * @file    sound_card.h
 * @brief   Sound card ADT. In fact, this is just asoundlib fascade with
 * addidional logging to make programming with ALSA easier and more transparent.
 */

#ifndef SOUND_CARD_H_
#define SOUND_CARD_H_

#include <stddef.h>
#include <stdint.h>

#define SOUND_CARD_MAX_CHANNEL 2u

/* Opaque sound card handle */
typedef struct sound_card_s sound_card_t;

struct buffer_and_size
{
        void* buffer;
        uint32_t size;
};

enum sound_card_stream
{
        PLAYBACK = 0,
        CAPTURE,
        FULL_DUPLEX,
};

enum sound_card_sample_format
{
        S16 = 2,
};

struct sound_card_config
{
        const char* name;

        enum sound_card_stream stream;
        enum sound_card_sample_format format;

        uint32_t rate;
        uint8_t channels;

        uint32_t buffer_time_us;
        uint32_t period_time_us;
};

struct sound_card_playback_task
{
        struct buffer_and_size channel_data[SOUND_CARD_MAX_CHANNEL];
        uint16_t num_of_periods;
};

sound_card_t* const sound_card_init(const struct sound_card_config* const config);
void sound_card_deinit(sound_card_t* const this);

int sound_card_capture(sound_card_t* const this, void* buffer, size_t size);
int sound_card_playback(
        sound_card_t* const this,
        const struct sound_card_playback_task* const task);

#endif /* SOUND_CARD_H_ */
