#include <GL/glew.h> // glew must be included before the main gl libs
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier


//--Main
int main(int argc, char **argv)
{
   // Initialize glut
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
   glutInitWindowSize(w, h);
   // Name and create the Window
   glutCreateWindow("Matrix Example");
   
   // Now that the window is created the GL context is fully set up
   // Because of that we can now initialize GLEW to prepare work with shaders
   GLenum status = glewInit();
   if( status != GLEW_OK)
	  {
	  std::cerr << "[F] GLEW NOT INITIALIZED: ";
	  std::cerr << glewGetErrorString(status) << std::endl;
	  return -1;
	  }
   
   // Set all of the callbacks to GLUT that we need
   glutDisplayFunc(render);// Called when its time to display
   glutReshapeFunc(reshape);// Called if the window is resized
   glutIdleFunc(update);// Called if there is nothing else to do
   glutKeyboardFunc(keyboard);// Called if there is keyboard input
   
   // Initialize all of our resources(shaders, geometry)
   bool init = initialize();
   if(init)
	  {
	  t1 = std::chrono::high_resolution_clock::now();
	  glutMainLoop();
	  }
   
   // Clean up after ourselves
   cleanUp();
   return 0;
}