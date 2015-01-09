/**
* @brief OBJ Parsing Class
* @file obj_mesh.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 17/01/2014
*
*/
__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");

#include <obj_loader.hpp>

using namespace std;
using namespace s9;


ObjMesh::ObjMesh(const s9::File &file) {
  Parse(file);
}


/// Parse the Obj File. At the moment, only accept triangulated mesh files
void ObjMesh::Parse(const s9::File &file) {

  // Temporary locations for the various information
  std::set<ObjVert> uniques;
 
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> texcoords;

  std::vector<TempMesh> temp_meshes;

  std::vector<IndicesType> final_positions;

  bool using_materials = false;

  std::ifstream ifs;
  ifs.open (file.final_path(), std::ifstream::in);

  Directory directory (file.DirectoryName());
  std::vector<File> dir_files = directory.ListFiles();

  cout << "SEBURO ObjMesh Directory Listing..." << endl;
  for (File f : dir_files){
     cout << "  " << f.final_path() << endl;
  }

 
  // Create our Shared Geometry

  ///\todo can we make some guess as to how big our global vertices vectors and such need to be?

  // read the lines
  std::string line;

  TempMesh * current_mesh = nullptr;
  

  while (std::getline(ifs, line)){
      
    // Now the actual vertices, faces and similar


    if ( line.compare(0, 2, "v ") == 0) {

      std::istringstream v(line.substr(2));
      float_t x,y,z;
      v >> x >> y >> z;
      vertices.push_back(glm::vec3(x,y,z));
    } 
    
    else if(line.compare(0, 2, "vt") == 0){

      std::istringstream v(line.substr(3));
      glm::vec2 tex;
      float_t U,V;
      v>>U; v>>V;
      tex= glm::vec2(U,V);
      texcoords.push_back(tex);
    } 

    else if(line.compare(0, 2, "vn") == 0){

      std::istringstream v(line.substr(3));
      float_t x,y,z;
      v >> x >> y >> z;
      normals.push_back(glm::vec3(x,y,z));
    } 


    else if(line.compare(0, 2, "f ") == 0){
      // We have a face definition
      // It seems that faces come after all their vertices are defined.

      std::istringstream v(line.substr(2));

      ///\todo figure out this code
      vector<string> verts{istream_iterator<string>{v},
         istream_iterator<string>{}};

    
      if (verts.size() == 3) {

        ObjFace new_face;

        // This is a triangle so do the normal thing
        for (size_t i = 0; i < 3; ++i){

          // Split on / if there is any
          vector<string> tokens = SplitStringChars(verts[i],"/");
          // Create a new vertex...
          ObjVert new_vertex;
          new_vertex.p = FromStringS9<IndicesType>(tokens[0]);
        
          if (tokens.size() > 1){
            if (tokens[1].length() > 0)
              new_vertex.t =  FromStringS9<IndicesType>(tokens[1]);
            if (tokens.size() > 2)
              new_vertex.n = FromStringS9<IndicesType>(tokens[2]);
          } 


          set<ObjVert>::iterator it = (uniques.insert(new_vertex)).first;

          // it now points to where we need so lets add it to a face
          new_face.vertices[i] = it;
        }

        current_mesh->faces.push_back(new_face);

      } else if (verts.size() == 4){
        // We need to triangulate as this is a silly quad that we dont like ><

        ObjFace new_face_0;
        ObjFace new_face_1;

        ObjVert new_vertex[4];

        for (size_t i = 0; i < 4; ++i){

          // Split on / if there is any
          vector<string> tokens = SplitStringChars(verts[i],"/");
          new_vertex[i].p = FromStringS9<IndicesType>(tokens[0]);
            
          if (tokens.size() > 1){
            if (tokens[1].length() > 0)
              new_vertex[i].t = FromStringS9<IndicesType>(tokens[1]);
            if (tokens.size() > 2)
              new_vertex[i].n = FromStringS9<IndicesType>(tokens[2]);
          }

          set<ObjVert>::iterator it = (uniques.insert(new_vertex[i])).first;

          // Triangulate this quad
          switch (i){
            case 0:
              new_face_0.vertices[0] = it;
              new_face_1.vertices[0] = it;
            break;

            case 1:
              new_face_0.vertices[1] = it;
            break;

             case 2:
              new_face_0.vertices[2] = it;
              new_face_1.vertices[1] = it;
            break;

            case 3:
              new_face_1.vertices[2] = it;
            break;
          }
        }
        current_mesh->faces.push_back(new_face_0);
        current_mesh->faces.push_back(new_face_1);
      }  else if (verts.size() > 4) {
        // Apparently we can work it like a triangular fan

        vector<ObjVert> new_vertex;
        for (size_t i = 0; i < verts.size(); ++i)
          new_vertex.push_back(ObjVert());

        set<ObjVert>::iterator it_start[2];
        set<ObjVert>::iterator it_end;
 
        for (size_t i = 0; i < 2; ++i){

          // Split on / if there is any
          vector<string> tokens = SplitStringChars(verts[i],"/");
          new_vertex[i].p = FromStringS9<IndicesType>(tokens[0]);
            
          if (tokens.size() > 1){
            if (tokens[1].length() > 0)
              new_vertex[i].t = FromStringS9<IndicesType>(tokens[1]);
            if (tokens.size() > 2)
              new_vertex[i].n = FromStringS9<IndicesType>(tokens[2]);
          }

          it_start[i] = (uniques.insert(new_vertex[i])).first;
        }

        it_end = it_start[1];

        // We should have the first two points now
        for (size_t i = 2; i < verts.size() ; ++i) {
          ObjFace new_face;
          ObjVert new_vertex;

          vector<string> tokens = SplitStringChars(verts[i],"/");
          new_vertex.p = FromStringS9<IndicesType>(tokens[0]);
            
          if (tokens.size() > 1){
            if (tokens[1].length() > 0)
              new_vertex.t = FromStringS9<IndicesType>(tokens[1]);
            if (tokens.size() > 2)
              new_vertex.n = FromStringS9<IndicesType>(tokens[2]);
          }


          new_face.vertices[0] = it_start[0];
          new_face.vertices[1] = it_start[1];
          new_face.vertices[2] = (uniques.insert(new_vertex)).first;

          it_start[1] = new_face.vertices[2];

          current_mesh->faces.push_back(new_face);
        }

        // Final Face
        ObjFace new_face;
        new_face.vertices[0] = it_start[0];
        new_face.vertices[1] = it_start[1];
        new_face.vertices[2] = it_end;

        current_mesh->faces.push_back(new_face);
      }
    
    }
  }

  // Now Generate the meshes proper
  // We should have the unique vertices and a stack of faces
  /*
  soup_ = VertexSoup(uniques.size());

  // Copy the unique vertices into our vertex soup
  IndicesType idx = 0;
  for (set<ObjVert>::iterator it = uniques.begin(); it != uniques.end(); ++it){

    soup_.geometry()[idx].p =  vertices[it->p -1];
    soup_.geometry()[idx].n =  normals[it->n -1];
    soup_.geometry()[idx].u =  texcoords[it->t -1];

    *(it->final_pos) = idx;

    idx++;
 
  }

  // For each material, create our own SharedTriMesh

  for (TempMesh temp_mesh : temp_meshes){
    SharedTriMesh mesh(soup_, temp_mesh.faces.size() * 3 );

    idx = 0;
    for (ObjFace face : temp_mesh.faces) {


      mesh.geometry().SetIndex( idx++, *(face.vertices[0]->final_pos));
      mesh.geometry().SetIndex( idx++, *(face.vertices[1]->final_pos));
      mesh.geometry().SetIndex( idx++, *(face.vertices[2]->final_pos));
    }

    Node mesh_node(mesh);
    mesh_node.Add(temp_mesh.material);
    if (temp_mesh.texture)
      mesh_node.Add(temp_mesh.texture);     

    Add(mesh_node);

  }*/


  

  ifs.close();
}