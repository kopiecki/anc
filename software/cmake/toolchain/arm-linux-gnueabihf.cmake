set (CMAKE_SYSTEM_NAME      "linux")
set (CMAKE_SYSTEM_PROCESSOR "arm")

set (prefix                 arm-linux-gnueabihf-)
set (CMAKE_C_COMPILER       ${prefix}gcc)
set (CMAKE_CXX_COMPILER     ${prefix}g++)
set (CMAKE_LINKER           ${prefix}ld)
set (CMAKE_OBJCOPY          ${prefix}objcopy)
set (CMAKE_AR               ${prefix}gcc-ar)
set (CMAKE_SIZE             ${prefix}size)

set(CMAKE_C_FLAGS "-mfpu=neon -ftree-vectorize")
set (PACKAGE_ARCHITECTURE arm)
