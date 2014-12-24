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

//Imagemagick
#include <ImageMagick-6/Magick++.h>

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
	  bool hasTexture();
	  int bufferSize();
	  int numFaces();
	  int getSizeOf();
	  void* getOffSetUV();
	  Vertex* getData();
	  const GLvoid* getImageData();
	  GLsizei getImageColumns();
	  GLsizei getImageRows();
	  
   private:
	  int faces;
	  std::vector<Vertex> data;
	  Assimp::Importer object;
      Magick::Image* m_pImage;
      Magick::Blob m_blob;
	  bool textures;
};
