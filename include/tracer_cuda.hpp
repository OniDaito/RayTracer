#ifndef __tracer_cuda_hpp__
#define __tracer_cuda_hpp__

#include <iostream>
#include <cuda_runtime.h>
#include "main.hpp"
#include "scene.hpp"
#include "geometry_cuda.hpp"

__global__ void RenderKernel(float3 *output);

void RaytraceKernelCUDA(RaytraceBitmap  &bitmap, const RaytraceOptions &options, const Scene &scene );

#endif
