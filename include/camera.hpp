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

#ifdef _USE_CUDA
#include "cuda_math.hpp"
#endif


// A GLM based camera for the shooting of the rays! :)
// TODO - should we really have width and height? Just needs to be a ratio I think?
// In addition, the near plane distance and the fov are related so its no wonder we are getting odd results
struct Camera {

#ifdef _USE_CUDA
  __device__ Camera(float3 position, float3 lookat, float3 up, uint32_t width, uint32_t height, float fov, float near, float far) 
    : up_(up), lookat_(lookat), position_(position), width_ (width), height_(height), fov_(fov), near_(near), far_(far)  {
      Update();      
    }

#else
  Camera(glm::vec3 position, glm::vec3 lookat,  glm::vec3 up, uint32_t width, uint32_t height, float fov, float near, float far) 
    : up_(up), lookat_(lookat), position_(position), width_ (width), height_(height), fov_(fov), near_(near), far_(far)  {
      Update();      
    }
#endif

#ifdef _USE_CUDA

  __device__ float3 position() { return position_; }
  __device__ void position(float3 p) {position_ = p; Update(); }

  __device__ float3 lookat() { return lookat_; }
  __device__ void lookat(float3 l) {lookat_ = l; Update(); }

  __device__ float3 up() { return up_; }
  __device__ void up(float3 u) {up_ = u; Update(); }

  // Dont like this pointer business on the matrices
  __device__ Matrix4 projection() { return projection_; }
  __device__ void projection(Matrix4 &m) { projection_ = m; }

  __device__ Matrix4 view() { return view_; }
  __device__ void view(Matrix4 &m) { view_ = m; }

#else

  glm::vec3 position() { return position_; }
  void position(glm::vec3 p) {position_ = p; Update(); }

  glm::vec3 lookat() { return lookat_; }
  void lookat(glm::vec3 l) {lookat_ = l; Update(); }

  glm::vec3 up() { return up_; }
  void up(glm::vec3 u) {up_ = u; Update(); }

  glm::mat4 projection() { return projection_; }
  void projection(glm::mat4 m) { projection_ = m; }


  glm::mat4 view() { return view_; }
  void view(glm::mat4 m) { view_ = m; }

#endif

  uint32_t width() { return width_; }
  void width(uint32_t w) { width_ = w; }

  uint32_t height() { return height_; }
  void height(uint32_t h) { height_ = h; }
  float near() { return near_; }
  void near(float n) { near_ = n; }

  float far() {return far_; }
  void far(float f) {far_ = f; }

  float fov() { return fov_; }
  void fov(float f) { fov_ = f; }

#ifdef _USE_CUDA
  __device__ void Update() {
    projection_.Perspective(fov_, width_ / height_, near_, far_);
    view_.LookAt(position_, lookat_, up_);
  }

  float3 position_;
  float3 lookat_;
  float3 up_;
  Matrix4 projection_;
  Matrix4 view_;


#else
  void Update(){ 
    projection_ = glm::perspectiveFov( fov_ / 2.0f, static_cast<float>(width_),  static_cast<float>(height_), near_, far_);
    view_ = glm::lookAt(position_, lookat_, up_);
  }

  glm::vec3 position_;
  glm::vec3 lookat_;
  glm::vec3 up_;
  glm::mat4 projection_;
  glm::mat4 view_;
 
#endif

  uint32_t width_, height_;

  float near_, far_, fov_;

};

#endif
