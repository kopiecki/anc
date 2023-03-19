/**
 * @file    main.c
 * @brief   <Provide some short description here>
 */

#define __FORCE_DEBUG_SEVERITY LOGGER_DEBUG_SEVERITY_TRACE
#include "logger/logger.h"
#include "sound_card/sound_card.h"

#define SOUND_CARD_NAME "plughw:1,0"

int main(int argc, char* argv[])
{
        SoundCard_Config_t config = {
                .name           = SOUND_CARD_NAME,
                .format         = S16,
                .rate           = 44100,
                .channels       = 2,
                .stream         = PLAYBACK,
                .buffer_time_us = 1000000,
                .period_time_us = 500000,
        };

        SoundCard_t* const sound_card = sound_card_init(&config);
        if (sound_card != NULL)
        {
                sound_card_playback(sound_card, NULL, 1);
                sound_card_deinit(sound_card);
        }

        return 0;
}
