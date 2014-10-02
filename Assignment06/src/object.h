//
// Object Class
//

//Include GL Library
#include <GL/glew.h> // glew must be included before the main gl libs
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <GL/glut.h> // doing otherwise causes compiler shouting

//Include assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <vector>

#include "texture.h"

struct Vertex
   {
    GLfloat position[3];
    GLfloat uv[2];
   };

class Object
{
   public:
      //Necessary functions
      Object();
      Object(Object &src);
      ~Object();
	  
	  //setup object file
	  bool readFile(char *fileLoc);
      bool readFile_Alternate(char *fileLoc);
	  bool setMaterials(const aiScene* pScene, char *fileLoc);
	  
	  //get the size for buffer
	  int bufferSize();
	  int numFaces();
	  int getSizeOf();
	  void bindTexture();
	  void* getOffSetUV();
	  Vertex* getData();
	  
   private:
	  int faces;
	  std::vector<Vertex> data;
	  Assimp::Importer object;
	  std::vector<Texture*> textures;
};
