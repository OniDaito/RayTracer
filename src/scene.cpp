/**
* @brief Test raytracer for Apocrita
* @file main.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 7/01/2015
*
*/

#include "scene.hpp"

// Create some test geometry for our scene
// We read from a file with the following format
// S x y z radius mr mg mb shiny    // Sphere details
// L r g b x y z                    // Lights

Scene createScene(RaytraceOptions &options){

  Scene scene;

  if (options.scene_filename != "none"){
    
    ifstream scene_file;
    scene_file.open(options.scene_filename); 
    std::string line;

    while (std::getline(scene_file,line)){

      std::istringstream iss(line);

      if (StringContains(line,"S")){
        std::string s;
        float x,y,z,r, mr, mg, mb, sy;
        iss >> s >> x >> y >> z >> r >> mr >> mg >> mb >> sy; 
        std::shared_ptr<Sphere> ss(glm::vec3(x,y,z),r);
        ss->material.shiny = sy;
        ss->material.colour = glm::vec3(mr,mg,mb);
        scene.objects.push_back(ss);

      } else if (StringContains(line,"L")){
        std::string s;
        float x,y,z,r,g,b;
        iss >> s >> r >> g >> b >> x >> y >> z;
        LightPoint ll;
        ll.pos = glm::vec3(x,y,z);
        ll.colour = glm::vec3(r,g,b);
        scene.lights.push_back(ll);
      }
    
    }
    return scene;
  } 

  // Test Spheres and lights for a default scene

  std::shared_ptr<Sphere> s0(glm::vec3(0.5f, 0.8f, 2.5f), 1.0f);
  std::shared_ptr<Sphere> s1(glm::vec3(1.3f, 0.1f, 3.5f), 0.75f);
  std::shared_ptr<Sphere> s2(glm::vec3(-1.2f, 0.2f, 2.5f), 0.75f);
  std::shared_ptr<Sphere> s3(glm::vec3(0.0f, -0.1f, 2.0f), 0.75f);

  std::shared_ptr<Material> m0(glm::vec3(0.0f,0.0f,1.0f), 0.3);
  std::shared_ptr<Material> m1(glm::vec3(1.0f,0.0f,0.0f), 0.1);
  std::shared_ptr<Material> m2(glm::vec3(0.0f,1.0f,1.0f), 0.9);
  std::shared_ptr<Material> m3(glm::vec3(0.0f,1.0f,1.0f), 0.2);

  s0->material(m0);
  s1->material(m1);
  s2->material(m2);
  s3->material(m3);

  scene.objects.push_back(s0);
  scene.objects.push_back(s1);
  scene.objects.push_back(s2);
  scene.objects.push_back(s3);

  // Lights

  LightPoint l0;
  l0.pos = glm::vec3(1.0f,5.0f,5.0f);
  l0.colour = glm::vec3(0.4f,0.4f,0.4f);
  scene.lights.push_back(l0);

  LightPoint l1;
  l1.pos = glm::vec3(1.0f,0.0f,-1.0f);
  l1.colour = glm::vec3(0.3f,0.3f,0.3f);
  scene.lights.push_back(l1);


  LightPoint l2;
  l1.pos = glm::vec3(-5.0f,5.0f,15.0f);
  l1.colour = glm::vec3(0.1f,0.0f,0.1f);
  scene.lights.push_back(l2);

  return scene;
}


