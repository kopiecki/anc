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
install(TARGETS ${test_signal} RUNTIME DESTINATION bin)

set(test_sound "test_sound")
set(test_sound_sources
        functional/test_sound.c
)

add_executable(${test_sound} ${test_sound_sources})
include_directories(${test_sound} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${test_sound}
        PRIVATE
                signal
                utils
                sound_card
)
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
)

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

install(TARGETS ${test_lms} RUNTIME DESTINATION bin)
