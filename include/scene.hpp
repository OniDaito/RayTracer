#ifndef __scene_hpp__
#define __scene_hpp__


#include "geometry.hpp"
#include "main.hpp"

// Scene - Collection of all our objects basically
struct Scene {
  std::vector< std::shared_ptr<Hittable> >  objects;     
  std::vector<LightPoint> lights;
};

Scene CreateScene(RaytraceOptions &options);

#endif

