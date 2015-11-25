#!/bin/bash
mpirun -np 4 -verbose inspxe-cl -r my_result -collect mi1 -- ./build/raytracer -w 800 -h 600 -n 1 -b 10 -p 400 -f test.bmp -x
