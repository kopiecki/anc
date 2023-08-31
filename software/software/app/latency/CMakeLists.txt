set(target_name "latency")

set(target_sources
        src/latency.c
)

add_executable(${target_name} ${target_sources})

include(compiler/CompilerWarningsForC)
compiler_warnings_install(${target_name} WARNINGS_AS_ERRORS)

find_library(ALSA_LIB NAMES asound PATHS $ENV{ARM_TOOLCHAIN_PATH}/lib)

target_include_directories(${target_name}
    PRIVATE
        $ENV{ARM_TOOLCHAIN_PATH}/include
)

target_link_libraries(${target_name}
    PRIVATE
        ${ALSA_LIB}
        m
        sound_card
        dsp
        signal
        utils

)

install(TARGETS ${target_name} RUNTIME DESTINATION bin)
