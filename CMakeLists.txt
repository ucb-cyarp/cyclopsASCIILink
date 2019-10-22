cmake_minimum_required(VERSION 3.5.1)
project(cyclopsASCIILink C)

find_package(Threads)

set(CMAKE_C_STANDARD 11)

add_compile_options("$<$<CONFIG:RELEASE>:-O3>")

set(COMMON_SRCS
        src/cyclopsFraming.c
        src/cyclopsFraming.h
        src/helpers.c
        src/helpers.h
        src/feedbackDefines.h
        src/vitisStructure.h
        src/demoText.c
        src/demoText.h
        src/mainThread.c
        src/mainThread.h)

add_executable(cyclopsASCIILink src/main.c ${COMMON_SRCS})
target_link_libraries(cyclopsASCIILink ${CMAKE_THREAD_LIBS_INIT})