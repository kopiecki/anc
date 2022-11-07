# ----------------------------------------------------
# Environment
# ----------------------------------------------------

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# ----------------------------------------------------
# Sources
# ----------------------------------------------------

set(APP_SOURCE_FILES
    ${PROGRAM_APP_PATH}/main.c
)
set(MODULES_SOURCE_FILES "")
set(EXTERNAL_SOURCE_FILES "")

add_executable(
    ${PROJECT_NAME}

    ${APP_SOURCE_FILES}
    ${MODULES_SOURCE_FILES}
    ${EXTERNAL_SOURCE_FILES}
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        SUFFIX .bin
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS YES
)

set(INCLUDES_PATH
    ${PROGRAM_APP_PATH}
    ${PROGRAM_EXTERNAL_PATH}
    ${PROGRAM_MODULES_PATH}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${INCLUDES_PATH}
)

# ----------------------------------------------------
# Compiler
# ----------------------------------------------------

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    target_compile_definitions(
        ${PROJECT_NAME}
        PRIVATE
            DEBUG   
    )

    set(OPTIMIZATION_FLAGS -Og -g3)

else()
    target_compile_definitions(
        ${PROJECT_NAME}
        PRIVATE
            __FORCE_DEBUG_SEVERITY=3   
    )

    set(OPTIMIZATION_FLAGS -O2 -g0 -flto)

endif()

toolchain_target_setup(
    ${PROJECT_NAME}
    OPTIMIZATION_FLAGS "${OPTIMIZATION_FLAGS}"
)

# ----------------------------------------------------
# Warning as errors
# ----------------------------------------------------

include(${CMAKE_PATH}/compiler/CompilerWarningsForC.cmake)
compiler_warnings_install(${PROJECT_NAME} WARNINGS_AS_ERRORS)
