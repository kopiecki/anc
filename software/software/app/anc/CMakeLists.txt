set(target_name "anc")

set(target_sources
        src/anc.c
        src/main.c
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)

add_executable(${target_name} ${target_sources})
include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

include(compiler/CompilerWarningsForC)
compiler_warnings_install(${target_name} WARNINGS_AS_ERRORS)

target_link_libraries(${target_name}
    PRIVATE
        ${CMAKE_THREAD_LIBS_INIT}
        sound_card
        dsp
        signal
        utils

)

install(TARGETS ${target_name} RUNTIME DESTINATION bin)
