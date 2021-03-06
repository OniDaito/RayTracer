
#include "geometry.hpp"
#include "main.hpp"

// SphereRayIntersection Test

bool SphereRayIntersection(const Ray &ray, RayHit &hit, float radius, glm::vec3 centre) {
  float l = glm::dot(ray.direction, (ray.origin - centre));
  float p = pow(l, 2) - pow(glm::distance(ray.origin, centre),2) + pow(radius,2);

  if (p < 0){
    return false;
  }

  p = sqrt(p);

  float dist0 = -l - p;
  float dist1 = -l + p;

  if (abs(dist0 - dist1) < EPSILON){
    return false;
  }

  if( dist1 > 0) hit.dist = dist1;
  if (dist0 < dist1 && dist0 > 0) hit.dist = dist0;
  else return false;

  hit.loc = ray.direction * hit.dist + ray.origin;
  hit.normal = glm::normalize( hit.loc - centre);

  return true;
 
}


// TODO - There seems to be an issue with spheres under 1.0 radius :S

bool Sphere::RayIntersection(const Ray &ray, RayHit &hit){
  return SphereRayIntersection(ray,hit,radius,centre); 
}

bool Light::RayIntersection(const Ray &ray, RayHit &hit) {
  return SphereRayIntersection(ray,hit,radius,pos); 
}

bool Ground::RayIntersection(const Ray &ray, RayHit &hit) {
  
  if ( ray.direction.y >= 0.0f && height <= 0.0f)
    return false;
 
  if ( ray.direction.y < 0.0f && height > 0.0f)
    return false;

  hit.dist =  static_cast<float>(fabs(static_cast<float>(fabs(ray.origin.y - height)) / ray.direction.y));
  
  hit.normal.x = 0.0f;
  hit.normal.y = 1.0f;
  hit.normal.z = 0.0f;
  glm::normalize(hit.normal);

  hit.loc = ray.direction * hit.dist + ray.origin;
  
  return true;

}

bool TestTriangle(const Triangle &triangle, const Ray &ray, float &distance ) {
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
    return true;
  }

  // No hit, no win
return false;
}
