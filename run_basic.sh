#!/bin/bash
mpirun -np 5 -verbose ./build/raytracer -w 800 -h 600 -n 1 -b 10 -p 400 -f test.bmp -x
