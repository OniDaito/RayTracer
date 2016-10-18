/**
* @brief Scene creation class
* @file scene_cuda.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 18/08/2016
*
*/

#include <fstream>
#include <sstream>

#include "string_utils.hpp"
#include "scene_cuda.hpp"

using namespace std;
using namespace s9;

// Create some test geometry for our scene
// We read from a file with the following format
// S x y z radius mr mg mb shiny    // Sphere details
// L r g b x y z                    // Lights

Scene CreateScene(RaytraceOptions &options){

  Scene scene;

  scene.sky_colour.x = scene.sky_colour.y = scene.sky_colour.z = 0.0f; 

  if (options.scene_filename != "none"){
    
    ifstream scene_file;
    scene_file.open(options.scene_filename); 
    std::string line;

    while (std::getline(scene_file,line)){

      std::istringstream iss(line);

      if (StringBeginsWith(line,"S")){
        std::string s;
        float r, sy;
        float3 pos;
        float3 mat;
        iss >> s >> pos.x >> pos.y >> pos.z >> r >> mat.x >> mat.y >> mat.z >> sy;
        Sphere *ss = new Sphere(pos,r);
        Material *mm = new Material(mat, sy);
        ss->material = mm;
        scene.spheres.push_back(ss);
#ifdef _DEBUG
        std::cout << "Added Sphere at " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
#endif
        // push back a new lambda function that acts as a closure    
      } else if (StringBeginsWith(line,"L")){
        std::string s;
        float l;
        float3 pos, col;
        iss >> s >> col.x >> col.y >> col.z >> pos.x >> pos.y >> pos.z >> l;
        Light* ll = new Light(pos,col, l);
        scene.lights.push_back(ll); // We add to both objects and lights for now
#ifdef _DEBUG
        std::cout << "Added Light at " << x << ", " << y << ", " << z << std::endl;
#endif
      } else if (StringBeginsWith(line,"C")){
        std::string s;
        float fov, n,f;
        float3 eye, look, up;
        int w,h;
        iss >> s >> eye.x >> eye.y >> eye.z >> look.x >> look.y >> look.z >> up.x >> up.y >> up.z >> w >> h >> fov >> n >> f;
        
        scene.camera = new Camera(
          eye,
          look,
          up,
          w,h,fov,n,f       
        );
#ifdef _DEBUG
        std::cout << "Added Camera at " << ex << "," << ey << "," << ez << std::endl; 
#endif
      } else if (StringBeginsWith(line,"G")){
        std::string s;
        float gy, ms;
        float3 col;
        iss >> s >> gy >> col.x >> col.y >> col.z >> ms;

        scene.ground = new Ground(gy);
        Material *mm = new Material( col, ms);
        scene.ground->material = mm;


      } else if (StringBeginsWith(line,"K")){
        std::string s;
        float3 col;
        iss >> s >> col.x >> col.y >> col.z; 
        scene.sky_colour = col;

      }
 
    }

    return scene;
  } 

    
  // Test Spheres and lights for a default scene
  // Cant have them nearer than the near plane of the camera

  // TODO - roll this into the above function I think

  float3 p0,p1,p2,p3;
  float3 c0,c1,c2,c3,c4;
  p0.x = 0.0f;  p0.y = 1.0f; p0.z = 0.0f;
  p1.x = 2.3f;  p1.y = 1.0f; p1.z = 2.5f;
  p2.x = -3.2f; p2.y = 1.0f; p2.z = -1.5f;
  p3.x = 0.0f;  p3.y = 1.0f; p3.z = 2.0f;
  c0.x = 0.0f;  c0.y = 0.0f; c0.z = 1.0f;
  c1.x = 1.0f;  c1.y = 0.0f; c1.z = 0.0f;
  c2.x = 0.0f;  c2.y = 1.0f; c2.z = 1.0f;
  c3.x = 0.0f;  c3.y = 1.0f; c3.z = 1.0f;
  c4.x = 0.312f;  c4.y = 0.785f; c4.z = 0.213f;


  Sphere *s0 = new Sphere(p0, 1.0f);
  Sphere *s1 = new Sphere(p1, 0.75f);
  Sphere *s2 = new Sphere(p2, 0.75f);
  Sphere *s3 = new Sphere(p3, 0.75f);
  Material *m0 = new Material(c0, 0.0);
  Material *m1 = new Material(c1, 0.1);
  Material *m2 = new Material(c2, 0.9);
  Material *m3 = new Material(c3, 0.2);
  Material *m4 = new Material(c4, 0.2);

  Ground *g0 = new Ground(0.0f);
  g0->material = m4;

  s0->material = m0;
  s1->material = m1;
  s2->material = m2;
  s3->material = m3;

  // TODO - will need to remove the hit function thing for CUDA materials

  scene.spheres.push_back(s0);
  scene.spheres.push_back(s1);
  scene.spheres.push_back(s2);
  scene.spheres.push_back(s3);
  scene.ground = g0;

  // Lights
  float3 lp, lc;
  lp.x = 0.0f; lp.y = 12.0f; lp.z = 3.0f; lc.x = 0.1f; lc.y = 0.1f; lc.z=0.1f;
  Light *l0 = new Light( lp, lc, 5.0f) ;
  scene.lights.push_back(l0);

  // Camera  
  float3 ce,cl,cu;
  ce.x = 0.0f; ce.y = 2.0f; ce.z = 5.0f;
  cl.x = 0.0f; cl.y = 0.0f; cl.z = 0.0f;
  cu.x = 0.0f; cu.y = 1.0f; cu.z = 0.0f;

  scene.camera = new Camera(
    ce,cl,cu,
    options.width,options.height,90.0f,1.0f,100.f       
  ); 

  // Sky
  scene.sky_colour.x = 0.0846f;
  scene.sky_colour.y = 0.0933f;
  scene.sky_colour.z = 0.0949f;

  return scene;
}


