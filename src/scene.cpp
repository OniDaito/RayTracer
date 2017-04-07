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

  scene.sky_colour = glm::vec3(0,0,0);

  if (options.scene_filename != "none"){
    
    ifstream scene_file;
    scene_file.open(options.scene_filename); 
    std::string line;

    while (std::getline(scene_file,line)){

      std::istringstream iss(line);

      if (StringBeginsWith(line,"S")){
        std::string s;
        float x,y,z,r, mr, mg, mb, sy;
        iss >> s >> x >> y >> z >> r >> mr >> mg >> mb >> sy; 
        std::shared_ptr<Sphere> ss(new Sphere(glm::vec3(x,y,z),r));
        std::shared_ptr<Material> mm (new Material( glm::vec3(mr,mg,mb), sy));
        ss->material = mm;
        scene.spheres.push_back(ss);
        std::cout << "Added Sphere at " << x << ", " << y << ", " << z << std::endl;
  
        // push back a new lambda function that acts as a closure    
        auto hitfunc = [ss](const Ray &ray, RayHit &hit, std::shared_ptr<Material> &m) { m = ss->material; return ss->RayIntersection(ray,hit); };
        scene.intersection_funcs.push_back(hitfunc);

      } else if (StringBeginsWith(line,"L")){
        std::string s;
        float x,y,z,r,g,b,l;
        iss >> s >> r >> g >> b >> x >> y >> z >> l;
        std::shared_ptr<Light> ll ( new Light(glm::vec3(x,y,z), glm::vec3(r,g,b), l));
        scene.lights.push_back(ll); // We add to both objects and lights for now
        std::cout << "Added Light at " << x << ", " << y << ", " << z << std::endl;
      
      } else if (StringBeginsWith(line,"C")){
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

        std::cout << "Added Camera at " << ex << "," << ey << "," << ez << std::endl; 

      } else if (StringBeginsWith(line,"G")){
        std::string s;
        float gy, mr, mg, mb, ms;
        iss >> s >> gy >> mr >> mg >> mb >> ms;

        scene.ground = std::shared_ptr<Ground> (new Ground(gy));
        std::shared_ptr<Material> mm (new Material( glm::vec3(mr,mg,mb), ms));
        scene.ground->material = mm;


      } else if (StringBeginsWith(line,"K")){
        std::string s;
        float sr, sg, sb;
        iss >> s >> sr >> sg >> sb; 
        scene.sky_colour = glm::vec3(sr,sg,sb);

      }
 
    }

    return scene;
  } 

  // Test Spheres and lights for a default scene
  // Cant have them nearer than the near plane of the camera

  // TODO - roll this into the above function I think

  std::shared_ptr<Sphere> s0( new Sphere(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f));
  std::shared_ptr<Sphere> s1( new Sphere(glm::vec3(2.3f, 1.0f, 2.5f), 0.75f));
  std::shared_ptr<Sphere> s2( new Sphere(glm::vec3(-3.2f, 1.0f, -1.5f), 0.75f));
  std::shared_ptr<Sphere> s3( new Sphere(glm::vec3(0.0f, 1.0f, 2.0f), 0.75f));
  std::shared_ptr<Material> m0(new Material(glm::vec3(0.0f,0.0f,1.0f), 0.0));
  std::shared_ptr<Material> m1(new Material(glm::vec3(1.0f,0.0f,0.0f), 0.1));
  std::shared_ptr<Material> m2(new Material(glm::vec3(0.0f,1.0f,1.0f), 0.9));
  std::shared_ptr<Material> m3(new Material(glm::vec3(0.0f,1.0f,1.0f), 0.2));
  //std::shared_ptr<Material> m4(new Material(glm::vec3(0.312f,0.785f,0.123f), 0.2));

  std::shared_ptr<Material> m4(new Material(glm::vec3(0.0f, 0.785f, 0.0f), 0.2));
  
  std::shared_ptr<Ground> g0(new Ground(0.0f));
  
  g0->material = m4;

  s0->material = m0;
  s1->material = m1;
  s2->material = m2;
  s3->material = m3;

  auto hitfunc0 = [s0](const Ray &ray, RayHit &hit, std::shared_ptr<Material> &m) { m = s0->material; return s0->RayIntersection(ray,hit); };
  scene.intersection_funcs.push_back(hitfunc0);

  auto hitfunc1 = [s1](const Ray &ray, RayHit &hit, std::shared_ptr<Material> &m) { m = s1->material; return s1->RayIntersection(ray,hit); };
  scene.intersection_funcs.push_back(hitfunc1);

  auto hitfunc2 = [s2](const Ray &ray, RayHit &hit, std::shared_ptr<Material> &m) { m = s2->material; return s2->RayIntersection(ray,hit); };
  scene.intersection_funcs.push_back(hitfunc2);

  auto hitfunc3 = [s3](const Ray &ray, RayHit &hit, std::shared_ptr<Material> &m) { m = s3->material; return s3->RayIntersection(ray,hit); };
  scene.intersection_funcs.push_back(hitfunc3);

  scene.spheres.push_back(s0);
  scene.spheres.push_back(s1);
  scene.spheres.push_back(s2);
  scene.spheres.push_back(s3);
  scene.ground = g0;

  // Lights
  std::shared_ptr<Light> l0 (new Light( glm::vec3(0.0f,12.0f,3.0f),  glm::vec3(0.1f,0.1f,0.1f), 5.0f) );
  scene.lights.push_back(l0);

  // Camera  
  scene.camera = std::shared_ptr<Camera> ( new Camera(
          glm::vec3(0.0f,2.0f,5.0f),
          glm::vec3(0.0f,0.0f,0.0f),
          glm::vec3(0.0f,1.0f,0.0f),
          options.width,options.height,90.0f,1.0f,100.f       
        )); 

  // Sky
  
  scene.sky_colour = glm::vec3(0.0846f, 0.0933f, 0.0949f);

  return scene;
}


