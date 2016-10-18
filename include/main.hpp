#ifndef __main_hpp__
#define __main_hpp__

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

// Cheap return type
struct BitmapRGB {
  unsigned int r;
  unsigned int g;
  unsigned int b;
};

// Easy buffer type for our final result frame
struct RaytraceBitmap {
  
  RaytraceBitmap(unsigned int w, unsigned int h) {
    width = w;
    height = h;

    for (int i =0; i < width * height * 3; ++i){
      data.push_back(0);
    }

  }

  void SetRGB(unsigned int x, unsigned int y, float r, float g, float b) {
    int p = (y * width + x) * 3;
    data[p] = static_cast<unsigned int>(floor(r * 255.0));
    data[p+1] = static_cast<unsigned int>(floor(g * 255.0));
    data[p+2] = static_cast<unsigned int>(floor(b * 255.0));
  };

  BitmapRGB GetRGB(int x, int y) { 
    BitmapRGB colour;
    int p = (y * width + x) * 3;
    colour.r = data[p];
    colour.g = data[p+1];
    colour.b = data[p+2];
    return colour;
  };

  std::vector<unsigned int> data;
  unsigned int width;
  unsigned int height;
};

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
