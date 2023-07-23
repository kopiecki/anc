set(target_name sound_card)
set(target_sources
        src/sound_card.c
)

add_library(${target_name}
    STATIC
        ${target_sources}
)

find_library(ALSA_LIB NAMES asound PATHS $ENV{ARM_TOOLCHAIN_PATH}/lib)

target_include_directories(${target_name}
    PRIVATE
        $ENV{ARM_TOOLCHAIN_PATH}/include
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(${target_name}
    PUBLIC
        ${ALSA_LIB}
        signal
        utils
        m
)
