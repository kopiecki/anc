set(target_name sound_card)
set(target_sources
        src/sound_card.c
)

add_library(${target_name}
    STATIC
        ${target_sources}
)

target_include_directories(${target_name}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(${target_name}
    PRIVATE
        asound
        logger
        utils
        m
)
