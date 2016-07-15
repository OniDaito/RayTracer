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

  glm::vec3 origin;         //Ray origin (can possibly bin this)
  glm::vec3 direction;      //Ray direction
  int bounces;
};


// Basic material - shiny phong
struct Material {
  Material() {shiny = 0.45; colour = glm::vec3(1.0,1.0,1.0); }
  Material (glm::vec3 c, float s) : shiny(s), colour(c) {}
  float shiny;
  glm::vec3 colour;
};


// A GLM based camera for the shooting of the rays! :)
// TODO - should we really have width and height? Just needs to be a ratio I think?
// In addition, the near plane distance and the fov are related so its no wonder we are getting odd results
struct Camera {
  Camera(glm::vec3 position, glm::vec3 lookat,  glm::vec3 up, uint32_t width, uint32_t height, float fov, float near, float far) 
    : up_(up), lookat_(lookat), position_(position), width_ (width), height_(height), fov_(fov), near_(near), far_(far)  {
      Update();      
    }

  glm::vec3 position() { return position_; }
  void position(glm::vec3 p) {position_ = p; Update(); }

  glm::vec3 lookat() { return lookat_; }
  void lookat(glm::vec3 l) {lookat_ = l; Update(); }

  glm::vec3 up() { return up_; }
  void up(glm::vec3 u) {up_ = u; Update(); }

  uint32_t width() { return width_; }
  void width(uint32_t w) { width_ = w; }

  uint32_t height() { return height_; }
  void height(uint32_t h) { height_ = h; }

  glm::mat4 projection() { return projection_; }
  void projection(glm::mat4 m) { projection_ = m; }

  glm::mat4 view() { return view_; }
  void view(glm::mat4 m) { view_ = m; }

  float near() { return near_; }
  void near(float n) { near_ = n; }

  float far() {return far_; }
  void far(float f) {far_ = f; }

  float fov() { return fov_; }
  void fov(float f) { fov_ = f; }

  void Update(){ 
    projection_ = glm::perspectiveFov( fov_ / 2.0f, static_cast<float>(width_),  static_cast<float>(height_), near_, far_);
    view_ = glm::lookAt(position_, lookat_, up_);
  }

  glm::vec3 position_;
  glm::vec3 lookat_;
  glm::vec3 up_;
  
  uint32_t width_, height_;

  glm::mat4 projection_;
  glm::mat4 view_;

  float near_, far_, fov_;

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
