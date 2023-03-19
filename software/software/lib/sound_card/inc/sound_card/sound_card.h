/**
 * @file    sound_card.h
 * @brief   <Provide some short description here>
 */

#ifndef SOUND_CARD_H_
#define SOUND_CARD_H_

#include <stddef.h>
#include <stdint.h>

typedef enum SoundCard_Access_e
{
        INTERLEAVED = 0,

} SoundCard_Access_t;

typedef enum SoundCard_Stream_e
{
        PLAYBACK = 0,
        CAPTURE,

} SoundCard_Stream_t;

typedef enum SoundCard_Format_e
{
        S16 = 2,

} SoundCard_Format_t;

typedef struct SoundCard_Config_s
{
        const char* name;

        SoundCard_Stream_t stream;
        SoundCard_Format_t format;

        uint32_t rate;
        uint8_t channels;

        uint32_t buffer_time_us;
        uint32_t period_time_us;

} SoundCard_Config_t;

typedef struct SoundCard_s SoundCard_t;

SoundCard_t* const sound_card_init(const SoundCard_Config_t* const config);
void sound_card_deinit(SoundCard_t* const this);

int sound_card_capture(SoundCard_t* const this, void* buffer, size_t size);
int sound_card_playback(
        SoundCard_t* const this,
        const void* buffer,
        uint8_t num_of_periods);

#endif /* SOUND_CARD_H_ */
