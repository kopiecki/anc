set(target_name utils)
set(target_sources
        src/dummy.c
        src/malloc_wrapper.c
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
        logger
)
