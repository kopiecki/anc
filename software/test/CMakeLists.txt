set(target_name "functional")

set(target_sources
        functional/test_signal/test_signal.c
        functional/test_filter/test_filter.c
        functional/test_sound/test_sound.c
        functional/main.c
)

add_executable(${target_name} ${target_sources})
include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${target_name}
        PRIVATE
                sound_card
                filter
                signal
                logger
                utils
)

install(TARGETS ${target_name} RUNTIME DESTINATION bin)
