#!/bin/sh
#$ -cwd            # Set the working directory for the job to the current directory
#$ -V
#$ -m be
#$ -M b.blundell@qmul.ac.uk
#$ -pe openmpi 1    # Request 4 CPU cores - can also be -pe openmpi 12 for example
#$ -l h_rt=0:10:0  # Request 24 hour runtime
#$ -l h_vmem=1G    # Request 1GB RAM / core, i.e. 4GB total
#for i in `seq 0 359`;
#do
#	mpirun -np 12 ./build/raytracer -w 800 -h 600 -n $i -f frame$i.bmp
#done
mpirun -np 4 ./build/raytracer -w 320 -h 240 -n 1 -b 3 -f frame00.bmp
