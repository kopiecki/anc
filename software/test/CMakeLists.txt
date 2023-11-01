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


set(test_prng "test_prng")
set(test_prng_sources
        functional/test_prng.c
)
add_executable(${test_prng} ${test_prng_sources})
include_directories(${test_prng} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${test_prng}
        PRIVATE
                signal
                utils
                dsp
)
install(TARGETS ${test_prng} RUNTIME DESTINATION bin)
