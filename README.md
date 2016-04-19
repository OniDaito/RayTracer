# Rays v1.0

![Build Status Images](https://travis-ci.org/OniDaito/RayTracer.svg)

A small raytracer built for fun, learning and profiling

## Requirements

This program requires a C++ compiler and cmake. The compiler should support OpenMP. gcc/g++ version should be 4.8 or above.

## Building

Create a directory called raytracer_build at the same level as this directory. You can then run cmake thusly

    cmake ../RayTracer

If this runs without error, run make

    make

## Running

The final executable is called *rays* and can be run with no arguments

    ./rays

This produces a file called *test.bmp*.

### Command Line Options

The command line options can be found in main.cpp but in a nutshell

  - w (integer) the width of the image
  - h (integer) the height of the image
  - s (string)  a path to a scene file
  - a (integer) the number of supersamples (default=4)
  - b (integer) the maximum number of ray bounces (default=10)
  - i (float)   the ray intensity (default=1.0f)
  - r (integer) the number of rays per pixel (default=10)

## Scene file

You can pass in a scene file (a default scene.txt is available). The format is as follows:

    // Spheres
    // S x y z radius r g b shiny
    S 1.0 1.0 1.0 1.5 1.0 0.0 0.0 0.1

    // Lights
    // L r g b x y z radius
    L 0.8 0.7 0.8 0.0 12.0 0.0 2.0

    // Camera
    // eye-x eye-y eye-z look-x look-y look-z up-x up-y up-z width height field-of-view near far
    C -5.0 5.0 -5.0 0.0 0.0 0.0 0.0 1.0 0.0 320 240 90.0 0.1 100.0

    // Ground
    // G height-y r g b shiny
    G 0.0 0.312 0.785 0.123 0.1

    // Sky
    // K r g b
    K 0.0846 0.0933 0.0949

## TODO

  - Triangle intersections
  - OBJ Loader integration
  - CUDA version
  - OpenCL version
  - Remake the MPI Version
  - Profiling
  - Vectorisation 
