#ifndef __scene_hpp__
#define __scene_hpp__

#include <memory>

#include "geometry.hpp"
#include "main.hpp"

// Scene - Collection of all our objects basically
struct Scene {
  std::vector< std::shared_ptr<Hittable> >  objects;     
  std::vector< std::shared_ptr<Light> > lights;
  std::shared_ptr<Camera> camera;
};

Scene CreateScene(RaytraceOptions &options);

#endif

