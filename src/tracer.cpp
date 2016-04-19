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

#include <iostream>
#include <ostream>
#include <cstdlib>

using namespace std;
using namespace s9;


// Pre-define
glm::vec3 TraceRay(Ray ray, const Scene &scene);

// Fire a ray from the origin through our virtual screen
// We need to reverse the projection and such to get the ray into world space where we shall
// work

Ray GenerateRay(int x, int y, const RaytraceOptions &options, std::shared_ptr<Camera> camera, glm::vec2 offset) {
  
  Ray r;

  float ffx = tan(camera->fov() / 2.0f);
  float ratio = camera->width() / camera->height();
  float ffy = ffx * ratio;

  glm::vec3 pos_on_near (( (float(x) + offset.x) / float(options.width) * 2.0f - 1.0f) * ffx, 
    ((float(y) + offset.y) / float(options.height) * 2.0f - 1.0f) * ffy,
    camera->near());
 
  // I suspect -1.0f for the w homogenous value is correct?
  glm::vec4 pos_in_homo =  glm::inverse(camera->view()) * glm::inverse(camera->projection()) * glm::vec4(pos_on_near,-1.0f);
   
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
glm::vec3 TraceRay(Ray ray, const RaytraceOptions &options, const Scene &scene){

  glm::vec3 accum_colour(1.0f,1.0f,1.0f);

  for (int i = 0; i < options.max_bounces; ++i){
    
    // Find the closest thing hit by this ray
    float closest = MAX_DISTANCE;
    bool did_hit = false;
    RayHit hit, test_hit;
     
    std::shared_ptr<Hittable> hit_object;

    // Did we hit an object?
    for ( std::shared_ptr<Hittable> h : scene.objects) { 
      if( h->RayIntersection(ray,test_hit)) {

        did_hit = true;

        if (test_hit.dist < closest){
          closest = hit.dist;
          hit_object = h;
          hit = test_hit;
        }
      } 
    }


    // If we hit update the colour and go again, else add sky colour and break
    if (did_hit){

      if (hit_object->IsLight()) { 
        // cout << "hit light " << VecToString(accum_colour) << endl;
        // Annoyingly, in classic C++ style we now have to cast :S
         
        std::shared_ptr<Light> light_hit = std::dynamic_pointer_cast<Light>(hit_object);      
        // Direct hit on the light
        if (i==0){
          glm::vec3 cc = light_hit->colour(); 
          return cc;
        }
        accum_colour *= light_hit->colour();
        return accum_colour; 
      } else {
        std::shared_ptr<Material> mat = hit_object->material();
        glm::vec3 reflected = glm::reflect(ray.direction, hit.normal);
        ray.origin = hit.loc;
        ray.origin += hit.normal * 0.001f;
    
        // Now we need to check the material and fire off a load of diffuse rays depending on shiny
        glm::vec3 diffuse_dir = HemisphereDiffuseRay(hit.normal);
        ray.direction = (diffuse_dir * (1.0f - mat->shiny())) + (reflected *  mat->shiny()tt  
        ray.direction = glm::normalize(ray.direction);
        //ray.direction = diffuse_dir;
        //ray.direction = reflected;
        accum_colour *= mat->colour();
      }
    } else {
      // We hit empty space so break and go for the sky colour
      break;
    }
    
    // Russian Roulette early culling of rays if they are getting darker and darker
    if (i > 3) {
      float p = max(accum_colour.x, max(accum_colour.y, accum_colour.z));
      float d = static_cast<float>(std::rand()) / RAND_MAX;

      if (d > p) {
        accum_colour /= p;
        break;
      }        
    }
  }
  // Return the sky colour

  accum_colour *= scene.sky_colour;
  
  return accum_colour;

}


inline float  maxc (float x) { return x > 1.0f ? 1.0f : x; }
glm::vec3 maxv (glm::vec3 v) { return glm::vec3(maxc(v.x), maxc(v.y), maxc(v.z)); }

// Fire multiple rays for a pixel and combine to make up the final colour
// take the mean average of all the rays

glm::vec3 FireRays(int x, int y, const RaytraceOptions &options, const Scene &scene, std::shared_ptr<Camera> camera) {

  glm::vec3 pixel_colour(0.0f,0.0f,0.0f);
  
  // Make sure the maximum colour doesnt blow up! :S

  // Apocrita GCC doesnt like this random stuff :(
  
  // 4 rays for supersampling
  for (int i=0; i < 4; ++i){
  
    float rr = (static_cast<float>(std::rand()) / RAND_MAX) - 0.5f;
    glm::vec2 offset(rr,rr);

    glm::vec3 pixel_colour_inner(0.0f,0.0f,0.0f);
    
    for (int j=0; j < options.num_rays_per_pixel; ++j){
      Ray ray = GenerateRay(x, y, options, camera, offset);
      pixel_colour_inner += TraceRay(ray, options, scene);
    }
  
    pixel_colour_inner = maxv(pixel_colour_inner);
    pixel_colour += pixel_colour_inner;

  } 

  pixel_colour /= 4;

  return pixel_colour;
}

// The Core of the Raytracer for an entire frame

void RaytraceKernel(RaytraceBitmap  &bitmap, const RaytraceOptions &options, const Scene &scene ) {

  #pragma omp parallel for
  for (int i = 0; i < options.height; ++i ){
    for (int j = 0; j < options.width; ++j ) {
     
      int x = j;
      int y = i;

      glm::vec3 ray_colour = FireRays(x, y, options, scene, scene.camera);

      /*unsigned long int colour;
      colour = static_cast<unsigned long> (255 * ray_colour.x) << 16 |
      static_cast<char> (255 * ray_colour.y) << 8 |
      static_cast<char> (255 * ray_colour.z);*/

      bitmap[y][x] = ray_colour;
    }
  } 
}


