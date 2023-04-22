cmake_minimum_required(VERSION 3.18)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(PROJECT_NAME "anc" CACHE STRING "Project name - Can be overridden by Makefile")

project(${PROJECT_NAME} LANGUAGES C CXX ASM)

option (ANC_BUILD_TEST "Build functional tests"               ON)

add_subdirectory(software)
if (ANC_BUILD_TEST)
    add_subdirectory(test)
endif()
