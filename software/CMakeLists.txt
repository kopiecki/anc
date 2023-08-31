cmake_minimum_required(VERSION 3.18)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(PROJECT_NAME "anc" CACHE STRING "Project name - Can be overridden by Makefile")
set(ANC_BUILD_TEST OFF)

project(${PROJECT_NAME} LANGUAGES C CXX ASM)
set(CMAKE_C_STANDARD 23)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(ANC_BUILD_TEST "Build functional tests"               ON)
set(DEBUG                                                 ON)

if (ARMHF)
    message(STATUS "Adding NEON and FPU support!")

    add_compile_options(
        -O2
        -g
        -Wall
        -std=c11
        -ffast-math
        -ftree-vectorize
        -mcpu=cortex-a8
        -mfpu=neon
        -mhard-float
    )

endif()

add_subdirectory(software)
if (ANC_BUILD_TEST)
    add_subdirectory(test)
endif()
