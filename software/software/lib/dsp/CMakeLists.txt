set(target_name dsp)
set(target_sources
        src/fir.c
        src/lms.c
        src/nlms.c
)

add_library(${target_name}
    STATIC
        ${target_sources}
)

target_include_directories(${target_name}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

include(compiler/CompilerWarningsForC)
compiler_warnings_install(${target_name} WARNINGS_AS_ERRORS)

target_link_libraries(${target_name}
    PUBLIC
        utils
)
