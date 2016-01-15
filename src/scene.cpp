/**
* @brief Scene creation class
* @file scene.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 7/01/2015
*
*/

#include <fstream>
#include <sstream>

#include "string_utils.hpp"
#include "scene.hpp"

using namespace std;
using namespace s9;

// Create some test geometry for our scene
// We read from a file with the following format
// S x y z radius mr mg mb shiny    // Sphere details
// L r g b x y z                    // Lights

Scene CreateScene(RaytraceOptions &options){

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
        std::shared_ptr<Sphere> ss(new Sphere(glm::vec3(x,y,z),r));
        std::shared_ptr<Material> mm (new Material( glm::vec3(mr,mg,mb), sy));
        ss->material(mm);
        scene.objects.push_back(ss);

      } else if (StringContains(line,"L")){
        std::string s;
        float x,y,z,r,g,b,l;
        iss >> s >> r >> g >> b >> x >> y >> z >> l;
        std::shared_ptr<Light> ll ( new Light(glm::vec3(x,y,z), glm::vec3(r,g,b), l));
        scene.lights.push_back(ll);
      
      } else if (StringContains(line,"C")){
        std::string s;
        float ex,ey,ez, lx,ly,lz, ux,uy,uz, fov, n,f;
        int w,h;
        iss >> s >> ex >> ey >> ez >> lx >> ly >> lz >> ux >> uy >> uz >> w >> h >> fov >> n >> f;
        
        scene.camera = std::shared_ptr<Camera> ( new Camera(
          glm::vec3(ex,ey,ez),
          glm::vec3(lx,ly,lz),
          glm::vec3(ux,uy,uz),
          w,h,fov,n,f       
        )); 

      } 
    }

    return scene;
  } 

  // Test Spheres and lights for a default scene
  // Cant have them nearer than the near plane of the camera

  std::shared_ptr<Sphere> s0( new Sphere(glm::vec3(0.0f, 0.0f, 0.0f), 1.0f));
  std::shared_ptr<Sphere> s1( new Sphere(glm::vec3(2.3f, 0.0f, 2.5f), 0.75f));
  std::shared_ptr<Sphere> s2( new Sphere(glm::vec3(-3.2f, 0.0f, -1.5f), 0.75f));
  std::shared_ptr<Sphere> s3( new Sphere(glm::vec3(0.0f, 0.0f, 2.0f), 0.75f));

  std::shared_ptr<Material> m0(new Material(glm::vec3(0.0f,0.0f,1.0f), 0.0));
  std::shared_ptr<Material> m1(new Material(glm::vec3(1.0f,0.0f,0.0f), 0.1));
  std::shared_ptr<Material> m2(new Material(glm::vec3(0.0f,1.0f,1.0f), 0.9));
  std::shared_ptr<Material> m3(new Material(glm::vec3(0.0f,1.0f,1.0f), 0.2));

  s0->material(m0);
  s1->material(m1);
  s2->material(m2);
  s3->material(m3);

  scene.objects.push_back(s0);
  scene.objects.push_back(s1);
  scene.objects.push_back(s2);
  scene.objects.push_back(s3);

  // Lights
  std::shared_ptr<Light> l0 (new Light( glm::vec3(1.0f,5.0f,5.0f),  glm::vec3(0.1f,0.1f,0.1f), 1.0f) );
  scene.lights.push_back(l0);

  // Camera
  
  scene.camera = std::shared_ptr<Camera> ( new Camera(
          glm::vec3(0,0,-5.0),
          glm::vec3(0,0,0),
          glm::vec3(0,1.0,0),
          options.width,options.height,90.0f,1.0f,100.f       
        )); 

  return scene;
}


