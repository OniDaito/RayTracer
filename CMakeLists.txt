cmake_minimum_required(VERSION 2.8)

PROJECT(raytracer)

# GLM Includes
include_directories( "include/glm" )

# Basic includes
include_directories( "include" )

ADD_EXECUTABLE(raytracer src/main.cpp src/obj_loader.cpp src/file.cpp src/simplex.cpp)
SET(CMAKE_C_COMPILER mpicc)
SET(CMAKE_CXX_COMPILER mpicxx)

# Options 
# -static-libstdc++ is here to make sure it works with Apocrita
# Also we cant use c++11 for std due to gcc4.6.6
#SET(CMAKE_CXX_FLAGS "-include gcc-preinclude.h -std=c++11 -static-libstdc++")
SET(CMAKE_CXX_FLAGS "-std=c++0x -static-libstdc++")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -std=c++0x -static-libstdc++")

# Require MPI for this project
find_package(MPI REQUIRED)
set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} ${MPI_LINK_FLAGS})
include_directories(MPI_INCLUDE_PATH)
target_link_libraries(raytracer ${MPI_LIBRARIES}) 