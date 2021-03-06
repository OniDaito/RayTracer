/**
* @brief OBJ File Format parser
* @file obj_mesh.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 17/01/2014
*
*/

#ifndef S9_OBJ_MESH_HPP
#define S9_OBJ_MESH_HPP

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "file.hpp"
#include "math_utils.hpp"
#include "string_utils.hpp"

#include <set>
#include <iterator>

#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <fstream>

namespace s9 {

  typedef uint8_t IndicesType;

  class ObjMesh {
  public:
    ObjMesh(){}
    ObjMesh (const s9::File &file);

  protected:

    void Parse(const s9::File &file); 

    /// A struct that represents a unique vertex

    // Annoyingly, lots of new and delete but we must do this because std::set is immutable

    struct ObjVert{
      IndicesType p,t,n;
      IndicesType *final_pos;

      ObjVert() {
        p = t = n = 0;
        final_pos = new IndicesType;
        *final_pos = 0;
      }

      ObjVert(const ObjVert& other) {
        p = other.p;
        t = other.t;
        n = other.n;
        final_pos = new IndicesType;
        *final_pos = *(other.final_pos);
      }

      ~ObjVert() {
        delete final_pos;
      }

      const bool operator<(const ObjVert& b) const {
        return (p < b.p || (p == b.p && t < b.t) || ( p == b.p && t == b.t && n < b.n));
      }

    };

  
    bool CompareObjVert(const ObjVert &a, const ObjVert &b) {
      return (a.p < b.p || (a.p == b.p &&  a.t < b.t) || ( a.p == b.p && a.t == b.t && a.n < b.n));
    }

    // A struct that represents a triangle
    struct ObjFace {
      std::set<ObjVert>::iterator vertices[3];
    };

    /**
     * A Struct that holds all the information on a mesh-in-potentia, split by material. 
     * It also holds any additional vertices, normals etc that we need to create
     */


    struct TempMesh {

      TempMesh() {  }
    
      std::vector<ObjFace> faces;

      std::string tag;

    };


  };


}


#endif
