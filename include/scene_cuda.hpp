/**
* @brief Test raytracer for Apocrita
* @file scene_cuda.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 18/08/2016
*
*/

#ifndef __scene_cuda_hpp__
#define __scene_cuda_hpp__

#include <memory>

#include "geometry_cuda.hpp"
#include "camera.hpp"
#include "main.hpp"

// Scene - Collection of all our objects basically
// http://stackoverflow.com/questions/7477310/why-cant-i-create-a-vector-of-lambda-in-c11

struct Scene {
  std::vector< Sphere* >  spheres;  
  std::vector< Light* > lights;
  std::vector< std::function<bool(const Ray &ray, RayHit &hit, Material *m)> > intersection_funcs;
  Ground* ground;
  Camera* camera;
  float3 sky_colour;
};

Scene CreateScene(RaytraceOptions &options);

#endif


