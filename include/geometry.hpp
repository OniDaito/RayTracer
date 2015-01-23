#ifndef __geometry_hpp__
#define __geometry_hpp__

#include <vector>

// Basic Ray
struct Ray {

  Ray(glm::vec3 ogn, glm::vec3 dir) : origin(ogn), direction(dir){ bounces = 0; colour = glm::vec3(0.0,0.0,0.0); }
  Ray() : origin ( glm::vec3(0,0,0)), direction( glm::vec3(0,0,1)) { bounces = 0; colour = glm::vec3(0.0,0.0,0.0);}

  glm::vec3 origin;         //Ray origin (can possibly bin this)
  glm::vec3 direction;      //Ray direction
  glm::vec3 colour;         // Intensity in the 3 colour values
  int bounces;

};


struct Material {
  Material() {shiny = 0.45; colour = glm::vec3(1.0,1.0,1.0); }
  float shiny; // 0 -> 1 with 0 being fully diffuse
  glm::vec3 colour;
};

class Hittable {
public:
  Material material;
};


// Represents a hit on geometry
struct RayHit{
  float dist;
  glm::vec3 loc;
  glm::vec3 normal;
  Material material;
};



// Triangle with stored normal
class Triangle : public Hittable {
public:
  glm::vec3 v0,v1,v2;
  glm::vec3 normal;
};


// Sphere
class Sphere : public Hittable {
public:
  Sphere (glm::vec3 c, float r) : centre(c), radius(r) { }
  glm::vec3 centre;
  float radius;
};

// Light 
struct LightPoint {
  glm::vec3 pos;
  glm::vec3 colour;
  float intensity;
};

// Scene - Collection of all our objects basically
struct Scene {
  std::vector<Sphere>     spheres;
  std::vector<Triangle>   triangles;
  std::vector<LightPoint> lights;
};

#endif