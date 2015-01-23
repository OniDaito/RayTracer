/**
* @brief Test raytracer for Apocrita
* @file physics.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 22/01/2015
*
*/

#ifndef __physics_hpp__
#define __physics_hpp__

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <algorithm>


#include "geometry.hpp"

const float EPSILON = 1e-9;

int   testTriangle(const Triangle &triangle, const Ray &ray, float &distance );
bool  testSphere (Ray ray, const Sphere &sphere, RayHit &hit);
bool  testAllSpheres(Ray ray, std::vector<Sphere> spheres, RayHit &hit);
bool  testGround (Ray ray, RayHit &hit);

bool  sortHits (RayHit i, RayHit j);

#endif