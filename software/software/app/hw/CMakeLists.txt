set(target_name "hw")

set(target_sources
        src/hw.c
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)

add_executable(${target_name} ${target_sources})

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
