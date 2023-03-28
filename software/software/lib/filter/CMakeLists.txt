set(target_name filter)
set(target_sources
        src/fir.c
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
    PUBLIC
        utils
        logger
)
