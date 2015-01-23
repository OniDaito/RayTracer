cmake_minimum_required(VERSION 2.8)

PROJECT(raytracer)

# GLM Includes
include_directories( "include/glm" )

# Basic includes
include_directories( "include" )

ADD_EXECUTABLE(raytracer src/main.cpp src/obj_loader.cpp src/file.cpp src/simplex.cpp src/physics.cpp src/tracer.cpp)
SET(CMAKE_C_COMPILER mpicc)
SET(CMAKE_CXX_COMPILER mpicxx)

# Options 
#SET(CMAKE_CXX_FLAGS "-include gcc-preinclude.h -std=c++11 -static-libstdc++")
SET(CMAKE_CXX_FLAGS "-std=c++11 -static-libstdc++")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -std=c++11 -static-libstdc++")

# Require MPI for this project
find_package(MPI REQUIRED)
set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} ${MPI_LINK_FLAGS})
include_directories(MPI_INCLUDE_PATH)
target_link_libraries(raytracer ${MPI_LIBRARIES}) 
