set(test_signal "test_signal")
set(test_signal_sources
        functional/test_signal.c
)

add_executable(${test_signal} ${test_signal_sources})
include_directories(${test_signal} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${test_signal}
        PRIVATE
                signal
                utils
)

include(compiler/CompilerWarningsForC)
compiler_warnings_install(${test_signal} WARNINGS_AS_ERRORS)

install(TARGETS ${test_signal} RUNTIME DESTINATION bin)

set(test_sound "test_sound")
set(test_sound_sources
        functional/test_sound.c
)

add_executable(${test_sound} ${test_sound_sources})
include_directories(${test_sound} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${test_sound}
        PRIVATE
                -static-libgcc
                -static-libstdc++
                -static
                signal
                utils
                sound_card
                dsp
)

include(compiler/CompilerWarningsForC)
compiler_warnings_install(${test_sound} WARNINGS_AS_ERRORS)

install(TARGETS ${test_sound} RUNTIME DESTINATION bin)

set(test_fir "test_fir")
set(test_fir_sources
        functional/test_fir.c
)

add_executable(${test_fir} ${test_fir_sources})
include_directories(${test_fir} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${test_fir}
        PRIVATE
                signal
                utils
                dsp
                -static-libgcc
                -static-libstdc++
)

include(compiler/CompilerWarningsForC)
compiler_warnings_install(${test_sound} WARNINGS_AS_ERRORS)

install(TARGETS ${test_fir} RUNTIME DESTINATION bin)

set(test_lms "test_lms")
set(test_lms_sources
        functional/test_lms.c
)

add_executable(${test_lms} ${test_lms_sources})
include_directories(${test_lms} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${test_lms}
        PRIVATE
                signal
                utils
                dsp
)

include(compiler/CompilerWarningsForC)
compiler_warnings_install(${test_lms} WARNINGS_AS_ERRORS)

install(TARGETS ${test_lms} RUNTIME DESTINATION bin)

set(test_anc "test_anc")
set(test_anc_sources
        functional/test_anc.c
)

add_executable(${test_anc} ${test_anc_sources})
include_directories(${test_anc} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${test_anc}
        PRIVATE
                signal
                sound_card
                utils
                dsp
)

include(compiler/CompilerWarningsForC)
compiler_warnings_install(${test_anc} WARNINGS_AS_ERRORS)

install(TARGETS ${test_anc} RUNTIME DESTINATION bin)
