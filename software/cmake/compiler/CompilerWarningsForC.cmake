function (compiler_warnings_install TARGET_NAME)

    # Argument parser
    set(ARG_OPTIONS WARNINGS_AS_ERRORS)
    set(ARG_SINGLE  "")
    set(ARG_MULTI   "")
    cmake_parse_arguments(
        PARSE_ARGV 1 ARG
        "${ARG_OPTIONS}" "${ARG_SINGLE}" "${ARG_MULTI}"
    )

    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
    )

    if (MSVC)
        set(PROJECT_WARNINGS ${MSVC_WARNINGS})
    elseif (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        set(PROJECT_WARNINGS ${CLANG_WARNINGS})
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(PROJECT_WARNINGS ${GCC_WARNINGS})
    else ()
        message(AUTHOR_WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
    endif ()

    target_compile_options(${TARGET_NAME} PRIVATE ${PROJECT_WARNINGS})
    message(STATUS "Compiler warnings configured to: ${TARGET_NAME}")

endfunction ()
