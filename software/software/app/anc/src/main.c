/**
 * @file    main.c
 * @brief   <Provide some short description here>
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils/logger.h"
#include "sound_card/sound_card.h"

int main(int argc, char* argv[])
{
        (void)argc;
        (void)argv;

        while (1) {
                sleep(1);
                LOG_INFO("Entry point!");
        }
        return 0;
}
