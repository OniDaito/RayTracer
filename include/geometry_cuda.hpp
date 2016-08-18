/**
* @brief Test raytracer for Apocrita
* @file geometry_cuda.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 15/08/2016
*
*/

#ifndef __geometry_cuda_hpp__
#define __geometry_cuda_hpp__

#include <cuda_runtime.h>
#include <vector_types.h>


// It would appear that defining functions in the header is the preferred style for now

// Basic Ray
struct Ray {
  __device__ Ray(float3 ogn, float3 dir) : origin(ogn), direction(dir){ bounces = 0; }
  __device__ Ray() { 
    bounces = 0; 
    origin.x = 0.0f; origin.y = 0.0f; origin.z = 0.0f;
    direction.x = 0.0f; direction.y = 0.0f; direction.z = 1.0f; 
  }

  float3 origin;      // Ray origin (can possibly bin this)
  float3 direction;   // Ray direction
  int bounces;        // TODO replace or add energy variable for more realism
};


// Basic material - shiny phong
struct Material {
  __device__ Material() {shiny = 0.45; colour.x = 1.0f; colour.y = 1.0f; colour.z = 1.0f; }
  __device__ Material (float3 c, float s) : shiny(s), colour(c) {}
  float shiny;
  float3 colour;
};


// Represents a hit on geometry
struct RayHit{
  float dist;
  float3 loc;
  float3 normal;
};

// Triangle with stored normal
struct Triangle {
  float3 v0, v1, v2;
  float3 normal;
};


// Ground Plane
struct Ground {
  Ground(float g = 0.0f) : height(g) { }
  
  __device__ bool RayIntersection(const Ray &ray, RayHit &hit);

  float height;
  Material *material;
};

// Sphere
struct Sphere {
  __device__ Sphere (float3 c, float r) : centre(c), radius(r) { }
  __device__ bool RayIntersection(const Ray &ray, RayHit &hit); 

  float3 centre;
  float radius;
  Material *material;
};

// Light - rendered as a sphere
struct Light {

  __device__ Light (float3 p, float3 c, float r) : pos(p), radius(r), colour(c) { };
  __device__ bool RayIntersection(const Ray &ray, RayHit &hit); 

  float3 colour;
  float3 pos;
  float radius;
};

#endif

