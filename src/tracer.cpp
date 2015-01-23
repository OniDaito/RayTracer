/**
* @brief Test raytracer for Apocrita
* @file tracer.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 22/01/2015
*
*/

#include "tracer.hpp"
#include "physics.hpp"

#include <iostream>
#include <ostream>

// Cheeky Globals (maybe make into a class?)

int MAX_BOUNCES;

// Pre-define
Ray traceRay(Ray r, const Scene &scene);

// Fire a ray from the origin through our vritual screen

Ray generateRay(int x, int y, const int &w, const int &h,
  const float near_plane, const float far_plane,
  const glm::mat4 &perspective) {

  glm::vec3 origin (0,0.0,0);
  
  // Apocrita GCC doesnt like this random stuff :(
  //std::default_random_engine generator;
  //std::uniform_real_distribution<float> distribution(0,1);
  //float rr = distribution(generator) * 2.0f - 1.0f;
  //float noise = raw_noise_2d(rr * 10.0f, rr * 10.0f) * 0.01;

  Ray r;

  glm::vec2 position (float(x) / float(w) * 2.0f - 1.0f, 
    float(h - y) / float(h) * 2.0f - 1.0f);

  //position.x += noise;
  //position.y += noise;

  glm::vec3 t3 = glm::normalize(glm::vec3(position, near_plane));
  glm::vec4 t4 = perspective * glm::vec4(t3,1.0f);
  r.direction =  glm::normalize(glm::vec3(t4.x, t4.y, t4.z) / t4.w);
  r.origin = origin;

  return r;
}


// See if this ray actually intersects with anything in the scene
// If it does not, alter its colour via the light
void shadowRay(Ray &r, const RayHit &hit, const Scene &scene){
  
  RayHit hitlight;

  for (LightPoint l : scene.lights){

    Ray light_ray(hit.loc, glm::normalize( l.pos - hit.loc));

    light_ray.origin = hit.loc + (hit.normal * 0.001f);

    bool occ = false;
 
    if(testAllSpheres(light_ray,scene.spheres,hitlight)){
      // This ray is occluded
      occ = true;      
    }


    // Triangles
    for (Triangle t : scene.triangles){

    }

    if (!occ){
      r.colour += l.colour;
    }
  }
}

// Reflect our ray, taking on some new colours depending on the material
// Reduce the intensity a little bit too!
Ray reflectionRay(Ray ray, RayHit hit, const Scene &scene){
  Ray rp;

  rp.direction = glm::reflect(ray.direction,hit.normal);
  rp.origin = hit.loc;
  rp.bounces = ray.bounces + 1;
  // Not too sure about this colour calculation but nm for now
  //rp.colour = (ray.colour * hit.material.shiny) + (ray.colour * hit.material.colour * (1.0f - hit.material.shiny));
  rp.colour = ray.colour * hit.material.colour;

  return traceRay(rp,scene);

}

Ray refractionRay(Ray r, RayHit hit, const Scene &scene){
  Ray rp;

  float eta = 1.0f;
  rp.direction = glm::refract(r.direction,hit.normal,eta);
  rp.origin = hit.loc;
  rp.bounces = r.bounces + 1;
  // Not too sure about this colour calculation but nm for now
  rp.colour = (r.colour * hit.material.shiny) + (r.colour * hit.material.colour * (1.0f - hit.material.shiny));

  return traceRay(rp,scene);
}


// Recursive call for our ray with reflection and refraction and light tests
Ray traceRay(Ray r, const Scene &scene){

  if (r.bounces < MAX_BOUNCES){
    std::vector<RayHit> hits;
    RayHit hit;

    // Spheres    
    if(testAllSpheres(r,scene.spheres,hit)){
      hits.push_back(hit);
    } 
    if (testGround(r,hit)){
      hits.push_back(hit);
    }

    // Triangles
    for (Triangle t : scene.triangles){
      //hits.push_back(hit);
    }
    

    if (hits.size() > 0){
      std::sort (hits.begin(), hits.end(), sortHits);
      shadowRay(r,hits[0],scene);
      Ray rp = reflectionRay(r,hits[0],scene);
      r.colour += rp.colour;
    }
  }

  return r;
}


// Fire multiple rays for a pixel and combine to make up the final colour

glm::vec3 fireRays(int x, int y, const int &w, const int &h, 
  const glm::mat4 &perspective,
  const float near_plane, const float far_plane,
  const Scene &scene, 
  const int max_bounces,
  const int num_rays_per_pixel ) {

  MAX_BOUNCES = max_bounces;

  //std::default_random_engine generator;
  //std::uniform_real_distribution<float> distribution(0,1);

  glm::vec3 pixel_colour(0.0f,0.0f,0.0f);

  for (int i=0; i < num_rays_per_pixel; ++i){
    Ray ray = generateRay(x,y,w,h,near_plane,far_plane,perspective);
    ray = traceRay(ray,scene);
    pixel_colour += ray.colour;
  } 

  return pixel_colour;
}