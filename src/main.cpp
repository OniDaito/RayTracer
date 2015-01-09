/**
* @brief Test raytracer for Apocrita
* @file main.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 7/01/2015
*
*/
__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <limits.h>
#include <stdlib.h> 
#include <getopt.h>
#include <string_utils.hpp>

#include <random>

#include <simplex.hpp>

#include <mpi.h>

using namespace std;
using namespace s9;

#define EPSILON 1e-6

// Option struct
typedef struct {
  unsigned int width;
  unsigned int height;
  unsigned int num_bounces;
  unsigned int frame;
  unsigned int num_rays_per_pixel;
  float ray_intensity;
  float near_plane;
  glm::mat4 perspective;
  std::string filename;
} RaytraceOptions;


// Basic Ray
struct Ray {

  Ray(glm::vec3 ogn, glm::vec3 dir) : origin(ogn), direction(dir){ colour = glm::vec3(0,0,0); }
  Ray() : origin ( glm::vec3(0,0,0)), direction( glm::vec3(0,0,1)) { colour = glm::vec3(0,0,0);}

  glm::vec3 origin;         //Ray origin (can possibly bin this)
  glm::vec3 direction;      //Ray direction
  glm::vec3 colour;

};

// Represents a hit on geometry
typedef struct {
  float dist;
  glm::vec3 loc;
  glm::vec3 normal;
  glm::vec3 colour;
}RayHit;


// Triangle with stored normal
typedef struct {
  glm::vec3 v0,v1,v2;
  glm::vec3 normal;
} Triangle;

// Sphere

struct Sphere {
  Sphere (glm::vec3 c, float r) : centre(c), radius(r) {}
  glm::vec3 centre;
  float radius;
};

// Naughty Globals ><

std::vector<Sphere> spheres;
RaytraceOptions options;
MPI_Status stat;


// Test against a triangle

int triangle_intersection(const Triangle &triangle, const Ray &ray, float &distance ) {
  glm::vec3 e1, e2;  //Edge1, Edge2
  glm::vec3 p, q, t;
  float det, inv_det, u, v;
  float b;
 
  //Find vectors for two edges sharing V1
  e1 = triangle.v1 - triangle.v0;
  e2 = triangle.v2 - triangle.v0;

  //Begin calculating determinant - also used to calculate u parameter
  p = glm::cross(ray.direction, e2);

  //if determinant is near zero, ray lies in plane of triangle
  det = glm::dot(e1, p);

  //NOT CULLING
  if(det > -EPSILON && det < EPSILON) return 0;
  inv_det = 1.f / det;
 
  //calculate distance from V0 to ray origin
  t = ray.origin - triangle.v0;
 
  //Calculate u parameter and test bound
  u = glm::dot(t, p) * inv_det;

  //The intersection lies outside of the triangle
  if(u < 0.f || u > 1.f) return 0;
 
  //Prepare to test v parameter
  q = glm::cross(t,e1);
 
  //Calculate V parameter and test bound
  v = glm::dot(ray.direction, q) * inv_det;
  
  //The intersection lies outside of the triangle
  if(v < 0.f || u + v  > 1.f) return 0;
 
  b = glm::dot(e2, q) * inv_det;
 
  if(b > EPSILON) { //ray intersection
    distance = b;
    return 1;
  }
 
  // No hit, no win
  return 0;
}

// Test against a sphere 5 unit away, of radius 1

bool testSphere (Ray ray, Sphere sphere, RayHit &hit) {

  float l = glm::dot(ray.direction, (ray.origin - sphere.centre));
  float p = pow(l, 2) - pow(glm::distance(ray.origin, sphere.centre),2) + pow(sphere.radius,2);

  if (p < 0){
    return false;
  }

  p = sqrt(p);

  float dist0 = -l - p;
  float dist1 = -l + p;

  if (abs(dist0 - dist1) < EPSILON){
    return false;
  }

  // TODO - This is a bit messy
  if (dist0 > 0){
    if (dist1 > 0 && dist1 > dist0){
      hit.dist = dist0;
    } else {
      hit.dist = dist1;
    }

  } else if (dist1 > 1){
    hit.dist = dist1;
  } else {
    // Both negative - quit out
    return false;
  }
  
  /*if (dist0 < dist1)
    hit.dist = dist0;
  else
    hit.dist = dist1;
*/

  hit.loc = ray.direction * hit.dist + ray.origin;

  hit.normal = glm::normalize( hit.loc - sphere.centre);
  hit.colour = glm::vec3(0.0,0.0,1.0);

  return true;

}

// Test if the ray hits the ground
// Since we are operating in eye space we make the ground -1 instead of 0

bool testGround (Ray ray, RayHit &hit) {

  if ( ray.direction.y > 0)
    return false;

  hit.dist = abs ((ray.origin.y - 1 ) / ray.direction.y);
  
  hit.normal.x = 0;
  hit.normal.y = 1;
  hit.normal.z = 0;

  hit.loc = ray.direction * hit.dist + ray.origin;

  hit.colour = glm::vec3(0,1,0);

  return true;

}

// Fire a ray from the origin through our vritual screen

Ray fireRay(int x, int y) {

  glm::vec3 origin (0,0.0,0);
  std::default_random_engine generator;
  std::uniform_real_distribution<float> distribution(0,1);

  Ray r;

  glm::vec2 position (float(x) / float(options.width) * 2.0 - 1.0, 
    float(options.height - y) / float(options.height) * 2.0 - 1.0);

  float rr = distribution(generator) * 10.0;
  float noise = raw_noise_2d(x + rr, y + rr) * 0.001;

  glm::vec3 t3 = glm::normalize(glm::vec3(position + noise, options.near_plane));
  glm::vec4 t4 = options.perspective * glm::vec4(t3,1.0);
  r.direction =  glm::normalize(glm::vec3(t4.x, t4.y, t4.z) / t4.w);
  //r.direction = t3;
  r.origin = origin;
  
  return r;
}


// Test all the geometry to see if we've hit anything and what we've actually hit

bool _sortHits (RayHit i, RayHit j) { return ( i.dist < j.dist); }

bool rayHitTest (Ray &r, RayHit &hit) {

  RayHit ghit;
  std::vector<RayHit> hits;
  if (testGround(r, ghit)){
    hits.push_back(ghit);
  }

  for (Sphere sphere : spheres){
    RayHit hit;
    if ( testSphere(r, sphere, hit) ){
      hits.push_back(hit);
    }
  }

  // Arrange hits in order of distance - lowest first
  if (hits.size() > 0){

    std::sort (hits.begin(), hits.end(), _sortHits);
    hit.loc = hits[0].loc;
    hit.dist = hits[0].dist;
    hit.normal = hits[0].normal;
    hit.colour = hits[0].colour;

    return true;
  }

  return false;

}


// Test to find the angle between our point light source and if its blocked

bool rayLightTest (Ray &r, RayHit &hit) {

  glm::vec3 light_pos (0.0,3.0, 3.0);

  // Move slightly off the origin along the normal
  Ray ray;
  ray.origin = hit.loc + (hit.normal * 0.001f);
  ray.direction = glm::normalize(light_pos - ray.origin);
  RayHit temp_hit;

  return (!rayHitTest(ray,temp_hit));
   
}


// fireRaysMPI

glm::vec3 fireRaysMPI(int x, int y) {

  std::default_random_engine generator;
  std::uniform_real_distribution<float> distribution(0,1);
 
  glm::vec3 colour(0,0,0);

  for (int i=0; i < options.num_rays_per_pixel; ++i){
    // Create a new ray and away we go
    Ray r = fireRay(x,y);
    RayHit hit;
    if (rayHitTest(r,hit)){
      if(rayLightTest(r,hit))
        colour += hit.colour * options.ray_intensity;
    }
  } 

  return colour;
}


// Command line opttions jobby

void parseCommandOptions (int argc, const char * argv[]) {
  int c;
  int digit_optind = 0;
  static struct option long_options[] = {
      {"width", 1, 0, 0},
      {"height", 0, 0, 0},
      {NULL, 0, NULL, 0}
  };
  int option_index = 0;

  while ((c = getopt_long(argc, (char **)argv, "w:h:f:?", long_options, &option_index)) != -1) {
  	int this_option_optind = optind ? optind : 1;
  	switch (c) {
      case 0 :
        break;
      case 'w' :
        options.width = FromStringS9<unsigned int>( std::string(optarg) );
        break;

      case 'f' :
        options.filename = std::string(optarg);
        break;

      case 'n' :
        options.frame = FromStringS9<unsigned int>( std::string(optarg) );
        break;

      case 'h' :
        options.height = FromStringS9<unsigned int>( std::string(optarg) );
        break;

      case '?':
        std::cout << "Basic raytracer. Use -w and -h for size of output" << std::endl;
        break;

      default:
        std::cout << "?? getopt returned character code" << c << std::endl;
      }
  }
  
  if (optind < argc) {
      std::cout << "non-option ARGV-elements: " << std::endl;
      while (optind < argc)
          std::cout << argv[optind++];
      std::cout << std::endl;
  }

}


// Write the bitmap out as an actual file
void writeBitmap (std::vector< std::vector< glm::vec3 > > &bitmap) {

  // TODO - Proper C++ style casts please!

  // Bitmap header
  struct __attribute__ ((packed)) bmp24_file_header
  {
    char        magic1;       // 'B'
    char        magic2;       // 'M'
    int         size;         // 0
    short int   reserved1;    // 0
    short int   reserved2;    // 0
    int         offbytes;      // 14 + 40
  };

  // Bitmap info header
  struct __attribute__ ((packed)) bmp24_info_header
  {
    int        size;                // 40
    int        width;               // pic.width
    int        height;              // pic.height
    short int   planes;             // 1
    short int   bit_count;          // 24
    int        compression;         // 0
    int        size_image;          // (pic.width * 3 + extra_bytes) * pic.height
    int        x_pels_per_meter;    // 2952
    int        y_pels_per_meter;    // 2952
    int        clr_used;            // 0
    int        clr_important;       // 0
  };


  ofstream myfile("frame.bmp",  ios::out | std::ios::binary);

  //std::cout << sizeof(bmp24_file_header) << "," << sizeof(bmp24_info_header) << std::endl;

  bmp24_file_header header;
  bmp24_info_header info;

  int extra_bytes = (4 - (options.width * 3) % 4) % 4;

  // Write the header to the bmp
  header.magic1 = 'B';
  header.magic2 = 'M';
  header.size = 0; // (options.width * 3 + extra_bytes) * options.height + 14 + 40;
  header.reserved1 = 0;
  header.reserved2 = 0;
  header.offbytes =  sizeof(bmp24_file_header) + sizeof(bmp24_info_header);

  // Write the header

  info.size = sizeof(bmp24_info_header);
  info.width = static_cast<long>(options.width);
  info.height = static_cast<long>(options.height);
  info.planes = 1;
  info.bit_count = 24;
  info.compression = 0;
  info.size_image = (options.width * 3 + extra_bytes) * options.height;
  info.x_pels_per_meter = 2952;
  info.y_pels_per_meter = 2952;
  info.clr_used = 0;
  info.clr_important = 0;

  myfile.write( reinterpret_cast<char *>(&header), sizeof(header));
  myfile.write( reinterpret_cast<char *>(&info), sizeof(info));

  for (int y = options.height - 1; y >= 0; --y ){

    for (int x = 0; x < options.width; ++x){
      glm::vec3 colour = bitmap[y][x];
      char r = char( floor( colour.x * 255));
      char g = char( floor( colour.y * 255));
      char b = char( floor( colour.z * 255));
      myfile << r << g << b;
    }
    for (int b = 0; b < extra_bytes; ++b){
      myfile << 'a';
    }
  }
  
  myfile.close();
}


// The server process basically listens for messages
// and pulls in pixel values. TBF we could actually use
// the MPI map and fold functions?

void runServerProcess(int num_mpi_procs) {

  std::vector< std::vector< glm::vec3 > > bitmap;

  // Set the background to black
  for (int i = 0; i < options.height; ++i){
    bitmap.push_back( std::vector< glm::vec3 >() );
    for (int j = 0; j < options.width; ++j){
      bitmap[i].push_back( glm::vec3(0,0,0) );
    }
  }

  // Now listen to the clients and fill our pixel buffer

  long int pixel_count = 0;

  while (pixel_count < options.height * options.width) {
    MPI_Status status;
    int source;
    int tag = 999;
    float colour[3];
    int coord[2];

    for (source = 1; source < num_mpi_procs; ++source ){
      MPI_Recv(coord,2,MPI_UNSIGNED,source,tag,MPI_COMM_WORLD,&status);
      MPI_Recv(colour,3,MPI_FLOAT,source,tag,MPI_COMM_WORLD,&status);

      bitmap[coord[0]][coord[1]] = glm::vec3(colour[0],colour[1],colour[2]);

      pixel_count++;

    }
  }

  // We've got all we need so write out 
  writeBitmap(bitmap);

}

// Run a client process - return the colour data to our server with 
// the pixel we've coloured in.

void runClientProcess(int offset, int range) {

  for (int i=0; i < range; ++i) {
    unsigned int x = (offset + i) % options.width;
    unsigned int y = (offset + i) / options.width;

    glm::vec3 colour = fireRaysMPI(x,y);

    int dest = 0;
    int tag = 999;

    float colour_packed[3];
    colour_packed[0] = colour.x;
    colour_packed[1] = colour.y;
    colour_packed[2] = colour.z;

    unsigned int coord_packed[2];
    coord_packed[0] = x;
    coord_packed[1] = y;

    MPI_Send(colour_packed,3,MPI_FLOAT,dest,tag,MPI_COMM_WORLD);
    MPI_Send(coord_packed,2,MPI_UNSIGNED,dest,tag,MPI_COMM_WORLD);

  }

}

void rayTraceLoop() {

   std::vector<std::vector< glm::vec3 >> bitmap;

  std::default_random_engine generator;
  std::uniform_real_distribution<float> distribution(0,1);
  for (int y = 0; y < options.height; ++y){
    for (int x = 0; x < options.width; ++x){
      for (int i=0; i < options.num_rays_per_pixel; ++i){
      // Create a new ray and away we go
      Ray r = fireRay(x,y);
      RayHit hit;
      if (rayHitTest(r,hit)){
        if(rayLightTest(r,hit))
          bitmap[y][x] += hit.colour * options.ray_intensity;
        }
      }
    }
  }
}

// Our main function - sets up MPI and similar

int main (int argc, const char * argv[]) {

  // Naughty! Stripping const which is a tad bad
  MPI_Init(&argc, const_cast<char***>(&argv));

  int numprocs;
  int myid;
  int length_name;
  char name[200];

  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);
  
  MPI_Get_processor_name(name, &length_name);

  std::cout << "MPI NumProcs: " << numprocs << ", id: " << myid << ", name: " << name << std::endl;  

  // Defaults
  options.width = 320;
  options.height = 240;
  options.num_bounces = 3;
  options.num_rays_per_pixel = 5;
  options.near_plane = 0.1f;
  options.ray_intensity = 0.1;
  options.filename = "teapot.obj";

  parseCommandOptions(argc,argv);

  std::cout << "Rendering size " << options.width << ", " << options.height <<  " for file: " << options.filename << std::endl;

  options.perspective = glm::perspective(48.0f, static_cast<float>(options.width) / static_cast<float>(options.height), options.near_plane, 10.f);

  Sphere s0(glm::vec3(0.0, 0.0, 2.0), 0.5);
  Sphere s1(glm::vec3(1.5, 0.1, 2.0), 0.5);

  spheres.push_back(s0);
  spheres.push_back(s1);

  // Make some decisions about client and server processes

  // We need to divide up the workload on our machines
  if (numprocs > 1){
    if (myid == 0 ) { 
      runServerProcess(numprocs);
    } else {
      // Divide up our image amongst the processes
      int range = options.width * options.height / (numprocs-1);
      runClientProcess(range * (myid-1), range);
    }
  }

  
  MPI_Finalize();

}