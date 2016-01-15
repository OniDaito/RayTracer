/**
* @brief Test raytracer for Apocrita
* @file tracer.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 22/01/2015
*
*/

#ifndef __tracer_hpp__
#define __tracer_hpp__

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include "scene.hpp"
#include "geometry.hpp"

// Our Kernel, given a buffer and the options, creates the scene. 
void RaytraceKernel(RaytraceBitmap &bitmap, const RaytraceOptions &options, const Scene &scene);

#endif
