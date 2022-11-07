# ----------------------------------------------------
# Environment
# ----------------------------------------------------

cmake_minimum_required(VERSION 3.18)
set(PROJECT_NAME "anc" CACHE STRING "Project name - Can be overridden by Makefile")

set(SYSTEM_PATH             ${CMAKE_SOURCE_DIR}/system)
set(TOOLS_PATH              ${CMAKE_SOURCE_DIR}/tools)
set(PROGRAM_PATH            ${CMAKE_SOURCE_DIR}/program)
set(TEST_PATH               ${CMAKE_SOURCE_DIR}/test)
set(CMAKE_PATH              ${CMAKE_SOURCE_DIR}/cmake)

set(PROGRAM_APP_PATH        ${PROGRAM_PATH}/application)
set(PROGRAM_EXTERNAL_PATH   ${PROGRAM_PATH}/external)
set(PROGRAM_MODULES_PATH    ${PROGRAM_PATH}/modules)

project(${PROJECT_NAME} LANGUAGES C CXX ASM)

# ----------------------------------------------------
# Project directories
# ----------------------------------------------------

add_subdirectory(${PROGRAM_PATH})
add_subdirectory(${TEST_PATH})
