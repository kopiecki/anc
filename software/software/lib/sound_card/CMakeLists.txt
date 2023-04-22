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

include(compiler/CompilerWarningsForC)
compiler_warnings_install(${target_name} WARNINGS_AS_ERRORS)

add_definitions(-DALSA_PCM_NEW_HW_PARAMS_API)
target_link_libraries(${target_name}
    PRIVATE
        asound
        signal
        utils
        m
)
