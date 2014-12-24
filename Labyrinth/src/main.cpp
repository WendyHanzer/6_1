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
//Math
//#include <cmath>
//GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Bullet
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#define BIT(x) (1<<(x))

//Game Controls
bool PAUSED = false;
btVector3 ZEROS(0,0,0);

//Structure to hold the objects 
struct Object
   {
	//Object Data
	Mesh objectMesh;

	//Model and function to create the model
	glm::mat4 model;
	glm::mat4 mvp;
	float defaultX, defaultY, defaultZ;

	//Bullet Data
	btCollisionShape* shape;
	btDefaultMotionState* motion;
	btRigidBody* rigidBody;
	
	//Movement Data
	float linVelocity[3];
	float angVelocity[3];
	float gravity[3];
	void saveMovement(){
				btVector3 temp;
				temp = rigidBody->getLinearVelocity();
				linVelocity[0] = temp.x();
				linVelocity[1] = temp.y();
				linVelocity[2] = temp.z();
				temp = rigidBody->getAngularVelocity();
				angVelocity[0] = temp.x();
				angVelocity[1] = temp.y();
				angVelocity[2] = temp.z();
				temp = rigidBody->getGravity();
				gravity[0] = temp.x();
				gravity[1] = temp.y();
				gravity[2] = temp.z();
				rigidBody->setLinearVelocity(ZEROS);
				rigidBody->setAngularVelocity(ZEROS);
				rigidBody->setGravity(ZEROS);
				rigidBody->clearForces();

			};
	void restoreMovement(){
				rigidBody->setLinearVelocity(btVector3(linVelocity[0],linVelocity[1],linVelocity[2]));
				rigidBody->setAngularVelocity(btVector3(angVelocity[0],angVelocity[1],angVelocity[2]));
				rigidBody->setGravity(btVector3(gravity[0],gravity[1],gravity[2]));
			};

	float x(){
			return model[3][0];
		};
	float y(){
			return model[3][1];
		};
	float z(){
			return model[3][2];
		};

	


   };

// Globals to Hold Accesible OpenGL Stuff
int w = 640, h = 480;
GLuint program;
GLint loc_position;
GLint loc_color;
GLint loc_mvpmat;
GLint loc_normal;
GLint loc_modelView;
GLint loc_isDL;
GLint loc_isSL;
GLint loc_isPL;
GLint loc_isAmb;
GLint loc_Light1x;
GLint loc_Light1y;
GLint loc_Light1z;
GLint loc_Light2x;
GLint loc_Light2y;
GLint loc_Light2z;
glm::mat4 view;
glm::mat4 projection;
int NUMBER_BALLS = 0;
int NUMBER_WALLS = 0;

// box moving stuff
btQuaternion box_qoordinates;
static float xpos = 0.00, ypos = 0.00, zpos = 0.00;
bool UP = false;
bool DOWN = false;
bool LEFT = false;
bool RIGHT = false;
float MOVE_TOLERANCE = .01;
float MOVE_MAX = .05;



std::vector<Object> balls;
std::vector<Object> walls;


// The Bullet World
btBroadphaseInterface* broadphase;
btDefaultCollisionConfiguration* collisionConfiguration;
btCollisionDispatcher* dispatcher;
btSequentialImpulseConstraintSolver* solver;
btDiscreteDynamicsWorld* dynamicsWorld;
int collision;
void myTickCallback(btDynamicsWorld *world, btScalar timeStep);

//For the Camera
float X_VIEW = 0.0, Y_VIEW = 9.0, Z_VIEW = -9.0;
float X_FOCUS = 0.0, Y_FOCUS = 0.0, Z_FOCUS = 0.0;
float X_FOCUS_ADJ = 0, Y_FOCUS_ADJ = 0, Z_FOCUS_ADJ = 0;
float X_VIEW_ADJ= -10.0, Y_VIEW_ADJ= 1.0, Z_VIEW_ADJ= 9.0;



//Game Options
int DIFFICULTY = 0; //0 = Easy, 1 = Medium, 2 = Hard, 3 = Inverted
bool CAM_FOLLOW = 0;
int CAM_MODE;
GLclampf BG_R = .2;
GLclampf BG_G = 0.0;
GLclampf BG_B = 1.0;




//For Mouse Controls
int MOUSE_X, MOUSE_Y;
float MOUSE_SENSITIVITY = .5;
int CURSOR_VISIBLE = 1;

//Score
int SCORE = 0;
float timeChange = 0.0;
int level = 1;

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void menu(int id);
void keyboardSpecial(int key, int x_pos, int y_pos);
void mouseMovement(int mouseX, int mouseY);

//--Function Specs
bool initialize();
void myKey(unsigned char key, int x, int y);
glm::mat4 getView();
bool load(const char*);
void writeString(char* line, float x, float y);
void cleanUp();
void resetGame();
void resetGoal();
float calcVelocity(btVector3 vel);
bool loadBall();
bool loadLevel(int level);

//--Time Keeping
std::chrono::time_point<std::chrono::high_resolution_clock> t0,t1,t2;
float getDT();

//Lighting Controls
float ENABLE_AMB = 1;
float ENABLE_DL = 1;
float ENABLE_PL = 1;
float ENABLE_SL = 1.2;
float light1x = 1;
float light1y = -1;
float light1z = 1;
float light2x = -1;
float light2y = -1;
float light2z = 1;
void updateSpotLights();
int lightControlled = 0;
void lightsEnabled();

//--Main
int main(int argc, char **argv)
   {
	// Initialize glut
	Magick::InitializeMagick(*argv);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(w, h);

	// Name and create the Window
	glutCreateWindow("Labryinth");

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
	glutPassiveMotionFunc(mouseMovement);
	glutSetCursor(GLUT_CURSOR_NONE);
	int sub = glutCreateMenu(menu);
		glutAddMenuEntry("Add One", 5);
		glutAddMenuEntry("Remove One", 6);
	int sub2 = glutCreateMenu(menu);
		glutAddMenuEntry("Level 1", 7);
		glutAddMenuEntry("Level 2", 8);
	int sub3 = glutCreateMenu(menu);
		glutAddMenuEntry("Slow and Boring", 9);
		glutAddMenuEntry("Moderate but Doable", 10);
		glutAddMenuEntry("Fast and Blind", 11);
	glutCreateMenu(menu);
		glutAddMenuEntry("Quit", 1);
		glutAddMenuEntry("Start", 2);
		glutAddMenuEntry("Pause", 3);
		glutAddMenuEntry("Restart", 4);
		glutAddSubMenu("Balls", sub);
		glutAddSubMenu("Levels", sub2);
		glutAddSubMenu("Difficulty", sub3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	//Begin the program
	bool init = initialize();
	if( init )
	   {
		t1 = std::chrono::high_resolution_clock::now();
		t0 = std::chrono::high_resolution_clock::now();
		glutMainLoop();

		// Clean up after ourselves
		cleanUp();
	   }

	//Return
	return 0;
   }

//x at 5.5,y < 0 

// This is the render call, it will put the objects to the screen
void render()
   {
	//clear the screen
	glClearColor(BG_R, BG_G, BG_B, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//enable the shader program
	glUseProgram(program);

	//Get the time change
	float dt = getDT();
	timeChange += dt;

	//Update the current world
	dynamicsWorld->stepSimulation(1.f/60.f, 1);

	//Loop through all the objects to output
	for(int i = 0; i < NUMBER_BALLS; i++)
	   {
		//Variables
		btTransform trans;
		btScalar m[16];

		//Get the new transformation
		balls[i].rigidBody->getMotionState()->getWorldTransform(trans);

		//Output Location
		//std::cout<<"Model " << i << " Y: "<< trans.getOrigin().getY() << " X: " << trans.getOrigin().getX() << " Z: " << trans.getOrigin().getZ() << std::endl;

		//Set up the new model
		trans.getOpenGLMatrix(m);

		if(collision == 3 && timeChange > 1)
		   {
			float distance = sqrt(pow(balls[i].x()- walls[0].x(),2) + pow(balls[i].y()- walls[0].y(),2) + pow(balls[i].z()- walls[0].z(),2));
			if(distance < 1.4)
			{
				// Print the score
				SCORE = timeChange;
				printf("Score: %d\n", SCORE);
				if(level == 1)
					{
						menu(8);
					}
				else
					{
						menu(7);
					}

				
			}
		   }

		if(trans.getOrigin().getY() < (balls[i].defaultY - 20.0))
		   {
			resetGame();
		   }

		if((walls[0].y() < -20.0) || abs(walls[0].x() - walls[0].defaultX) > 3 || abs(walls[0].z() - walls[0].defaultZ) > 3)
		   {
			resetGoal();
		   }
		glm::vec4 one = glm::vec4(m[0], m[1], m[2], m[3]);
		glm::vec4 two = glm::vec4(m[4], m[5], m[6], m[7]);
		glm::vec4 three = glm::vec4(m[8], m[9], m[10], m[11]);
		glm::vec4 four = glm::vec4(m[12], m[13], m[14], m[15]);
		glm::mat4 temp = glm::mat4(one, two, three, four);

		balls[i].model = temp;

		//Set the mvp in the object
		balls[i].mvp = projection * view * balls[i].model;

		//upload the matrix to the shader
		glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(balls[i].mvp));
		glUniformMatrix4fv(loc_modelView, 1, GL_FALSE, glm::value_ptr(balls[i].model));
	
	  	 glUniform1f(loc_isAmb,ENABLE_AMB );
	  	 glUniform1f(loc_isDL,ENABLE_DL );
	 	 glUniform1f(loc_isPL,ENABLE_PL );
	  	 glUniform1f(loc_isSL,ENABLE_SL );
	   
	  	 glUniform1f(loc_Light1x, light1x );
	  	 glUniform1f(loc_Light1y, light1y );
	   	 glUniform1f(loc_Light1z, light1z );
	   
	  	 glUniform1f(loc_Light2x, light2x );
	  	 glUniform1f(loc_Light2y, light2y );
	  	 glUniform1f(loc_Light2z, light2z );

		// Render Objects
		balls[i].objectMesh.Render(loc_position, loc_color, loc_normal);
	   }

	//Loop through all the walls
	for(int i = 0; i < NUMBER_WALLS; i++)
	   {
		if(i == 0 || i == level)
		   {
			//Variables
			btTransform trans;
			btScalar m[16];

			//Get the new transformation
			walls[i].rigidBody->getMotionState()->getWorldTransform(trans);

			//Output Location
			//std::cout<<"Model " << i << " Y: "<< trans.getOrigin().getY() << " X: " << trans.getOrigin().getX() << " Z: " << trans.getOrigin().getZ() << std::endl;

			//Set up the new model
			trans.getOpenGLMatrix(m);

			glm::vec4 one = glm::vec4(m[0], m[1], m[2], m[3]);
			glm::vec4 two = glm::vec4(m[4], m[5], m[6], m[7]);
			glm::vec4 three = glm::vec4(m[8], m[9], m[10], m[11]);
			glm::vec4 four = glm::vec4(m[12], m[13], m[14], m[15]);
			glm::mat4 temp = glm::mat4(one, two, three, four);

			walls[i].model = temp;

			//Set the mvp in the object
			walls[i].mvp = projection * view * walls[i].model;

			//upload the matrix to the shader
			glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(walls[i].mvp));
			glUniformMatrix4fv(loc_modelView, 1, GL_FALSE, glm::value_ptr(walls[i].model));
	
	  	 	glUniform1f(loc_isAmb,ENABLE_AMB );
	  	 	glUniform1f(loc_isDL,ENABLE_DL );
	 	 	glUniform1f(loc_isPL,ENABLE_PL );
	  	 	glUniform1f(loc_isSL,ENABLE_SL );
	   
	  	 	glUniform1f(loc_Light1x, light1x );
	  	 	glUniform1f(loc_Light1y, light1y );
	   	 	glUniform1f(loc_Light1z, light1z );
	   
	  		glUniform1f(loc_Light2x, light2x );
	  	 	glUniform1f(loc_Light2y, light2y );
	  		glUniform1f(loc_Light2z, light2z );

			// Render Objects
			walls[i].objectMesh.Render(loc_position, loc_color, loc_normal);
		   }
	   }

	//Setup and output the scores
	char buffer[250];
	int min = timeChange/60;
	int sec = timeChange - (min*60);
	sprintf(buffer, "Previous Score     %d             TIME    %d:%.2d", SCORE, min, sec);
	writeString(buffer, -0.9, 0.75);

	//swap the buffers
	glutSwapBuffers();
   }

//Move the objects through time, and update the models
void update()
   {	   
	//UPdate the camera
	if(CAM_FOLLOW == 1)
	{
		if(NUMBER_BALLS > 0)
			{
				X_FOCUS = balls[0].model[3][0];
				Z_FOCUS = balls[0].model[3][2];
			}
	}
	view = getView();
	updateSpotLights();
	//Get the matrix location
	btTransform trans;
	walls[level].rigidBody->getMotionState()->getWorldTransform(trans);
	box_qoordinates = trans.getRotation();

	btVector3 v = walls[level].rigidBody->getAngularVelocity();
	if(!PAUSED)
	{
		// RIGHT KEY
		 if( box_qoordinates.x() <= -0.100000 )
		 {
		  RIGHT = false;
		if(v.x() < 0)
			{
				walls[level].rigidBody->setAngularVelocity( btVector3(-v.x()/2, 0, v.z()) );
				xpos = xpos * 0;
			}
		 }
		 else
		 {
		  RIGHT = true;
		 }
		// LEFT KEY
		 if( box_qoordinates.x() >= 0.100000 )
		 {
		  LEFT = false;
		if(v.x() > 0)
			{
				walls[level].rigidBody->setAngularVelocity( btVector3(-v.x()/2, 0, v.z()) );
				xpos = xpos * 0;
			}
		 }
		 else
		 {
		  LEFT = true;
		 }
		// DOWN KEY
		 if( box_qoordinates.z() >= 0.100000 )
		 {
		  DOWN = false;
		if(v.z() > 0)
			{
		 	 walls[level].rigidBody->setAngularVelocity( btVector3(v.x(), 0, -v.z()/2) );
		 	 zpos = zpos * 0;
			}
		 }

		 else
		 {
		  DOWN = true;
		 }

		// UP KEY
		 if( box_qoordinates.z() <= -0.100000 )
		 {
		  UP = false;
		 if(v.z() < 0)
		 	{
		  	 walls[level].rigidBody->setAngularVelocity( btVector3(v.x(), 0, -v.z()/2) );
		  	 zpos = zpos * 0;
		 	}
		 }
		 else
		 {
		  UP = true;
		 }
	}
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

	//Toggle Lighting Options
	if(key == 'c')
	   {
		   ENABLE_SL = !ENABLE_SL;
	   }

	if(key == 'v')
	{
		   ENABLE_AMB = !ENABLE_AMB;

	}
	if(key == 'b')
	{
		   ENABLE_DL = !ENABLE_DL;
		   ENABLE_PL = !ENABLE_PL;
	}

	//Reset the camera to complete view
	if(key == ' ')
	   {
		CAM_MODE++;
		switch(CAM_MODE)
		{
			case 0:
				X_FOCUS = Y_FOCUS = Z_FOCUS = 0.0;
				X_VIEW_ADJ= -10.0;
				Y_VIEW_ADJ= 1.0;
				Z_VIEW_ADJ= 9.0;
				X_VIEW = 0.0; 
				Y_VIEW = 9.0;
				Z_VIEW = -9.0;
				CAM_FOLLOW = 0;
			break;
			case 1:
				Z_VIEW_ADJ = -8.0;
				Y_VIEW_ADJ = 1.0;
				X_VIEW_ADJ = 9.0;
				CAM_FOLLOW = 0;
			break;
			case 2:
				Z_VIEW_ADJ = 0;
				Y_VIEW_ADJ = 0;
				X_VIEW_ADJ = 0;
				CAM_MODE = -1;
				CAM_FOLLOW = 1;
			break;
			
		}
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

	//Reset the board
	if(key == 'p')
	   {
	   	resetGame();
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
	//Reset the board
	if(key == '0')
	   {
	   	MOUSE_SENSITIVITY +=.1;
	   }
	if(key == '9')
	   {
	   	MOUSE_SENSITIVITY -= .1;
	   }

	//add balls
	if(key == '=')
	   {
		if(NUMBER_BALLS < 10)
		   {
			dynamicsWorld->addRigidBody(balls[NUMBER_BALLS].rigidBody);
			NUMBER_BALLS++;
		   }
		resetGame();
	   }
	if(key == '-')
	   {
		if(NUMBER_BALLS > 0)
		   {
			NUMBER_BALLS--;
			dynamicsWorld->removeRigidBody(balls[NUMBER_BALLS].rigidBody);
		   }
		resetGame();
	   }
   }

//Initilizing the program called by main to set the shaders, objects, and bullet
bool initialize()
   {
    //Init Bullet
	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	solver = new btSequentialImpulseConstraintSolver;

	// Putting Bullet Together
	dynamicsWorld = new btDiscreteDynamicsWorld( dispatcher, broadphase, solver, collisionConfiguration);

	// Activate Gravity
	dynamicsWorld->setGravity(btVector3(0, -6.81, 0));

	//Set at least one ball and first level
	balls.resize(10);
	bool check1 = loadBall();
	check1 = loadLevel(1) && check1;
	dynamicsWorld->setInternalTickCallback(myTickCallback);

	//Check if the files have loaded in
	if(!check1)
	   {
		std::cerr << "Objects failed to read in."<<std::endl;
		return false;
	   }

	//Create Shader
	Shader vertexShader, fragmentShader;
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	//Set the shader source for compiling
	vertexShader.readFile("../bin/vertexshader3.glsl");
	const char* vertexSource = vertexShader.getShaderSource();
	fragmentShader.readFile("../bin/fragmentshader3.glsl");
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
	   
	   //Enable Ambience
	   loc_isAmb = glGetUniformLocation(program, const_cast<const char*>("isAmb"));
	   if(loc_isAmb == -1)
	   {
		   std::cerr << "Amb not found.\n";
		   return false;
	   }
	   
	   //Enable DirectLight
	   loc_isDL = glGetUniformLocation(program, const_cast<const char*>("isDL"));
	   if(loc_isDL == -1)
	   {
		   std::cerr << "DL not found.\n";
		   return false;
	   }
	   
	   //Enable PointLight
	   loc_isPL = glGetUniformLocation(program, const_cast<const char*>("isPL"));
	   if(loc_isPL == -1)
	   {
		   std::cerr << "PL not found.\n";
		   return false;
	   }
	   
	   //Enable SpotLight
	   loc_isSL = glGetUniformLocation(program, const_cast<const char*>("isSL"));
	   if(loc_isSL == -1)
	   {
		   std::cerr << "SL not found.\n";
		   return false;
	   }
	   
	   //Enable Lighting 1 Coordinates
	   loc_Light1x = glGetUniformLocation(program, const_cast<const char*>("light1x"));
	   if(loc_Light1x == -1)
	   {
		   std::cerr << "Light 1 X position not found.\n";
		   return false;
	   }
	   
	   loc_Light1y = glGetUniformLocation(program, const_cast<const char*>("light1y"));
	   if(loc_Light1y == -1)
	   {
		   std::cerr << "Light 1 Y position not found.\n";
		   return false;
	   }
	   
	   loc_Light1z = glGetUniformLocation(program, const_cast<const char*>("light1z"));
	   if(loc_Light1z == -1)
	   {
		   std::cerr << "Light 1 Z position not found.\n";
		   return false;
	   }
	   
	   //Enable Lighting 2 Coordinates
	   loc_Light2x = glGetUniformLocation(program, const_cast<const char*>("light2x"));
	   if(loc_Light2x == -1)
	   {
		   std::cerr << "Light 2 X position not found.\n";
		   //return false;
	   }
	   
	   loc_Light2y = glGetUniformLocation(program, const_cast<const char*>("light2y"));
	   if(loc_Light2y == -1)
	   {
		   std::cerr << "Light 2 Y position not found.\n";
		   //return false;
	   }
	   
	   loc_Light2z = glGetUniformLocation(program, const_cast<const char*>("light2z"));
	   if(loc_Light2z == -1)
	   {
		   std::cerr << "Light 2 Z position not found.\n";
		   //return false;
	   }

	//Set the view and projetion
	view = glm::lookAt( glm::vec3(X_VIEW, Y_VIEW, Z_VIEW), //Eye position
                        glm::vec3(0.0, 0.0, 0.0),          //Focus Point
                        glm::vec3(0.0, 1.0, 0.0));         //Position Y is up

	projection = glm::perspective( 45.0f,               //the FoV
                                   float(w)/float(h),   //Makes things stay consistent
                                   0.01f,               //Distance to the near plane
                                   100.0f);             //Distance to the far plane

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//Return Good
	return true;
   }

//returns the time delta
float getDT()
   {
	float ret;
	t2 = std::chrono::high_resolution_clock::now();
	ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
	t1 = std::chrono::high_resolution_clock::now();
	return ret;
   }

// Keyboard Arrow Keys
void keyboardSpecial(int key, int x_pos, int y_pos)
   {
   	//Check if its an arrow key
	if( (key == GLUT_KEY_LEFT || key == GLUT_KEY_DOWN || key == GLUT_KEY_UP || key == GLUT_KEY_RIGHT) && (!PAUSED))
	   {
		//Right Arrow
		if(key == GLUT_KEY_RIGHT)
		   {
		    if( LEFT == true )
		    {
		     if( xpos < MOVE_MAX )
		     {
		      walls[level].rigidBody->setAngularVelocity( btVector3(xpos+= MOVE_TOLERANCE, ypos, zpos) );
		     }
		     else
		     {
		      //do nothing
		     }
		    }
		   }

		//Left Arrow
		if(key == GLUT_KEY_LEFT)
		   {
		    if( RIGHT == true )
		    {
		     if( xpos > -MOVE_MAX )
		     {
		      walls[level].rigidBody->setAngularVelocity( btVector3(xpos-= MOVE_TOLERANCE, ypos, zpos) );
		     }
		     else
		     {
		      //do nothing
		     }
		    }
		   }

		//Down Arrow
		if(key == GLUT_KEY_DOWN)
		   {
		    if(DOWN == true)
		    {
		     if( zpos < MOVE_MAX )
		     {
		      walls[level].rigidBody->setAngularVelocity( btVector3(xpos, ypos, zpos+= MOVE_TOLERANCE) );
		     }
		     else
		     {
		      //do nothing
		     }
		    }
		   }

		//Up Arrow
		if(key == GLUT_KEY_UP)
		   {
		    if(UP == true)
		    {
		     if( zpos > -MOVE_MAX)
		     {
		      walls[level].rigidBody->setAngularVelocity( btVector3(xpos, ypos, zpos-= MOVE_TOLERANCE) );
		     }
		     else
		     {
		      //do nothing
		     }
		    }
		   }
	   }

	if(key == GLUT_KEY_F1)
	   {
		if(CURSOR_VISIBLE == 1)
		   {
			glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
			CURSOR_VISIBLE = 0;
		   }
		else
		   {
			glutSetCursor(GLUT_CURSOR_NONE);
			CURSOR_VISIBLE = 1;
		   }
	   }
   }

// Menu
void menu(int id)
   {
   	/*
   		How this data is setup 

	int sub = glutCreateMenu(menu);
		glutAddMenuEntry("Add One", 5);
		glutAddMenuEntry("Remove One", 6);
	int sub2 = glutCreateMenu(menu);
		glutAddMenuEntry("Level 1", 7);
		glutAddMenuEntry("Level 2", 8);
	glutCreateMenu(menu);
		glutAddMenuEntry("Quit", 1);
		glutAddMenuEntry("Start", 2);
		glutAddMenuEntry("Pause", 3);
		glutAddMenuEntry("Restart", 4);
		glutAddSubMenu("Balls", sub);
		glutAddSubMenu("Levels", sub2);
    */
	
	//switch on id
	switch (id) 
	   {
		//Escape
		case 1:
			exit(0);
			break;
		case 2:
		 	if(PAUSED)
		 	   {
				for(int i = 0; i < NUMBER_BALLS; i++)
				{
					balls[i].restoreMovement();
				}				
				walls[level].restoreMovement();
		 	   	PAUSED = false;

		 	   }
			break;
		case 3:
			if(!PAUSED)
			   {
				for(int i = 0; i < NUMBER_BALLS; i++)
				{
					balls[i].saveMovement();
				}
				walls[level].saveMovement();
				PAUSED = true;
			   }
			break;
		case 4:
			SCORE = 0;
			resetGame();
			break;
		case 5:
			if(NUMBER_BALLS < 10)
			   {
				if(NUMBER_BALLS == 1)
				{
					dynamicsWorld->addRigidBody(balls[NUMBER_BALLS].rigidBody);
				NUMBER_BALLS++;	
				}
					dynamicsWorld->addRigidBody(balls[NUMBER_BALLS].rigidBody);	
				NUMBER_BALLS++;
			   }
			resetGame();
			break;
		case 6:
			if(NUMBER_BALLS > 0)
			   {
				if(NUMBER_BALLS == 3)
				{
					NUMBER_BALLS--;
					dynamicsWorld->removeRigidBody(balls[NUMBER_BALLS].rigidBody);
				}
				NUMBER_BALLS--;
				dynamicsWorld->removeRigidBody(balls[NUMBER_BALLS].rigidBody);
			   }
			
			break;
		case 7:
			if(level != 1)
			   {
				walls[level].rigidBody->clearForces();
				walls[level].rigidBody->setAngularVelocity(ZEROS);
				level = 1;
				dynamicsWorld->removeRigidBody(walls[2].rigidBody);
				dynamicsWorld->addRigidBody(walls[1].rigidBody);
				resetGame();
				resetGoal();
			   }
			break;
		case 8:
			if(level != 2)
			   {
				walls[level].rigidBody->clearForces();
				walls[level].rigidBody->setAngularVelocity(ZEROS);
				level = 2;
				dynamicsWorld->removeRigidBody(walls[1].rigidBody);
				dynamicsWorld->addRigidBody(walls[2].rigidBody);
				resetGame();
				resetGoal();
			   }
			break;

		case 9:
			//Easy Difficulty
			MOVE_TOLERANCE = .05;
			MOVE_MAX = .1;
			BG_R = .2;
			BG_G = .0;
			BG_B = 1.0;
			ENABLE_AMB = 1;
			ENABLE_DL = 1;
			ENABLE_PL = 0;
			ENABLE_SL = 1;
			break;
		case 10:
			//Medium Difficulty
			MOVE_TOLERANCE = .09;
			MOVE_MAX = .2;
			BG_R = .2;
			BG_G = 0.0;
			BG_B = .63;
			ENABLE_AMB = 1;
			ENABLE_DL = 1;
			ENABLE_PL = 0;
			ENABLE_SL = 1;

			break;
		case 11:
			//Hard Difficulty
			MOVE_TOLERANCE = .2;
			MOVE_MAX = .3;
			BG_R = 0.01;
			BG_G = 0.01;
			BG_B = 0.01;
			ENABLE_AMB = 0;
			ENABLE_DL = 0;
			ENABLE_PL = 1;
			ENABLE_SL = 1;

			break;
		case 12:

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
	//Loop through all the balls
	for(int i = 0; i < NUMBER_BALLS; i++)
	   {
		dynamicsWorld->removeRigidBody(balls[i].rigidBody);
		delete balls[i].rigidBody->getMotionState();
		delete balls[i].rigidBody;
		delete balls[i].shape;
	   }

	//Loop through all the walls
	for(int i = 0; i < NUMBER_WALLS; i++)
	   {
		dynamicsWorld->removeRigidBody(walls[i].rigidBody);
		delete walls[i].rigidBody->getMotionState();
		delete walls[i].rigidBody;
		delete walls[i].shape;
	   }

	delete solver;
	delete dispatcher;
	delete collisionConfiguration;
	delete broadphase;
	delete dynamicsWorld;
	glDeleteProgram(program);
   }

// Reads the movement of the mouse on the screen
void mouseMovement(int mouseX, int mouseY)
   {
	if(!PAUSED)
	   {
		//Table Dimensions:
		//Variables
		//float moveThresh = MOUSE_SENSITIVITY;
		float mouseXDirection =    float((mouseX - MOUSE_X));
		float mouseYDirection =    float((mouseY - MOUSE_Y));

		MOUSE_X = w/2;
		MOUSE_Y = h/2;
		if(mouseYDirection != 0)
		   {
			glutWarpPointer(MOUSE_X, MOUSE_Y);

		   }
		if(mouseXDirection != 0)
		   {
			glutWarpPointer(MOUSE_X, MOUSE_Y);
		   }

				//Right Arrow

		      walls[level].rigidBody->setAngularVelocity( btVector3(xpos+= MOVE_TOLERANCE*mouseXDirection/w, ypos, zpos) );
			walls[level].rigidBody->setAngularVelocity( btVector3(xpos, ypos,  zpos+= MOVE_TOLERANCE*mouseYDirection/h) );

	   }

   }

//Writes a bit map string passed to the screen
void writeString( char* line, float x, float y )
   {
	// Set the GL for text rendering
	glMatrixMode(GL_PROJECTION);
	glPushMatrix() ; // save
	glLoadIdentity();// and clear
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	glDisable( GL_DEPTH_TEST ) ; // also disable the depth test so renders on top
	glDisable( GL_TEXTURE_2D );
	glUseProgram(0);
	glRasterPos2f( x, y); 
	glColor3f(1.0f, 0.0f, 0.0f);

   	// Output the text
	for( int x = 0; line[x] != '\0'; x++ )
	   {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, line[x]);
	   }

	// Reestablish GL
	glEnable(GL_DEPTH_TEST); // Turn depth testing back on
	glEnable(GL_TEXTURE_2D);
	glUseProgram(program);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix(); // revert back to the matrix I had before.
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
   }

//Reset the puck, and paddles to default 
void resetGame()
   {
	//Variables
	btTransform trans;
	btScalar m[16];
	timeChange = 0.0;

	// Reset the models
	for(int j = 0; j < NUMBER_BALLS; j++)
	   {
		//Get the matrix location
		balls[j].rigidBody->getMotionState()->getWorldTransform(trans);
		trans.getOpenGLMatrix(m);
		float currentX = m[12];
		float currentY = m[13];
		float currentZ = m[14];

		//Set Back to the defaults
		//Remove the velocities
		btVector3 translateBody( (balls[j].defaultX - currentX) , (balls[j].defaultY - currentY), (balls[j].defaultZ - currentZ));
		balls[j].rigidBody->translate(translateBody);
		balls[j].rigidBody->setLinearVelocity( btVector3( 0.0, 0.0, 0.0 ) );
		balls[j].rigidBody->activate(true);
		balls[j].rigidBody->applyCentralImpulse( btVector3(0.0f, 1.0f, 0.0f) );
	   }

	resetGoal();
   }

// Reset the goal
void resetGoal()
   {
	//Variables
	btTransform trans;
	btScalar m[16];

	// Reset the model
	//Get the matrix location
	walls[level].rigidBody->getMotionState()->getWorldTransform(trans);
	trans.getOpenGLMatrix(m);
	float currentX = m[12];
	float currentY = m[13];
	float currentZ = m[14];

	//Set Back to the defaults
	//Remove the velocities
	btVector3 translateBody( (walls[level].defaultX - currentX) , (walls[level].defaultY - currentY), (walls[level].defaultZ - currentZ));
	walls[level].rigidBody->translate(translateBody);
	walls[level].rigidBody->setLinearVelocity( btVector3( 0.0, 0.0, 0.0 ) );
	walls[level].rigidBody->activate(true);
   }

float calcVelocity(btVector3 vel)
   {
	return (vel[0]*vel[0] + vel[2]*vel[2]);
   }

//Loads the same ball design into the game
bool loadBall()
   {

   	//Load in the ball
	while(NUMBER_BALLS < 10)
	   {
		if(!balls[NUMBER_BALLS].objectMesh.LoadMesh("../bin/objects/ball.obj"))
	   	   {
			std::cerr<<"Ball Failed to Load\n";
			return false;
	   	   }

		btScalar mass = 1.0;
		btVector3 fallInertia(0,1,0);
		balls[NUMBER_BALLS].defaultX = 5.5 - NUMBER_BALLS;
		balls[NUMBER_BALLS].defaultY = 3;
		balls[NUMBER_BALLS].defaultZ = 5.5;
		balls[NUMBER_BALLS].shape = new btSphereShape(0.5);
		balls[NUMBER_BALLS].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(balls[NUMBER_BALLS].defaultX, balls[NUMBER_BALLS].defaultY, balls[NUMBER_BALLS].defaultZ)));
		balls[NUMBER_BALLS].shape->calculateLocalInertia(mass, fallInertia);
		btRigidBody::btRigidBodyConstructionInfo CI(mass, balls[NUMBER_BALLS].motion, balls[NUMBER_BALLS].shape, fallInertia);
		CI.m_restitution = 0.7;
		CI.m_friction = 0.3;
		balls[NUMBER_BALLS].rigidBody = new btRigidBody(CI);
		//dynamicsWorld->addRigidBody(balls[NUMBER_BALLS].rigidBody);

		//Init Struct Members
		btVector3 temp = dynamicsWorld->getGravity();
		balls[NUMBER_WALLS].gravity[0]= temp.x();
		balls[NUMBER_WALLS].gravity[1]= -6.81;
		balls[NUMBER_WALLS].gravity[2]= temp.z();

		//Increment the number of balls
		NUMBER_BALLS++;
	   }
	dynamicsWorld->addRigidBody(balls[0].rigidBody);
	NUMBER_BALLS = 1;


	//Return success
	return true;
   }

//Will read a file, and load the file level specified
bool loadLevel(int level)
   {
	//Set the name of the file to read from
	char buffer[50];
	sprintf(buffer, "level_%d", level);

	//Load in the main board
	walls.resize(3);

	// Lelev 1
	if(!walls[1].objectMesh.LoadMesh("../bin/objects/board10.obj"))
	   {
		std::cerr<<"Main Board Failed to Load\n";
		return false;
	   }

	NUMBER_WALLS++;

	btScalar mass = 1.0;
	btVector3 fallInertia(0, 1, 0);
	walls[1].shape = new btBvhTriangleMeshShape(walls[1].objectMesh.triMesh, false);
	walls[1].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
	walls[1].shape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo CI3(mass, walls[1].motion, walls[1].shape, fallInertia);
	CI3.m_restitution = 0.7;
	walls[1].rigidBody = new btRigidBody(CI3);
	walls[1].rigidBody->setActivationState(DISABLE_DEACTIVATION);
	dynamicsWorld->addRigidBody(walls[1].rigidBody);
	walls[1].rigidBody->setLinearFactor(btVector3(0, 0, 0));
	walls[1].rigidBody->setAngularFactor(btVector3(1, 0, 1));
	walls[1].rigidBody->setDamping(0, 0);
	walls[1].rigidBody->setGravity(btVector3(0, 0, 0));

	// Level 2
	if(!walls[2].objectMesh.LoadMesh("../bin/objects/board9.obj"))
	   {
		std::cerr<<"Main Board Failed to Load\n";
		return false;
	   }

	NUMBER_WALLS++;

	walls[2].shape = new btBvhTriangleMeshShape(walls[2].objectMesh.triMesh, false);
	walls[2].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
	walls[2].shape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo CI(mass, walls[2].motion, walls[2].shape, fallInertia);
	CI.m_restitution = 0.7;
	walls[2].rigidBody = new btRigidBody(CI);
	walls[2].rigidBody->setActivationState(DISABLE_DEACTIVATION);
	walls[2].rigidBody->setLinearFactor(btVector3(0, 0, 0));
	walls[2].rigidBody->setAngularFactor(btVector3(1, 0, 1));
	walls[2].rigidBody->setDamping(0, 0);
	walls[2].rigidBody->setGravity(btVector3(0, 0, 0));

	if(!walls[0].objectMesh.LoadMesh("../bin/objects/ball.obj"))
	   {
		std::cerr<<"Finish Failed to Load\n";
		return false;
	   }

	NUMBER_WALLS++;

	btScalar mass2 = 100.0;
	btVector3 fallInertia2(0,1,0);
	walls[0].defaultX = 5.5;
	walls[0].defaultY = 2;
	walls[0].defaultZ = -5.5;
	walls[0].shape = new btSphereShape(0.5);
	walls[0].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(walls[0].defaultX, walls[0].defaultY, walls[0].defaultZ)));
	walls[0].shape->calculateLocalInertia(mass2, fallInertia2);
	btRigidBody::btRigidBodyConstructionInfo CI2(mass2, walls[0].motion, walls[0].shape, fallInertia2);
	walls[0].rigidBody = new btRigidBody(CI2);
	dynamicsWorld->addRigidBody(walls[0].rigidBody);

	//Return success
	return true;
   }

void lightsEnabled(){

	
	std::cout << "Ambient:" << ENABLE_AMB << std::endl;
	std::cout << "DL:" << ENABLE_DL << std::endl;
	std::cout << "PL:" << ENABLE_PL << std::endl;
	std::cout << "SL:" << ENABLE_SL << std::endl;
}

void updateSpotLights(){
light1x = balls[0].x();
light1y = balls[0].y();
light1z = balls[0].z();

float ret;
ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t0).count();


light2x = sin(ret);
light2y = walls[0].y();
light2z = walls[0].z();
}

void myTickCallback(btDynamicsWorld *world, btScalar timeStep) 
   {
	collision = world->getDispatcher()->getNumManifolds();
   }

