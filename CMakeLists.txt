cmake_minimum_required(VERSION 3.5.1)
project(cyclopsASCIILink C)

option(BUILD_SHARED_MEM_VERSION "Build the Shared Memory Version of the Program" OFF)
if(BUILD_SHARED_MEM_VERSION)
        message(STATUS "Building the Shared Memory Version of cyclopsASCIILink")
else()
        message(STATUS "Building the POSIX Pipe Version of cyclopsASCIILink")
endif()

#Using technique from Marcus D. Hanwell on https://blog.kitware.com/cmake-and-the-default-build-type/ to set default build type
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
            STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

find_package(Threads)

# librt is only required if shared memory version 
if(BUILD_SHARED_MEM_VERSION)
        find_library(LIBRT rt)
        if(NOT LIBRT)
        message(FATAL_ERROR "librt not found")
        endif()
endif()

set(CMAKE_C_STANDARD 11)

if(BUILD_SHARED_MEM_VERSION)
        add_definitions(-DCYCLOPS_ASCII_SHARED_MEM)
endif()

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

set(LIBS_TO_LINK ${CMAKE_THREAD_LIBS_INIT})

if(BUILD_SHARED_MEM_VERSION)
        set(COMMON_SRCS
                ${COMMON_SRCS}
                src/depends/BerkeleySharedMemoryFIFO.c
                src/depends/BerkeleySharedMemoryFIFO.h)

        set(LIBS_TO_LINK ${LIBS_TO_LINK} ${LIBRT})
endif()

add_executable(cyclopsASCIILink src/main.c ${COMMON_SRCS})
target_link_libraries(cyclopsASCIILink ${LIBS_TO_LINK})