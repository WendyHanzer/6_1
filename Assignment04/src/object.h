//
// Object Class
//

//Include GL Library
#include <GL/glew.h> // glew must be included before the main gl libs
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <GL/glut.h> // doing otherwise causes compiler shouting

#include <iostream>
#include <vector>

struct Vertex
   {
    GLfloat position[3];
    GLfloat color[3];
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
	  
	  //get the size for buffer
	  int bufferSize();
	  int numFaces();
	  int getSizeOf();
	  void* getOffSetColor();
	  
	  //Vertex of data
	  std::vector<Vertex> data;
	  
   private:
	  int faces;
};
