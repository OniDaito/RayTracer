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

#include "geometry.hpp"

// Quite a lot of parameters on the stack here that 
// dont change that much. We should change that

glm::vec3 fireRays(int x, int y, const int &w, const int &h, 
  const glm::mat4 &perspective,
  const float near_plane, const float far_plane,
  const Scene &scene, const int max_bounces,
  const int num_rays_per_pixel );

#endif