/*

*/
//Headers
//Glew
#include <GL/glew.h> 
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <GL/glut.h> 

//IO Stream
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
using namespace std;

//Include Classes
#include "texture.h"
#include "shaderLoader.h"
#include "mesh.h"

//GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Structure to hold the objects 
struct Object
   {
	//Object Data
	Mesh objectMesh;

	//Model and function to create the model
	glm::mat4 model;
	glm::mat4 mvp;
	glm::mat4 ModelView;
   };

// Globals to Hold Accesible OpenGL Stuff
int w = 640, h = 480;
GLuint program;
GLint loc_position;
GLint loc_color;
GLint loc_mvpmat;
GLint loc_normal;
GLint loc_modelView;
glm::mat4 view;
glm::mat4 projection;
Object world[2];
int OBJ_INDEX = 0;

//For the Camera
float X_VIEW = 0.0, Y_VIEW = 9.0, Z_VIEW = -9.0;
float X_FOCUS = 0.0, Y_FOCUS = 0.0, Z_FOCUS = 0.0;
float X_FOCUS_ADJ = 0, Y_FOCUS_ADJ = 0, Z_FOCUS_ADJ = 0;
float X_VIEW_ADJ= -2.0, Y_VIEW_ADJ= -7.0, Z_VIEW_ADJ= 6;

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void menu(int id);
void keyboardSpecial(int key, int x_pos, int y_pos);

//--Function Specs
bool initialize();
glm::mat4 getView();
void cleanUp();
bool loadShader(const char *v_shader, const char *f_shader);

//--Main
int main(int argc, char **argv)
   {
	// Initialize glut
	Magick::InitializeMagick(*argv);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(w, h);

	// Name and create the Window
	glutCreateWindow("Lighting");

	GLenum status = glewInit();
	if( status != GLEW_OK)
	   {
		std::cerr << "[F] GLEW NOT INITIALIZED: ";
		std::cerr << glewGetErrorString(status) << std::endl;
		return -1;
	   }

	// Set all of the callbacks to GLUT that we need
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutIdleFunc(update);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboardSpecial);
	int sub = glutCreateMenu(menu);
		glutAddMenuEntry("No Light Shader", 2);
		glutAddMenuEntry("Per Vertex Lighting", 3);
		glutAddMenuEntry("Per Fragment Lighting", 6);
	int sub2 = glutCreateMenu(menu);
		glutAddMenuEntry("Wood Box", 4);
		glutAddMenuEntry("Earth", 5);
	glutCreateMenu(menu);
		glutAddMenuEntry("Quit", 1);
		glutAddSubMenu("Shaders", sub);
		glutAddSubMenu("Objects", sub2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	//Begin the program
	bool init = initialize();
	if( init )
	   {
		glutMainLoop();

		// Clean up after ourselves
		cleanUp();
	   }

	//Return
	return 0;
   }

// This is the render call, it will put the objects to the screen
void render()
   {
	//clear the screen
	glClearColor(0.2, 0.0, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//enable the shader program
	glUseProgram(program);

	//Set the mvp in the object
	world[OBJ_INDEX].ModelView = view * world[OBJ_INDEX].model;
	world[OBJ_INDEX].mvp = projection * view * world[OBJ_INDEX].model;

	//upload the matrix to the shader
	glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(world[OBJ_INDEX].mvp));
	glUniformMatrix4fv(loc_modelView, 1, GL_FALSE, glm::value_ptr(world[OBJ_INDEX].ModelView));

	// Render Objects
	world[OBJ_INDEX].objectMesh.Render(loc_position, loc_color, loc_normal);

	//swap the buffers
	glutSwapBuffers();
   }

//Move the objects through time, and update the models
void update()
   {	   
	//UPdate the camers
	view = getView();

	//Slowly rotate object
	static float t = 0;
	t += 0.005 * M_PI/2;
	world[OBJ_INDEX].model = glm::rotate(glm::mat4(1.0f), (t), glm::vec3(0.0, 1.0, 0.0));

	//Redisplay the object
	glutPostRedisplay();
   }

//If the screen is changed, then update the data
void reshape(int n_w, int n_h)
   {
	w = n_w;
	h = n_h;

	//Change the viewport to be correct
	glViewport( 0, 0, w, h);

	//Update the projection matrix as well
	projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 100.0f);
   }

//Get the input that comes from the keyboard
void keyboard(unsigned char key, int x_pos, int y_pos)
   {
	// Escape Key
	if(key == 27)
	   {
		exit(0);
	   }

	//Reset the camera to complete view
	if(key == ' ')
	   {
		Z_VIEW_ADJ = 0;
		Y_VIEW_ADJ = 11.0;
		X_VIEW_ADJ = 0;
	   }

	//Move the camera
	//X-Locations
	if(key == 'q')
	   {
		if(X_VIEW_ADJ > -30.0)
		   {
			X_VIEW_ADJ -= 1.0;
		   }
	   }
	if(key == 'w')
	   {
		if(X_VIEW_ADJ < 30.0)
		   {
			X_VIEW_ADJ += 1.0;
		   }
	   }
	//Y-Locations
	if(key == 'e')
	   {
		if(Y_VIEW_ADJ > -30.0)
		   {
			Y_VIEW_ADJ -= 1.0;
		   }
	   }
	if(key == 'r')
	   {
		if(Y_VIEW_ADJ < 30.0)
		   {
			Y_VIEW_ADJ += 1.0;
		   }
	   }
	//Z-Locations
	if(key == 't')
	   {
		if(Z_VIEW_ADJ > -27.0)
		   {
			Z_VIEW_ADJ -= 1.0;
		   }
	   }
	if(key == 'y')
	   {
		if(Z_VIEW_ADJ < 28.0)
		   {
			Z_VIEW_ADJ += 1.0;
		   }
	   }

	//Change the Focus
	//X-Focus
	if(key == 'a')
	   {
		X_FOCUS -= 1.0;
	   }
	if(key == 's')
	   {
		X_FOCUS += 1.0;
	   }
	//Y-Focus
	if(key == 'd')
	   {
		Y_FOCUS -= 1.0;
	   }
	if(key == 'f')
	   {
		Y_FOCUS += 1.0;
	   }
	//Z-Focus
	if(key == 'g')
	   {
		Z_FOCUS -= 1.0;
	   }
	if(key == 'h')
	   {
		Z_FOCUS += 1.0;
	   }
   }

//Initilizing the program called by main to set the shaders, objects, and bullet
bool initialize()
   {
	//Read in the object
	bool check1 = world[0].objectMesh.LoadMesh("../bin/objects/wood_box.obj");
		 check1 = world[1].objectMesh.LoadMesh("../bin/objects/earth.obj");

	//Check if the files have loaded in
	if(!check1)
	   {
		std::cerr << "Objects failed to read in."<<std::endl;
		return false;
	   }

	//Load in the default shaders
	if(!loadShader("../bin/shaders/vertexshader.glsl", "../bin/shaders/fragmentshader.glsl"))
	   {
		return false;
	   }

	//Set the view and projetion
	view = glm::lookAt( glm::vec3(X_VIEW, Y_VIEW, Z_VIEW), //Eye position
                        glm::vec3(0.0, 0.0, 0.0),          //Focus Point
                        glm::vec3(0.0, 1.0, 0.0));         //Position Y is up

	projection = glm::perspective( 45.0f,               //the FoV
                                   float(w)/float(h),   //Makes things stay consistent
                                   0.01f,               //Distance to the near plane
                                   100.0f);             //Distanc to the far plane

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//Return Good
	return true;
   }

// Keyboard Arrow Keys
void keyboardSpecial(int key, int x_pos, int y_pos)
   {
	//Left Arrow
	if(key == GLUT_KEY_LEFT)
	   {

	   }

	//Right Arrow
	if(key == GLUT_KEY_RIGHT)
	   {

	   }

	//Up Arrow
	if(key == GLUT_KEY_UP)
	   {

	   }

	//Down Arrow
	if(key == GLUT_KEY_DOWN)
	   {

	   }
   }

// Menu
void menu(int id)
   {	
	//switch on id
	switch (id) 
	   {
		//Escape
		case 1:
			exit(0);
			break;

		case 2:
			loadShader("../bin/shaders/vertexshader2.glsl", "../bin/shaders/fragmentshader2.glsl");
			break;

		case 3:
			loadShader("../bin/shaders/vertexshader.glsl", "../bin/shaders/fragmentshader.glsl");
			break;

		case 4:
			OBJ_INDEX = 0;
			break;

		case 5:
			OBJ_INDEX = 1;
			break;

		case 6:
			loadShader("../bin/shaders/vertexshader3.glsl", "../bin/shaders/fragmentshader3.glsl");
			break;
	   }
		
   }

//Move the camera around
glm::mat4 getView()
   {
	//Variables to update
	float finalXView = X_VIEW  + X_VIEW_ADJ;
	float finalYView = Y_VIEW  + Y_VIEW_ADJ;
	float finalZView = Z_VIEW  + Z_VIEW_ADJ;

	//Check for out of view
	if(finalXView > 22)
	   {
		finalXView = 22;
	   }
	if(finalYView > 22)
	   {
		finalYView = 22;
	   }
	if(finalZView > 22)
	   {
		finalZView = 22;
	   }
	if(finalXView < -22)
	   {
		finalXView = -22;
	   }
	if(finalYView < -22)
	   {
		finalYView = -22;
	   }
	if(finalZView < -22)
	   {
		finalZView = -22;
	   }

	//Update the view to return
	glm::mat4 tempView = glm::lookAt( glm::vec3(finalXView,finalYView,finalZView), //Eye position
	                     glm::vec3(X_FOCUS , Y_FOCUS, Z_FOCUS ),      //Focus Point
	                     glm::vec3(0.0, 1.0, 0.0));                   //Position Y is up

	//Return the accepted view
	return tempView;
   }

//Clean Up all the bullet pointers
void cleanUp()
   {
	glDeleteProgram(program);
   }

//Loads the shaders into the program
bool loadShader(const char *v_shader, const char *f_shader)
   {
	//Create Shader
	Shader vertexShader, fragmentShader;
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	//Set the shader source for compiling
	vertexShader.readFile(v_shader);
	const char* vertexSource = vertexShader.getShaderSource();
	fragmentShader.readFile(f_shader);
	const char* fragmentSource = fragmentShader.getShaderSource();

	//Compile Shaders
	GLint shader_status;

	//Vertex Shader
	char buffer[512];
	glShaderSource(vertex_shader, 1, &vertexSource, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_status);
	if(!shader_status)
	   {
		glGetShaderInfoLog(vertex_shader, 512, NULL, buffer);
		std::cerr << "Vertex Shader failed to compile.\n"<<buffer<<std::endl;
		return false;
	   }

	//Fragment Shader
	glShaderSource(fragment_shader, 1, (const GLchar**) &fragmentSource, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_status);
	if(!shader_status)
	   {
		glGetShaderInfoLog(fragment_shader, 512, NULL, buffer);
		std::cerr << "Fragment Shader failed to compile.\n"<<buffer<<std::endl;
		return false;
	   }

	//Link shader into program
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &shader_status);
	if(!shader_status)
	   {
		std::cerr << "Shader Program failed to link.\n";
		return false;
	   }
   
	//set the locations in the shaders
	//Position
	loc_position = glGetAttribLocation(program, const_cast<const char*>("v_position"));
	if(loc_position == -1)
	   {
		std::cerr << "Position not found.\n";
		return false;
	   }

	//color
	loc_color = glGetAttribLocation(program, const_cast<const char*>("texcoord"));
	if(loc_color == -1)
	   {
		std::cerr << "Texture not found.\n";
		return false;
	   }

	//mvp
	loc_mvpmat = glGetUniformLocation(program, const_cast<const char*>("mvpMatrix"));
	if(loc_mvpmat == -1)
	   {
		std::cerr << "MVP matrix not found.\n";
		return false;
	   }

	//normal
	loc_normal = glGetAttribLocation(program, const_cast<const char*>("normal"));
	if(loc_normal == -1)
	   {
		std::cerr << "Normal not found.\n";
		return false;
	   }

	//model view
	loc_modelView = glGetUniformLocation(program, const_cast<const char*>("ModelView"));
	if(loc_modelView == -1)
	   {
		std::cerr << "ModelView matrix not found.\n";
		return false;
	   }

	//All was well
	return true;
   }
