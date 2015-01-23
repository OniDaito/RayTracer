/**
* @brief Test raytracer for Apocrita
* @file physics.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 22/01/2015
*
*/


#include "physics.hpp"

#include <iostream>
#include <ostream>

// Test against a triangle

int testTriangle(const Triangle &triangle, const Ray &ray, float &distance ) {
  glm::vec3 e1, e2;  //Edge1, Edge2
  glm::vec3 p, q, t;
  float det, inv_det, u, v;
  float b;
 
  //Find vectors for two edges sharing V1
  e1 = triangle.v1 - triangle.v0;
  e2 = triangle.v2 - triangle.v0;

  //Begin calculating determinant - also used to calculate u parameter
  p = glm::cross(ray.direction, e2);

  //if determinant is near zero, ray lies in plane of triangle
  det = glm::dot(e1, p);

  //NOT CULLING
  if(det > -EPSILON && det < EPSILON) return 0;
  inv_det = 1.f / det;
 
  //calculate distance from V0 to ray origin
  t = ray.origin - triangle.v0;
 
  //Calculate u parameter and test bound
  u = glm::dot(t, p) * inv_det;

  //The intersection lies outside of the triangle
  if(u < 0.f || u > 1.f) return 0;
 
  //Prepare to test v parameter
  q = glm::cross(t,e1);
 
  //Calculate V parameter and test bound
  v = glm::dot(ray.direction, q) * inv_det;
  
  //The intersection lies outside of the triangle
  if(v < 0.f || u + v  > 1.f) return 0;
 
  b = glm::dot(e2, q) * inv_det;
 
  if(b > EPSILON) { //ray intersection
    distance = b;
    return 1;
  }
 
  // No hit, no win
  return 0;
}

// Test against a sphere 5 unit away, of radius 1

bool testSphere (Ray ray, const Sphere &sphere, RayHit &hit) {

  float l = glm::dot(ray.direction, (ray.origin - sphere.centre));
  float p = pow(l, 2) - pow(glm::distance(ray.origin, sphere.centre),2) + pow(sphere.radius,2);

  if (p < 0){
    return false;
  }

  p = sqrt(p);

  float dist0 = -l - p;
  float dist1 = -l + p;

  if (abs(dist0 - dist1) < EPSILON){
    return false;
  }

  // TODO - This is a bit messy
  if (dist0 > 0){
    if (dist1 > 0 && dist1 > dist0){
      hit.dist = dist0;
    } else {
      hit.dist = dist1;
    }

  } else if (dist1 > 1){
    hit.dist = dist1;
  } else {
    // Both negative - quit out
    return false;
  }
  
  hit.loc = ray.direction * hit.dist + ray.origin;
  hit.normal = glm::normalize( hit.loc - sphere.centre);
  hit.material = sphere.material;

  return true;

}

bool sortHits (RayHit i, RayHit j) { return ( i.dist < j.dist); }


// Given a list of spheres test them all and see which we hit first
bool testAllSpheres(Ray ray, std::vector<Sphere> spheres, RayHit &hit){
  RayHit ghit;
  std::vector<RayHit> hits;
  
  for (Sphere sphere : spheres){
    RayHit hit;
    if ( testSphere(ray, sphere, hit) ){
      hits.push_back(hit);
    }
  }

  // Arrange hits in order of distance - lowest first
  if (hits.size() > 0){

    std::sort (hits.begin(), hits.end(), sortHits);
    hit.loc = hits[0].loc;
    hit.dist = hits[0].dist;
    hit.normal = hits[0].normal;
    hit.material = hits[0].material;

    return true;
  }
 
  return false;
}


// Test if the ray hits the ground
// Since we are operating in eye space we make the ground -2 instead of 0

bool testGround (Ray ray, RayHit &hit) {

  if ( ray.direction.y > 0)
    return false;

  // abs term here was actually causing integer conversion oddly :S
  hit.dist = ( (ray.origin.y - 2.0f ) / ray.direction.y);
  
  hit.normal.x = 0.0f;
  hit.normal.y = 1.0f;
  hit.normal.z = 0.0f;

  hit.loc = ray.direction * hit.dist + ray.origin;
  hit.material.colour = glm::vec3(0.1f,0.9f,0.1f);
  hit.material.shiny = 0.1;

  return true;

}
