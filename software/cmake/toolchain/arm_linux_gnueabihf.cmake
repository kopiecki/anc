set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(COMPILER_PATH       /toolchain/gcc-arm-8.2-2019.01-x86_64-arm-linux-gnueabihf)
set(CMAKE_C_COMPILER    ${COMPILER_PATH}/bin/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER  ${COMPILER_PATH}/bin/arm-linux-gnueabihf-g++)
set(CMAKE_SIZE          ${COMPILER_PATH}/bin/arm-linux-gnueabihf-size)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

macro(toolchain_target_setup TARGET_NAME)
    set(ARG_OPTIONS "")
    set(ARG_SINGLE  LINKER_SCRIPT)
    set(ARG_MULTI   OPTIMIZATION_FLAGS)
    cmake_parse_arguments(
        ARG
        "${ARG_OPTIONS}" "${ARG_SINGLE}" "${ARG_MULTI}"
        ${ARGN}
    )

    target_compile_options(
        ${TARGET_NAME}
        PRIVATE
            -fdiagnostics-color=always
            -ffunction-sections
            -fdata-sections
            -fstack-usage
            -mthumb
    )

    target_link_options(
        ${TARGET_NAME}
        PRIVATE
            -Wl,-check-sections,-fatal-warnings
            -Wl,--gc-sections
            -static
    )

    if (ARG_OPTIMIZATION_FLAGS)
    target_compile_options(
        ${TARGET_NAME}
        PRIVATE
            ${ARG_OPTIMIZATION_FLAGS}
    )

    target_link_options(
        ${TARGET_NAME}
        PRIVATE
            ${ARG_OPTIMIZATION_FLAGS}
    )

    endif ()
    
    if (ARG_LINKER_SCRIPT)
    target_link_options(
        ${TARGET_NAME}
        PRIVATE
            -T${ARG_LINKER_SCRIPT}
    )

    endif ()

endmacro()
