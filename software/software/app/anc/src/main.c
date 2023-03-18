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
            .name     = SOUND_CARD_NAME,
            .rate     = 48000,
            .channels = 2,
            .stream   = PLAYBACK,
        };

        SoundCard_t* const sound_card = sound_card_init(&config);
        sound_card_deinit(sound_card);
        return 0;
}
