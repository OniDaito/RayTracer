#ifndef __main_hpp__
#define __main_hpp__

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

// Easy buffer type for our final result frame
typedef std::vector< std::vector< glm::vec3 > > RaytraceBitmap;

// const terms declared for all sections

static const float EPSILON = 0.000000001;
static const float MAX_DISTANCE = 100000000.0;

// Option struct
typedef struct {
  unsigned int width;
  unsigned int height;
  unsigned int max_bounces;
  unsigned int frame;
  unsigned int num_rays_per_pixel;  // How many rays per pixel? Related to ray_intensity
  unsigned int supersample;         // How many samples per pixel
  float ray_intensity;              // Usually set to 1.0f. Similar to Gamma. Correct for 10 rays per pixel
  bool live;
  std::string output_filename;
  std::string scene_filename;
} RaytraceOptions;

#endif
