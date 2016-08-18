/**
* @brief Test raytracer for Apocrita
* @file geometry.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 7/01/2015
*
*/

#ifndef __geometry_hpp__
#define __geometry_hpp__

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "math_utils.hpp"

// Basic Ray

struct Ray {
  Ray(glm::vec3 ogn, glm::vec3 dir) : origin(ogn), direction(dir){ bounces = 0; }
  Ray() : origin ( glm::vec3(0,0,0)), direction( glm::vec3(0,0,1)) { bounces = 0; }

  glm::vec3 origin;         // Ray origin (can possibly bin this)
  glm::vec3 direction;      // Ray direction
  int bounces;              // TODO replace or add energy variable for more realism
};

// Basic material - shiny phong
struct Material {
  Material() {shiny = 0.45; colour = glm::vec3(1.0,1.0,1.0); }
  Material (glm::vec3 c, float s) : shiny(s), colour(c) {}
  float shiny;
  glm::vec3 colour;
};

// Represents a hit on geometry
struct RayHit{
  float dist;
  glm::vec3 loc;
  glm::vec3 normal;
};

// Triangle with stored normal
struct Triangle {
  glm::vec3 v0, v1, v2;
  glm::vec3 normal;
};

// Ground Plane
struct Ground {
  Ground(float g = 0.0f) : height(g) { }
  bool RayIntersection(const Ray &ray, RayHit &hit);

  float height;
  std::shared_ptr<Material> material;
};

// Sphere
struct Sphere {
  Sphere (glm::vec3 c, float r) : centre(c), radius(r) { }
  bool RayIntersection(const Ray &ray, RayHit &hit); 

  glm::vec3 centre;
  float radius;
  std::shared_ptr<Material> material;
};

// Light - rendered as a sphere
struct Light {
  Light (glm::vec3 p, glm::vec3 c, float r) : pos(p), radius(r), colour(c) { };
  bool RayIntersection(const Ray &ray, RayHit &hit); 

  glm::vec3 colour;
  glm::vec3 pos;
  float radius;
};


#endif
