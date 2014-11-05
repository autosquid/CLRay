project(CLRaytracer)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# set output place for executable and lib and static lib
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(ARCHIVE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR on)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include -I/usr/local/include")

# This is not used yet, but it will be soon ;-)
include_directories(3rd)
include_directories(3rd/clew)
include_directories(3rd/clew/include)
include_directories(3rd/OpenCLHelper)

find_package(SDL2)
find_package(OpenGL REQUIRED)

include_directories(${SDL2_INCLUDE_DIR})

add_library(clew SHARED 3rd/clew/src/clew.c )
target_link_libraries( clew dl )

add_library(OpenCLHelper SHARED 3rd/OpenCLHelper/OpenCLHelper.cpp 3rd/OpenCLHelper/CLKernel.cpp )
target_link_libraries(OpenCLHelper clew )

add_subdirectory(src)

