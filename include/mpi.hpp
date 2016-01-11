/**
* @brief mpi header file for our raytracer
* @file mpi.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 7/01/2015
*
*/

#ifndef __mpi_hpp__
#define __mpi_hpp__

#include <mpi.hpp>

// MPI Custom Type

typedef struct {
  float r,g,b;
  int x,y;
}MPIPixel;

MPI_Datatype pixelType;

void runServerProcess(int num_mpi_procs);
void runClientProcess(long int offset, long int range);

#endif

