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
