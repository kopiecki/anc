# ----------------------------------------------------
# Environment
# ----------------------------------------------------

cmake_minimum_required(VERSION 3.18)
set(PROJECT_NAME "anc" CACHE STRING "Project name - Can be overridden by Makefile")

project(${PROJECT_NAME} LANGUAGES C CXX ASM)

# ----------------------------------------------------
# Project directories
# ----------------------------------------------------

add_subdirectory(software)
add_subdirectory(test)
