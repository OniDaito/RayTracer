#ifndef __scene_hpp__
#define __scene_hpp__

#include <memory>

#include "geometry.hpp"
#include "main.hpp"

// Scene - Collection of all our objects basically
// http://stackoverflow.com/questions/7477310/why-cant-i-create-a-vector-of-lambda-in-c11

struct Scene {
  std::vector< std::shared_ptr<Sphere> >  spheres;  
  std::vector< std::shared_ptr<Light> > lights;
  std::vector< std::function<bool(const Ray &ray, RayHit &hit, std::shared_ptr<Material> &m)> > intersection_funcs;
  std::shared_ptr<Ground> ground;
  std::shared_ptr<Camera> camera;
  glm::vec3 sky_colour;
};

Scene CreateScene(RaytraceOptions &options);

#endif

