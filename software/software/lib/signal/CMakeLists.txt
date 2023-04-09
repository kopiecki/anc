set(target_name signal)
set(target_sources
        src/signal.c    
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
        utils
        m
)
