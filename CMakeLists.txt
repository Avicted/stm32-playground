cmake_minimum_required(VERSION 3.22)
project(stm32_playground C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Hardware-independent code, shared by the firmware and the host tests.
add_library(corelib STATIC lib/ringbuf.c lib/cli.c)
target_include_directories(corelib PUBLIC lib)
target_compile_options(corelib PRIVATE -Wall -Wextra)

if(CMAKE_CROSSCOMPILING)
    add_subdirectory(firmware)
    add_subdirectory(apps)
else()
    enable_testing()
    add_subdirectory(tests/host)
endif()
