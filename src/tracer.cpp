/**
* @brief Test raytracer for Apocrita
* @file tracer.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 22/01/2015
*
*/

#include "tracer.hpp"
#include "main.hpp"

#include <iostream>
#include <ostream>
#include <random>


// Pre-define
glm::vec3 TraceRay(Ray ray, const Scene &scene);

// Fire a ray from the origin through our vritual screen
// We need to reverse the projection and such to get the ray into world space where we shall
// work

Ray GenerateRay(int x, int y, const RaytraceOptions &options, Camera &camera, glm::vec2 offset) {
  
  Ray r;

  glm::vec2 position ( (float(x) + offset.x) / float(options.width) * 2.0f - 1.0f, 
    (float(options.height - y) + offset.y) / float(options.height) * 2.0f - 1.0f);

  glm::vec3 t3 = glm::normalize(glm::vec3(position, camera.near()));
  glm::vec4 t4 = glm::inverse(camera.projection()) * glm::vec4(t3,1.0f);
  r.direction =  glm::normalize(glm::vec3(t4.x, t4.y, t4.z) / t4.w);
  r.origin = camera.position();

  return r;
}


// See if this ray actually intersects with anything in the scene
// If it does not, alter its colour via the light
glm::vec3 ShadowRay(Ray &r, const Scene &scene){
  
  RayHit hitlight;
  glm::vec3 light_colour(0.0f,0.0f,0.0f);

  for (LightPoint l : scene.lights){

    Ray light_ray(r.origin, glm::normalize( l.pos - r.origin));

    light_ray.origin = r.origin + (r.direction * 0.001f);

    bool occ = false;
 
    RayHit hit;
    for (std::shared_ptr<Hittable> h : scene.objects){
      if (h->RayIntersection(light_ray, hit)){
        occ = true;
        break;
      }
    }

    if (!occ){
      light_colour += l.colour;
    }
  }

  return light_colour;
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
glm::vec3 TraceRay(Ray ray, const RaytraceOptions &options, const Scene &scene){

  glm::vec3 colour(0.0f,0.0f,0.0f);

  for (int i = 0; i < options.max_bounces; ++i){
    
    // Find the closest thing hit by this ray
    float closest = MAX_DISTANCE;
    bool did_hit = false;
    RayHit hit;
    
    std::shared_ptr<Hittable> hit_object;

    for ( std::shared_ptr<Hittable> h : scene.objects) { 
      if( h->RayIntersection(ray,hit)) {
        hit_object = h;
        did_hit = true;

        if (hit.dist < closest){
          closest = hit.dist;
          hit_object = h;
        }
      } 
    }

    // If we hit update the colour and go again, else add sky colour and break
    if (did_hit){
      std::shared_ptr<Material> mat = hit_object->material();
      glm::vec3 reflected = glm::reflect(ray.direction, hit.normal);
      ray.origin = hit.loc;
      ray.direction = reflected;
      colour += ShadowRay(ray,scene) * mat->colour() * (1.0f - mat->shiny());

    } else {
      // Add the sky colour and break out of the loop
      colour += glm::vec3(0.9f, 0.9f, 0.9f);
      break;
    }
  }

  return colour;
}


// Fire multiple rays for a pixel and combine to make up the final colour
// take the mean average of all the rays

glm::vec3 FireRays(int x, int y, const RaytraceOptions &options, const Scene &scene, Camera &camera) {

  glm::vec3 pixel_colour(0.0f,0.0f,0.0f);

  // Apocrita GCC doesnt like this random stuff :(
  std::default_random_engine generator;
  std::uniform_real_distribution<float> distribution(0,1);
  
  for (int i=0; i < options.num_rays_per_pixel; ++i){
    // Super sampling the ray
    float rr = distribution(generator) - 0.5f;
    glm::vec2 offset(rr,rr);

    Ray ray = GenerateRay(x, y, options, camera, offset);
    pixel_colour += TraceRay(ray, options, scene);
  } 

  pixel_colour /= options.num_rays_per_pixel;

  return pixel_colour;
}

// The Core of the Raytracer for an entire frame

void RaytraceKernel(RaytraceBitmap  &bitmap, const RaytraceOptions &options, const Scene &scene, Camera &camera ) {

  #pragma omp parallel for
  for (int i = 0; i < options.height; ++i ){
    for (int j = 0; j < options.width; ++j ) {
     
      int x = j;
      int y = i;

      glm::vec3 ray_colour = FireRays(x, y, options, scene, camera);

      /*unsigned long int colour;
      colour = static_cast<unsigned long> (255 * ray_colour.x) << 16 |
      static_cast<char> (255 * ray_colour.y) << 8 |
      static_cast<char> (255 * ray_colour.z);*/

      bitmap[y][x] = ray_colour;
    }
  } 
}


