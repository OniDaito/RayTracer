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
class Material {
public:
  Material() {shiny_ = 0.45; colour_ = glm::vec3(1.0,1.0,1.0); }
  Material (glm::vec3 colour, float shiny) : shiny_(shiny), colour_(colour) {}
  float shiny() { return shiny_; }
  glm::vec3 colour() { return colour_; }

protected:
  float shiny_;
  glm::vec3 colour_;
};


// A GLM based camera for the shooting of the rays! :)
// TODO - should we really have width and height? Just needs to be a ratio I think?
// In addition, the near plane distance and the fov are related so its no wonder we are getting odd results
class Camera {
public:
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

protected:
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

// Hittable - abstract - mostly used as a generic and to hold a pointer to a material
// Lights are hittable but dont have a material (maybe they will one day :P)
// For now, it sort of makes it a little easier.

class Hittable {
public:
  Hittable () { };  
  virtual bool RayIntersection(const Ray &ray, RayHit &hit) = 0;
  virtual bool IsLight() { return false; };

  std::shared_ptr<Material> material() { return material_;} ;
  void material(std::shared_ptr<Material> m) { material_ = m;};

protected:
  std::shared_ptr<Material> material_;
};

// Ground Plane
class Ground : public Hittable {
public:
  Ground(float g = 0.0f) : height_(g) { }
  
  bool RayIntersection(const Ray &ray, RayHit &hit);

  float height() { return height_; }
protected:
  
  float height_;

};

// Sphere
class Sphere : public Hittable {
public:
  Sphere (glm::vec3 c, float r) : centre_(c), radius_(r) { }
  
  bool RayIntersection(const Ray &ray, RayHit &hit); 

  glm::vec3 centre() { return centre_; }
  void  centre(glm::vec3 c) { centre_ = c; }

  float radius() { return radius_; }
  void radius(float r) { radius_ = r; }

protected:
  glm::vec3 centre_;
  float radius_;
};

// Light - rendered as a sphere
class Light : public Sphere {
public:

  Light (glm::vec3 pos, glm::vec3 colour, float radius) : Sphere(pos, radius), colour_(colour) { };

  bool IsLight() { return true; };
  glm::vec3 colour() { return colour_; }
  
protected:
  glm::vec3 colour_;
};


#endif
