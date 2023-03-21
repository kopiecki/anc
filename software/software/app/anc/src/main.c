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
        const struct sound_card_config config = {
                .name           = SOUND_CARD_NAME,
                .format         = S16,
                .rate           = 44100,
                .channels       = 2,
                .stream         = PLAYBACK,
                .buffer_time_us = 1000000,
                .period_time_us = 500000,
        };

        const struct sound_card_playback_task task = {
                .channel_data   = NULL,
                .num_of_periods = 2,
        };

        sound_card_t* const sound_card = sound_card_init(&config);
        if (sound_card != NULL)
        {
                sound_card_playback(sound_card, &task);
                sound_card_deinit(sound_card);
        }

        return 0;
}
