/**
* @brief Test raytracer for Apocrita
* @file camera.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 18/08/2016
*
*/


#ifndef __camera_hpp__
#define __camera_hpp__

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "math_utils.hpp"


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

#endif
