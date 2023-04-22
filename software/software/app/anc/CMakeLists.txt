set(target_name "anc")

set(target_sources
        src/main.c
)

add_executable(${target_name} ${target_sources})
include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${target_name}
        PRIVATE
                sound_card
                dsp
                signal
                utils
)

include(compiler/CompilerWarningsForC)
compiler_warnings_install(${target_name} WARNINGS_AS_ERRORS)

install(TARGETS ${target_name} RUNTIME DESTINATION bin)
