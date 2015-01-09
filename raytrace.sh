#!/bin/sh
#$ -cwd            # Set the working directory for the job to the current directory
#$ -V
#$ -m be
#$ -M b.blundell@qmul.ac.uk
#$ -pe openmpi 4    # Request 4 CPU cores - can also be -pe openmpi 12 for example
#$ -l h_rt=0:10:0  # Request 24 hour runtime
#$ -l h_vmem=1G    # Request 1GB RAM / core, i.e. 4GB total
./raytracer -w 1600 -h 1400
