#ifndef __main_hpp__
#define __main_hpp__


// Option struct
typedef struct {
  unsigned int width;
  unsigned int height;
  unsigned int num_bounces;
  unsigned int frame;
  unsigned int num_rays_per_pixel;
  float ray_intensity;
  float near_plane;
  float far_plane;
  bool live;
  glm::mat4 perspective;
  std::string filename;
  unsigned int mpi_item_buffer;
} RaytraceOptions;


// MPI Custom Type

typedef struct {
  float r,g,b;
  int x,y;
}MPIPixel;

MPI_Datatype pixelType;


#endif
