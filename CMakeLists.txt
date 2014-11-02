project(CLRaytracer)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)

# set output place for executable and lib and static lib
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(ARCHIVE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

# This is not used yet, but it will be soon ;-)
include_directories(3rd/clew)
include_directories(3rd/clew/include)
include_directories(3rd/OpenCLHelper)

add_library(clew SHARED 3rd/clew/src/clew.c )
target_link_libraries( clew dl )

add_library(OpenCLHelper SHARED 3rd/OpenCLHelper/OpenCLHelper.cpp
    3rd/OpenCLHelper/CLKernel.cpp )
target_link_libraries(OpenCLHelper clew )

add_subdirectory(src)

