#ifndef __main_hpp__
#define __main_hpp__

// Easy buffer type for our final result frame
typedef std::vector< std::vector< glm::vec3 > > RaytraceBitmap;

// Option struct
typedef struct {
  unsigned int width;
  unsigned int height;
  unsigned int num_bounces;
  unsigned int frame;
  unsigned int num_rays_per_pixel;
  float ray_intensity;
  bool live;
  std::string output_filename;
  std::string scene_filename;
} RaytraceOptions;

#endif
