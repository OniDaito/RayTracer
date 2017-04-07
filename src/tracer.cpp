/**
* @brief Test raytracer for Apocrita
* @file tracer.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 22/01/2015
*
*/

#include "tracer.hpp"
#include "main.hpp"
#include "string_utils.hpp"

#ifdef _USE_WINDOW
#include "window.hpp"
#endif

#include <iostream>
#include <ostream>
#include <cstdlib>

using namespace std;
using namespace s9;

// A set of cached values we might need
typedef struct {
  glm::mat4 inv_view_proj;
  float ffx;
  float ffy;

}Cache;


// Pre-define
glm::vec3 TraceRay(Ray ray, const Scene &scene);

// Fire a ray from the origin through our virtual screen
// We need to reverse the projection and such to get the ray into world space where we shall
// work

Ray GenerateRay(float x, float y, const RaytraceOptions &options, std::shared_ptr<Camera> camera, Cache &cache ) {
  
  Ray r;

  //float ffx = tan(camera->fov() / 2.0f);
  //float ratio = camera->width() / camera->height();
  //float ffy = ffx * ratio;

  glm::vec3 pos_on_near (( x / float(options.width) * 2.0f - 1.0f) * cache.ffx, 
    (y / float(options.height) * 2.0f - 1.0f) * cache.ffy,
    camera->near());
 
  // I suspect -1.0f for the w homogenous value is correct?
  glm::vec4 pos_in_homo = cache.inv_view_proj * glm::vec4(pos_on_near,-1.0f);
   
  glm::vec3 pos_in_world  = glm::vec3(pos_in_homo.x, pos_in_homo.y, pos_in_homo.z) /  pos_in_homo.w;

  r.direction = glm::normalize (pos_in_world - camera->position());

  r.origin = camera->position();

  return r;
}


// Given our impact point, return a random ray inside the hemisphere - this is for diffuse surfaces
// Worked out in polar coordinates then converted to cartesian
glm::vec3 HemisphereDiffuseRay(const glm::vec3 &normal) {

  float z = static_cast<float>(std::rand()) / RAND_MAX;  
  float r = sqrt(1.0f - z * z);
  float phi = 2.0f * PI * static_cast<double>(std::rand()) / RAND_MAX;
  float x = cos(phi) * r;
  float y = sin(phi) * r;

  float inv3 = 1.0f / sqrt(3.0f);
  glm::vec3 major_axis;
  if (abs(normal.x) < inv3){
    major_axis = glm::vec3(1.0f,0,0);
  } else if ( abs(normal.y) < inv3){ 
    major_axis = glm::vec3(0,1.0f,0);
  } else {
    major_axis = glm::vec3(0,0,1.0f);
  }

  glm::vec3 u = glm::normalize(glm::cross(major_axis,normal));
  glm::vec3 v = glm::cross(normal,u);
  glm::vec3 w = normal;

  return glm::normalize(u * x + v * y + w * z);
}

/*
glm refractionRay(Ray r, RayHit hit, const Scene &scene){
  Ray rp;

  float eta = 1.0f;
  rp.direction = glm::refract(r.direction,hit.normal,eta);
  rp.origin = hit.loc;
  rp.bounces = r.bounces + 1;

  return traceRay(rp,scene);
}*/



// Trace a ray from the ray's origin to either a hit or its escape from the scene, returning a colour
// Pretty much the meat of the RayTraceKernel
glm::vec3 TraceRay(Ray ray, const RaytraceOptions &options, const Scene &scene, Cache &cache){

  glm::vec3 accum_colour(1.0f,1.0f,1.0f);

  for (int i = 0; i < options.max_bounces; ++i){
    
    // Find the closest thing hit by this ray
    float closest = MAX_DISTANCE;
    RayHit hit, test_hit;
     
    std::shared_ptr<Material> hit_material = nullptr;

    // lamba to do the setting of the hits
    auto do_hit = [](RayHit &td, RayHit &h, float &c) { if (td.dist < c) { c = td.dist; h = td; return true;} return false; };

    // Did we hit an object?
    // For now we split between sphere and ground but eventually
    // we will use pointers to RayIntersectFunc
    for ( std::function<bool(const Ray &ray, RayHit &hit, std::shared_ptr<Material> &m)> rh : scene.intersection_funcs  ) { 
      std::shared_ptr<Material> hm = nullptr; 
      if( rh(ray,test_hit,hm)) {
        if (do_hit(test_hit, hit, closest)){
          hit_material = hm;
        }
      } 
    }

    // Test the ground to see if its closer
    if (scene.ground->RayIntersection(ray, test_hit)){
      if (do_hit(test_hit, hit, closest)){
        hit_material = scene.ground->material;
      }
    }

    // But are the lights any closer?

    std::shared_ptr<Light> light_hit;   
    bool is_light_hit = false;
    
    for ( std::shared_ptr<Light> h : scene.lights) { 
      if( h->RayIntersection(ray,test_hit)) {
        is_light_hit = true;
        if (do_hit(test_hit, hit, closest)){
          light_hit = h;
        }
      } 
    }

    // If we hit a light we can return early
    if (light_hit) { 
      // Direct hit on the light
      if (i==0){
        glm::vec3 cc = light_hit->colour; 
        return cc;
      }
      accum_colour *= light_hit->colour;
      return accum_colour; 
    }

   
    // If we hit update the colour and go again, else add sky colour and break
    // TODO we could move this into a diffuse material func?
    if (hit_material != nullptr){

      glm::vec3 reflected = glm::reflect(ray.direction, hit.normal);
      ray.origin = hit.loc;
      ray.origin += hit.normal * 0.001f;
    
      // Now we need to check the material and fire off a load of diffuse rays depending on shiny
      glm::vec3 diffuse_dir = HemisphereDiffuseRay(hit.normal);
      ray.direction = (diffuse_dir * (1.0f - hit_material->shiny)) + (reflected *  hit_material->shiny);  
      ray.direction = glm::normalize(ray.direction);
      accum_colour *= hit_material->colour;
    }
    else {
      // We hit empty space so break and go for the sky colour
      break;
    }
    
  }
  // Return the sky colour - although it may not have hit the sky if its bounced around a lot

  accum_colour *= scene.sky_colour;
  
  return accum_colour;

}


inline float  maxc (float x) { return x > 1.0f ? 1.0f : x; }
glm::vec3 maxv (glm::vec3 v) { return glm::vec3(maxc(v.x), maxc(v.y), maxc(v.z)); }

// Fire multiple rays for a pixel and combine to make up the final colour
// take the mean average of all the rays

glm::vec3 FireRays(int x, int y, const RaytraceOptions &options, const Scene &scene, Cache &cache) {

  std::shared_ptr<Camera> camera = scene.camera;
  glm::vec3 pixel_colour(0.0f,0.0f,0.0f);
  
  // Make sure the maximum colour doesnt blow up! :S

  // Rays for supersampling within a pixel
  for (int i=0; i < options.supersample; ++i){
  
    float rx = ((static_cast<float>(std::rand()) / RAND_MAX) - 0.5f);
    float ry = ((static_cast<float>(std::rand()) / RAND_MAX) - 0.5f);

    glm::vec3 pixel_colour_inner(0.0f,0.0f,0.0f);
    
    for (int j=0; j < options.num_rays_per_pixel; ++j){
      Ray ray = GenerateRay(float(x) + rx, float(y) + ry, options, camera, cache);
      pixel_colour_inner += TraceRay(ray, options, scene, cache) * options.ray_intensity;
    }
  
    pixel_colour_inner = maxv(pixel_colour_inner);
    pixel_colour += pixel_colour_inner;

  } 

  pixel_colour /= static_cast<float>(options.supersample);

  return pixel_colour;
}

// Create a cache to hopefully speed things up

void CreateCache(const Scene &scene, Cache &cache) {

  float ffx = tan(scene.camera->fov() / 2.0f);
  float ratio = scene.camera->width() / scene.camera->height();
  float ffy = ffx * ratio;

  cache.ffx = ffx;
  cache.ffy = ffy;
  cache.inv_view_proj = glm::inverse(scene.camera->view()) * glm::inverse(scene.camera->projection());
}

// The Core of the Raytracer for an entire frame

void RaytraceKernel(RaytraceBitmap  &bitmap, const RaytraceOptions &options, const Scene &scene ) {

  Cache cache;
  CreateCache(scene,cache);

  #pragma omp parallel for
  for (int i = 0; i < options.height; ++i ){
    for (int j = 0; j < options.width; ++j ) {
      int x = j;
      int y = i;
      glm::vec3 ray_colour = FireRays(x, y, options, scene, cache);
      bitmap.SetRGB(x,y,ray_colour.x, ray_colour.y, ray_colour.z); 
    }
#ifdef _USE_WINDOW
      if (options.live){
        UpdateImage(options);
      }
#endif

  } 
}


