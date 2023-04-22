set(target_name utils)
set(target_sources
        src/file_helpers.c
        src/logger.c
        src/circ_fifo.c
)

add_library(${target_name}
    STATIC
        ${target_sources}
)

include(compiler/CompilerWarningsForC)
compiler_warnings_install(${target_name} WARNINGS_AS_ERRORS)

target_include_directories(${target_name}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
