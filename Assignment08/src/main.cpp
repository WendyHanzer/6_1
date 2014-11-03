/*
#OpenGL Air Hockey: CS 680 Graphics Game

###Group Members

- Andy Garcia
- Hardy Thrower
- Nolan Burfield

##Overview

The game will start off with a view of both players in the game and the puck in the center of the board, for best results make the game into a full screen mode. Initially the game will have an AI running the right side, and the player will control the left paddle with keys or mouse. The goal is to get the puck into the opposite side goal and the score will update on the top score board according to the team that got the goal. There will be the option to right click and select different models, and other game functionality. Keys can also be used to move the camera around, reset the puck, and reset the camera to the main view.

##System Requirement

- GLUT and GLEW
- glm
- Assimp
- ImageMagick
- Bullet Physics Library

##User Manual

###Compile and Run Program
	>$ cd build/
	>$ make
	>$ cd ../bin/
	>$ ./airhockey

###Game Play
######Keyboard Controls
- X-Camera (q, w)
- Y-Camera (e, r)
- Z-Camera (t, y)
- X-Focus (a, s)
- Y-Focus (d, f)
- Z-Focus (g, h)

- p will reset the puck
- SPACE will reset the camera to default position

######Paddle Movement
- Left Arrow, Right Arrow, Up Arrow, Down Arrow moves the right paddle accordingly.
- ‘,’ ‘/’ ‘l’ ‘.’ moves theleft paddle accordingly to their location on the keyboard. 
- The mouse will move the right paddle towards the location of the movement.

######Mouse Right Click
- Start Game
- Pause Game
- End Game
- Restart Game
- Switch Paddles
- Switch Pucks
- Switch AI / Two Players


##Tech Manual

The game board renders with multiple triangles on the surface, and they create an unsmooth surface to glide on so the puck sometimes appears to get stuck or bounce around the lines. 

There is an issue with the puck that sometimes gets stuck under the surface of the board with the added friction, and so it falls through but they reset.

The movement of the paddles is also a little jittery if too much force is given to it.
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

//Bullet
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#define BIT(x) (1<<(x))

float calcVelocity(btVector3 vel);

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
   };

// Globals to Hold Accesible OpenGL Stuff
int w = 640, h = 480;
GLuint program;
GLint loc_position;
GLint loc_color;
GLint loc_mvpmat;
glm::mat4 view;
glm::mat4 projection;
Object* hockeyWorld;
int NUMBER_OBJECTS = 0;

// The Bullet World
btBroadphaseInterface* broadphase;
btDefaultCollisionConfiguration* collisionConfiguration;
btCollisionDispatcher* dispatcher;
btSequentialImpulseConstraintSolver* solver;
btDiscreteDynamicsWorld* dynamicsWorld;

//For the Camera
float X_VIEW = 0.0, Y_VIEW = 9.0, Z_VIEW = -9.0;
float X_FOCUS = 0.0, Y_FOCUS = 0.0, Z_FOCUS = 0.0;
float X_FOCUS_ADJ = 0, Y_FOCUS_ADJ = 0, Z_FOCUS_ADJ = 0;
float X_VIEW_ADJ= 0, Y_VIEW_ADJ= 11.0, Z_VIEW_ADJ= 0;

// Enums
// Which Paddle is active
enum Paddle
   {
	TRIANGLE_PADDLE_R = 0, 
	TRIANGLE_PADDLE_L = 1, 
	CIRCLE_PADDLE_R = 2,
	CIRCLE_PADDLE_L = 3,
	SQUARE_PADDLE_R = 4,
	SQUARE_PADDLE_L = 5
   };

// Which Puck is Active
enum Puck
   {
	TRIANGLE_PUCK = 6,
	CIRCLE_PUCK = 7,
	STAR_PUCK = 8
   };

// What stuff collides with others
enum Collisions
   {
	NONE = 0,
	BOUND_BOX = BIT(0),
	PADDLE = BIT(1),
	PUCK = BIT(2),
	TABLE = BIT(3)
   };

//Game Controls
bool GAME_STARTED = false;
bool PAUSED = false;
btVector3 PUCK_V;
btVector3 PADDLE_L_V;
btVector3 PADDLE_R_V;
btVector3 ZEROS(0,0,0);

//For Mouse Controls
int MOUSE_X, MOUSE_Y;
float MOUSE_SENSITIVITY = .5;
float calcVelocity(btVector3 vel);
int CURSOR_VISIBLE = 0;

//Score
int LEFT_SCORE = 0, RIGHT_SCORE = 0; 

//Current puck and paddle
int CURRENT_PUCK = CIRCLE_PUCK;
int CURRENT_PADDLE_R = CIRCLE_PADDLE_R;
int CURRENT_PADDLE_L = CIRCLE_PADDLE_L;
bool AI_PLAYER = false;

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void menu(int id);
void keyboardSpecial(int key, int x_pos, int y_pos);
void mouseMovement(int mouseX, int mouseY);

//Artificial Intelligence
void updateAI();

//--Function Specs
bool initialize();
void myKey(unsigned char key, int x, int y);
glm::mat4 getView();
bool load(const char*);
void writeString(char* line);
void cleanUp();
void resetGame();

//--Time Keeping
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;
float getDT();

//--Main
int main(int argc, char **argv)
   {
	// Initialize glut
	Magick::InitializeMagick(*argv);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(w, h);

	// Name and create the Window
	glutCreateWindow("Air Hockey");

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
	glutCreateMenu(menu);
		glutAddMenuEntry("Quit", 1);
		glutAddMenuEntry("Start", 2);
		glutAddMenuEntry("Pause", 3);
		glutAddMenuEntry("Restart", 4);
		glutAddMenuEntry("Square Paddles", 5);
		glutAddMenuEntry("Triangle Paddles", 6);
		glutAddMenuEntry("Circle Paddles", 7);
		glutAddMenuEntry("Triangle Puck", 8);
		glutAddMenuEntry("Star Puck", 9);
		glutAddMenuEntry("Circle Puck", 10);
		glutAddMenuEntry("AI", 11);
		glutAddMenuEntry("Two Player", 12);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	//Begin the program
	bool init = initialize();
	if( init )
	   {
		t1 = std::chrono::high_resolution_clock::now();
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
	glClearColor(1.0, 0.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//enable the shader program
	glUseProgram(program);

	//Get the time change
	static float timeChange = 0.0;
	float dt = getDT();
	timeChange += dt * M_PI/2;

	//Update the current world
	dynamicsWorld->stepSimulation(1.f/60.f, 1);

	//Loop through all the objects to output
	for(int i = 0; i < NUMBER_OBJECTS-2; i++)
	   {
	   	//Check for the object to be active
	   	if(i == CURRENT_PUCK || i == CURRENT_PADDLE_L || i == CURRENT_PADDLE_R || i == 9)
	   	   {
			//Variables
			btTransform trans;
			btScalar m[16];

			//Get the new transformation
			hockeyWorld[i].rigidBody->getMotionState()->getWorldTransform(trans);

			//Output Location
			//std::cout<<"Model " << i << " Y: "<< trans.getOrigin().getY() << " X: " << trans.getOrigin().getX() << " Z: " << trans.getOrigin().getZ() << std::endl;

			//Set up the new model
			trans.getOpenGLMatrix(m);

			if(trans.getOrigin().getY() < (hockeyWorld[CURRENT_PUCK].defaultY - 1.0) && i == CURRENT_PUCK)
			   {
				//Check which side the puck was on and if it was in bounds
				std::cout<<"Model " << i << " Y: "<< trans.getOrigin().getY() << " X: " << trans.getOrigin().getX() << " Z: " << trans.getOrigin().getZ() << std::endl;
				bool xLeftScore = trans.getOrigin().getX() < -13 && trans.getOrigin().getX() > -15;
				bool xRightScore = trans.getOrigin().getX() < 14.5 && trans.getOrigin().getX() > 11.5;
				bool zScore = trans.getOrigin().getZ() > -1.5 && trans.getOrigin().getZ() < 1.5;
				if( xLeftScore && zScore )
				   {
					LEFT_SCORE++;
				   }
				else if( xRightScore && zScore )
				   {
					RIGHT_SCORE++;
				   }

				resetGame();

				// Print the score
				printf("Left Score: %d          Right Score: %d \n", LEFT_SCORE, RIGHT_SCORE);
			   }

			glm::vec4 one = glm::vec4(m[0], m[1], m[2], m[3]);
			glm::vec4 two = glm::vec4(m[4], m[5], m[6], m[7]);
			glm::vec4 three = glm::vec4(m[8], m[9], m[10], m[11]);
			glm::vec4 four = glm::vec4(m[12], m[13], m[14], m[15]);
			glm::mat4 temp = glm::mat4(one, two, three, four);

			hockeyWorld[i].model = temp;

			//Set the mvp in the object
			hockeyWorld[i].mvp = projection * view * hockeyWorld[i].model;

			//upload the matrix to the shader
			glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(hockeyWorld[i].mvp));

			// Render Objects
			hockeyWorld[i].objectMesh.Render(loc_position, loc_color);
	       }
	   }

	//Setup and output the scores
	char buffer[250];
	sprintf(buffer, "SCOREBOARD     LEFT: %d         RIGHT: %d", LEFT_SCORE, RIGHT_SCORE);
	writeString(buffer);

	//swap the buffers
	glutSwapBuffers();
   }

//Move the objects through time, and update the models
void update()
   {	   
	//UPdate the camers
	view = getView();
	
	//UpdateAI
	if(AI_PLAYER && GAME_STARTED && (!PAUSED))
	{
		updateAI();
	}	




		//Variables
	btTransform trans;
	btScalar m[16];

   	//Get the matrix location
	hockeyWorld[CURRENT_PUCK].rigidBody->getMotionState()->getWorldTransform(trans);
	trans.getOpenGLMatrix(m);
	float currentY = m[13];
	



	if(  currentY > .7 )
	{
		hockeyWorld[CURRENT_PUCK].rigidBody->applyCentralImpulse( btVector3( 0.0f, currentY * currentY * -1, 0.0f ) );
	}
	else
	{
		hockeyWorld[CURRENT_PUCK].rigidBody->applyCentralImpulse( btVector3( 0.0f, 0.5f, 0.0f ) );
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
//printf("CamerA Location: %f, %f, %f", X_VIEW_ADJ, Y_VIEW_ADJ, Z_VIEW_ADJ);
	//Left Move
	if(key == ',')
	   {
		hockeyWorld[CURRENT_PADDLE_L].rigidBody->activate(true);
		hockeyWorld[CURRENT_PADDLE_L].rigidBody->applyCentralImpulse( btVector3(60.0f, 0.0f, 0.0f) );
	   }

	//Right Move
	if(key == '/')
	   {
		hockeyWorld[CURRENT_PADDLE_L].rigidBody->activate(true);
		hockeyWorld[CURRENT_PADDLE_L].rigidBody->applyCentralImpulse( btVector3(-60.0f, 0.0f, 0.0f) );
	   }

	//Up Move
	if(key == 'l')
	   {
		hockeyWorld[CURRENT_PADDLE_L].rigidBody->activate(true);
		hockeyWorld[CURRENT_PADDLE_L].rigidBody->applyCentralImpulse( btVector3(0.0f, 0.0f, 60.0f) );
	   }

	//Down Move
	if(key == '.')
	   {
		hockeyWorld[CURRENT_PADDLE_L].rigidBody->activate(true);
		hockeyWorld[CURRENT_PADDLE_L].rigidBody->applyCentralImpulse( btVector3(0.0f, 0.0f, -60.0f) );
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
   }

//Initilizing the program called by main to set the shaders, objects, and bullet
bool initialize()
   {
    //Init Bullet
	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	solver = new btSequentialImpulseConstraintSolver;
	int boundBox = PADDLE,
		puck = PADDLE | TABLE,
		paddle = BOUND_BOX | TABLE | PUCK,
		table = PADDLE | PUCK;

	// Putting Bullet Together
	dynamicsWorld = new btDiscreteDynamicsWorld( dispatcher, broadphase, solver, collisionConfiguration);

	// Activate Gravity
	dynamicsWorld->setGravity(btVector3(0, -9.81, 0));

	//Allocat the memory for the world
	NUMBER_OBJECTS = 12;
	hockeyWorld = new Object[NUMBER_OBJECTS];

	//Get the files
	//Circle Paddle
	bool check1 = hockeyWorld[CIRCLE_PADDLE_R].objectMesh.LoadMesh("objects/Paddle.obj");
		 check1 = hockeyWorld[CIRCLE_PADDLE_L].objectMesh.LoadMesh("objects/Paddle.obj") && check1;
	// Square Paddle
		 check1 = hockeyWorld[SQUARE_PADDLE_R].objectMesh.LoadMesh("objects/Paddle_Square.obj") && check1;
		 check1 = hockeyWorld[SQUARE_PADDLE_L].objectMesh.LoadMesh("objects/Paddle_Square.obj") && check1;
	// Triangle Paddle
		 check1 = hockeyWorld[TRIANGLE_PADDLE_R].objectMesh.LoadMesh("objects/Paddle_Triangle2.obj") && check1;
		 check1 = hockeyWorld[TRIANGLE_PADDLE_L].objectMesh.LoadMesh("objects/Paddle_Triangle2.obj") && check1;
	// Circle Puck
		 check1 = hockeyWorld[CIRCLE_PUCK].objectMesh.LoadMesh("objects/Puck4.obj") && check1;
	// Trianlge Puck
		 check1 = hockeyWorld[TRIANGLE_PUCK].objectMesh.LoadMesh("objects/tri_puck.obj") && check1;
	// Star Puck
		 check1 = hockeyWorld[STAR_PUCK].objectMesh.LoadMesh("objects/star_puck.obj") && check1;
	// Table
		 check1 = hockeyWorld[9].objectMesh.LoadMesh("objects/Airhockey5.obj") && check1;

	//Check if the files have loaded in
	if(!check1)
	   {
		std::cerr << "Objects failed to read in."<<std::endl;
		return false;
	   }

	//Create the bounding box
	hockeyWorld[10].shape = new btBoxShape(btVector3(0.05, 2, 10));
	hockeyWorld[10].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
	btRigidBody::btRigidBodyConstructionInfo elevenCI(0, hockeyWorld[10].motion, hockeyWorld[10].shape, btVector3(0, 0, 0));
	hockeyWorld[10].rigidBody = new btRigidBody(elevenCI);
	dynamicsWorld->addRigidBody(hockeyWorld[10].rigidBody, BOUND_BOX, boundBox);

	// Create a ground (ground plane "a static rigid body)
	hockeyWorld[11].shape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
	hockeyWorld[11].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
	btRigidBody::btRigidBodyConstructionInfo oneCI(0, hockeyWorld[11].motion, hockeyWorld[11].shape, btVector3(0, 0, 0));
	hockeyWorld[11].rigidBody = new btRigidBody(oneCI);
	dynamicsWorld->addRigidBody(hockeyWorld[11].rigidBody);

	// Table	
	btScalar mass = 0.0;
	btVector3 fallInertia(0, 0, 0);
	hockeyWorld[9].defaultX = 0.0;
	hockeyWorld[9].defaultY = 1.0;
	hockeyWorld[9].defaultZ = 0.0;
	hockeyWorld[9].shape = new btBvhTriangleMeshShape(hockeyWorld[9].objectMesh.triMesh, true);
	hockeyWorld[9].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(hockeyWorld[9].defaultX, hockeyWorld[9].defaultY, hockeyWorld[9].defaultZ)));
	btRigidBody::btRigidBodyConstructionInfo CI(mass, hockeyWorld[9].motion, hockeyWorld[9].shape, fallInertia);

	CI.m_friction = .00;

	hockeyWorld[9].rigidBody = new btRigidBody(CI);
	dynamicsWorld->addRigidBody(hockeyWorld[9].rigidBody, TABLE, table);

	// Circle Paddle
	btScalar mass2 = 60.0;
	btVector3 fallInertia2(0, 1, 0);
	hockeyWorld[CIRCLE_PADDLE_L].defaultX = 11.0;
	hockeyWorld[CIRCLE_PADDLE_L].defaultY = 0.738;
	hockeyWorld[CIRCLE_PADDLE_L].defaultZ = 1.0;
	hockeyWorld[CIRCLE_PADDLE_L].shape = new btConvexTriangleMeshShape(hockeyWorld[CIRCLE_PADDLE_L].objectMesh.triMesh, true);
	hockeyWorld[CIRCLE_PADDLE_L].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(hockeyWorld[CIRCLE_PADDLE_L].defaultX, hockeyWorld[CIRCLE_PADDLE_L].defaultY, hockeyWorld[CIRCLE_PADDLE_L].defaultZ)));
	hockeyWorld[CIRCLE_PADDLE_L].shape->calculateLocalInertia(mass2, fallInertia2);
	btRigidBody::btRigidBodyConstructionInfo sevenCI(mass2, hockeyWorld[CIRCLE_PADDLE_L].motion, hockeyWorld[CIRCLE_PADDLE_L].shape, fallInertia2);
	hockeyWorld[CIRCLE_PADDLE_L].rigidBody = new btRigidBody(sevenCI);
	hockeyWorld[CIRCLE_PADDLE_L].rigidBody->setLinearFactor(btVector3(1, 0, 1));
	
	dynamicsWorld->addRigidBody(hockeyWorld[CIRCLE_PADDLE_L].rigidBody, PADDLE, paddle);
	

	// Circle Paddle 2
	btScalar mass3 = 60.0;
	btVector3 fallInertia3(0, 0, 0);
	hockeyWorld[CIRCLE_PADDLE_R].defaultX = -10.0;
	hockeyWorld[CIRCLE_PADDLE_R].defaultY = 0.738;
	hockeyWorld[CIRCLE_PADDLE_R].defaultZ = 1.0;
	hockeyWorld[CIRCLE_PADDLE_R].shape = new btConvexTriangleMeshShape(hockeyWorld[CIRCLE_PADDLE_R].objectMesh.triMesh, true);
	hockeyWorld[CIRCLE_PADDLE_R].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(hockeyWorld[CIRCLE_PADDLE_R].defaultX, hockeyWorld[CIRCLE_PADDLE_R].defaultY, hockeyWorld[CIRCLE_PADDLE_R].defaultZ)));
	hockeyWorld[CIRCLE_PADDLE_R].shape->calculateLocalInertia(mass3, fallInertia3);
	btRigidBody::btRigidBodyConstructionInfo eightCI(mass2, hockeyWorld[CIRCLE_PADDLE_R].motion, hockeyWorld[CIRCLE_PADDLE_R].shape, fallInertia3);
	hockeyWorld[CIRCLE_PADDLE_R].rigidBody = new btRigidBody(eightCI);
	hockeyWorld[CIRCLE_PADDLE_R].rigidBody->setLinearFactor(btVector3(1, 0, 1));
	hockeyWorld[CIRCLE_PADDLE_R].rigidBody->setActivationState(DISABLE_DEACTIVATION);
	dynamicsWorld->addRigidBody(hockeyWorld[CIRCLE_PADDLE_R].rigidBody, PADDLE, paddle);

	// Square Paddle
	hockeyWorld[SQUARE_PADDLE_L].defaultX = 11.0;
	hockeyWorld[SQUARE_PADDLE_L].defaultY = 0.738;
	hockeyWorld[SQUARE_PADDLE_L].defaultZ = 1.0;
	hockeyWorld[SQUARE_PADDLE_L].shape = new btConvexTriangleMeshShape(hockeyWorld[SQUARE_PADDLE_L].objectMesh.triMesh, true);
	hockeyWorld[SQUARE_PADDLE_L].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(hockeyWorld[SQUARE_PADDLE_L].defaultX, hockeyWorld[SQUARE_PADDLE_L].defaultY, hockeyWorld[SQUARE_PADDLE_L].defaultZ)));
	hockeyWorld[SQUARE_PADDLE_L].shape->calculateLocalInertia(mass2, fallInertia2);
	btRigidBody::btRigidBodyConstructionInfo seven2CI(mass2, hockeyWorld[SQUARE_PADDLE_L].motion, hockeyWorld[SQUARE_PADDLE_L].shape, fallInertia2);
	hockeyWorld[SQUARE_PADDLE_L].rigidBody = new btRigidBody(seven2CI);
	hockeyWorld[SQUARE_PADDLE_L].rigidBody->setLinearFactor(btVector3(1, 0, 1));

	// Square Paddle 2
	hockeyWorld[SQUARE_PADDLE_R].defaultX = -10.0;
	hockeyWorld[SQUARE_PADDLE_R].defaultY = 0.738;
	hockeyWorld[SQUARE_PADDLE_R].defaultZ = 1.0;
	hockeyWorld[SQUARE_PADDLE_R].shape = new btConvexTriangleMeshShape(hockeyWorld[SQUARE_PADDLE_R].objectMesh.triMesh, true);
	hockeyWorld[SQUARE_PADDLE_R].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(hockeyWorld[SQUARE_PADDLE_R].defaultX, hockeyWorld[SQUARE_PADDLE_R].defaultY, hockeyWorld[SQUARE_PADDLE_R].defaultZ)));
	hockeyWorld[SQUARE_PADDLE_R].shape->calculateLocalInertia(mass3, fallInertia3);
	btRigidBody::btRigidBodyConstructionInfo eight2CI(mass2, hockeyWorld[SQUARE_PADDLE_R].motion, hockeyWorld[SQUARE_PADDLE_R].shape, fallInertia3);
	hockeyWorld[SQUARE_PADDLE_R].rigidBody = new btRigidBody(eight2CI);
	hockeyWorld[SQUARE_PADDLE_R].rigidBody->setLinearFactor(btVector3(1, 0, 1));
	hockeyWorld[SQUARE_PADDLE_R].rigidBody->setActivationState(DISABLE_DEACTIVATION);

	// Triangle Paddle
	hockeyWorld[TRIANGLE_PADDLE_L].defaultX = 11.0;
	hockeyWorld[TRIANGLE_PADDLE_L].defaultY = 0.738;
	hockeyWorld[TRIANGLE_PADDLE_L].defaultZ = 1.0;
	hockeyWorld[TRIANGLE_PADDLE_L].shape = new btConvexTriangleMeshShape(hockeyWorld[TRIANGLE_PADDLE_L].objectMesh.triMesh, true);
	hockeyWorld[TRIANGLE_PADDLE_L].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(hockeyWorld[TRIANGLE_PADDLE_L].defaultX, hockeyWorld[TRIANGLE_PADDLE_L].defaultY, hockeyWorld[TRIANGLE_PADDLE_L].defaultZ)));
	hockeyWorld[TRIANGLE_PADDLE_L].shape->calculateLocalInertia(mass2, fallInertia2);
	btRigidBody::btRigidBodyConstructionInfo seven3CI(mass2, hockeyWorld[TRIANGLE_PADDLE_L].motion, hockeyWorld[TRIANGLE_PADDLE_L].shape, fallInertia2);
	hockeyWorld[TRIANGLE_PADDLE_L].rigidBody = new btRigidBody(seven3CI);
	hockeyWorld[TRIANGLE_PADDLE_L].rigidBody->setLinearFactor(btVector3(1, 0, 1));

	// Triangle Paddle 2
	hockeyWorld[TRIANGLE_PADDLE_R].defaultX = -10.0;
	hockeyWorld[TRIANGLE_PADDLE_R].defaultY = 0.738;
	hockeyWorld[TRIANGLE_PADDLE_R].defaultZ = 1.0;
	hockeyWorld[TRIANGLE_PADDLE_R].shape = new btConvexTriangleMeshShape(hockeyWorld[TRIANGLE_PADDLE_R].objectMesh.triMesh, true);
	hockeyWorld[TRIANGLE_PADDLE_R].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(hockeyWorld[TRIANGLE_PADDLE_R].defaultX, hockeyWorld[TRIANGLE_PADDLE_R].defaultY, hockeyWorld[TRIANGLE_PADDLE_R].defaultZ)));
	hockeyWorld[TRIANGLE_PADDLE_R].shape->calculateLocalInertia(mass3, fallInertia3);
	btRigidBody::btRigidBodyConstructionInfo eight3CI(mass2, hockeyWorld[TRIANGLE_PADDLE_R].motion, hockeyWorld[TRIANGLE_PADDLE_R].shape, fallInertia3);
	hockeyWorld[TRIANGLE_PADDLE_R].rigidBody = new btRigidBody(eight3CI);
	hockeyWorld[TRIANGLE_PADDLE_R].rigidBody->setLinearFactor(btVector3(1, 0, 1));
	hockeyWorld[TRIANGLE_PADDLE_R].rigidBody->setActivationState(DISABLE_DEACTIVATION);

	// Circle Puck	
	btScalar mass4 = 18.0;
	btVector3 fallInertia4(0, 0, 0);
	hockeyWorld[CIRCLE_PUCK].defaultX = 1.0;
	hockeyWorld[CIRCLE_PUCK].defaultY = 1.0;
	hockeyWorld[CIRCLE_PUCK].defaultZ = 1.0;
	hockeyWorld[CIRCLE_PUCK].shape = new btConvexTriangleMeshShape(hockeyWorld[CIRCLE_PUCK].objectMesh.triMesh, true);
	hockeyWorld[CIRCLE_PUCK].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(hockeyWorld[CIRCLE_PUCK].defaultX, hockeyWorld[CIRCLE_PUCK].defaultY, hockeyWorld[CIRCLE_PUCK].defaultZ)));
	hockeyWorld[CIRCLE_PUCK].shape->calculateLocalInertia(mass4, fallInertia4);
	btRigidBody::btRigidBodyConstructionInfo nineCI(mass4, hockeyWorld[CIRCLE_PUCK].motion, hockeyWorld[CIRCLE_PUCK].shape, fallInertia4);
	//nineCI.m_restitution = 0.0f;

	hockeyWorld[CIRCLE_PUCK].rigidBody = new btRigidBody(nineCI);
	hockeyWorld[CIRCLE_PUCK].rigidBody->setCcdMotionThreshold(2);
	hockeyWorld[CIRCLE_PUCK].rigidBody->setLinearFactor(btVector3(1, 0.4, 1));
	dynamicsWorld->addRigidBody(hockeyWorld[CIRCLE_PUCK].rigidBody, PUCK, puck);

	// Triangle Puck	
	hockeyWorld[TRIANGLE_PUCK].defaultX = 1.0;
	hockeyWorld[TRIANGLE_PUCK].defaultY = 1.0;
	hockeyWorld[TRIANGLE_PUCK].defaultZ = 1.0;
	hockeyWorld[TRIANGLE_PUCK].shape = new btConvexTriangleMeshShape(hockeyWorld[TRIANGLE_PUCK].objectMesh.triMesh, true);
	hockeyWorld[TRIANGLE_PUCK].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(hockeyWorld[TRIANGLE_PUCK].defaultX, hockeyWorld[TRIANGLE_PUCK].defaultY, hockeyWorld[TRIANGLE_PUCK].defaultZ)));
	hockeyWorld[TRIANGLE_PUCK].shape->calculateLocalInertia(mass4, fallInertia4);
	btRigidBody::btRigidBodyConstructionInfo nine2CI(mass4, hockeyWorld[TRIANGLE_PUCK].motion, hockeyWorld[TRIANGLE_PUCK].shape, fallInertia4);
	//nine2CI.m_restitution = 1.5f;

	hockeyWorld[TRIANGLE_PUCK].rigidBody = new btRigidBody(nine2CI);
	hockeyWorld[TRIANGLE_PUCK].rigidBody->setCcdMotionThreshold(2);
	hockeyWorld[TRIANGLE_PUCK].rigidBody->setLinearFactor(btVector3(1, 0.4, 1));

	// Star Puck	
	hockeyWorld[STAR_PUCK].defaultX = 1.0;
	hockeyWorld[STAR_PUCK].defaultY = 1.0;
	hockeyWorld[STAR_PUCK].defaultZ = 1.0;
	hockeyWorld[STAR_PUCK].shape = new btConvexTriangleMeshShape(hockeyWorld[STAR_PUCK].objectMesh.triMesh, true);
	hockeyWorld[STAR_PUCK].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(hockeyWorld[STAR_PUCK].defaultX, hockeyWorld[STAR_PUCK].defaultY, hockeyWorld[STAR_PUCK].defaultZ)));
	hockeyWorld[STAR_PUCK].shape->calculateLocalInertia(mass4, fallInertia4);
	btRigidBody::btRigidBodyConstructionInfo nine3CI(mass4, hockeyWorld[STAR_PUCK].motion, hockeyWorld[STAR_PUCK].shape, fallInertia4);
	//nine3CI.m_restitution = 1.5f;

	hockeyWorld[STAR_PUCK].rigidBody = new btRigidBody(nine3CI);
	hockeyWorld[STAR_PUCK].rigidBody->setCcdMotionThreshold(2);
	hockeyWorld[STAR_PUCK].rigidBody->setLinearFactor(btVector3(1, 0.4, 1));

	//Create Shader
	Shader vertexShader, fragmentShader;
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	//Set the shader source for compiling
	vertexShader.readFile("vertexshader.glsl");
	const char* vertexSource = vertexShader.getShaderSource();
	fragmentShader.readFile("fragmentshader.glsl");
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
	loc_color = glGetAttribLocation(program, const_cast<const char*>("v_uv"));
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
	if(!AI_PLAYER)
	{
	//Left Arrow
	if(key == GLUT_KEY_LEFT)
	   {

		hockeyWorld[CURRENT_PADDLE_R].rigidBody->activate(true);
		hockeyWorld[CURRENT_PADDLE_R].rigidBody->applyCentralImpulse( btVector3(80.0f, 0.5f, 0.0f) );


	   }

	//Right Arrow
	if(key == GLUT_KEY_RIGHT)
	   {

		hockeyWorld[CURRENT_PADDLE_R].rigidBody->activate(true);
		hockeyWorld[CURRENT_PADDLE_R].rigidBody->applyCentralImpulse( btVector3(-80.0f, 0.5f, 0.0f) );

	   }

	//Up Arrow
	if(key == GLUT_KEY_UP)
	   {

		hockeyWorld[CURRENT_PADDLE_R].rigidBody->activate(true);
		hockeyWorld[CURRENT_PADDLE_R].rigidBody->applyCentralImpulse( btVector3(0.0f, 0.5f, 80.0f) );

	   }

	//Down Arrow
	if(key == GLUT_KEY_DOWN)
	   {
		hockeyWorld[CURRENT_PADDLE_R].rigidBody->activate(true);
		hockeyWorld[CURRENT_PADDLE_R].rigidBody->applyCentralImpulse( btVector3(0.0f, 0.0f, -80.0f) );
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

   		glutAddMenuEntry("Quit", 1);
		glutAddMenuEntry("Start", 2);
		glutAddMenuEntry("Pause", 3);
		glutAddMenuEntry("Restart", 4);
		glutAddMenuEntry("Square Paddles", 5);
		glutAddMenuEntry("Triangle Paddles", 6);
		glutAddMenuEntry("Circle Paddles", 7);
		glutAddMenuEntry("Square Puck", 8);
		glutAddMenuEntry("Star Puck", 9);
		glutAddMenuEntry("Circle Puck", 10);
		glutAddMenuEntry("AI", 11);
		glutAddMenuEntry("Two Player", 12);
    */

    //Variables
	int puck = PADDLE | TABLE,
		paddle = BOUND_BOX | TABLE | PUCK;
	
	//switch on id
	switch (id) 
	   {
		//Escape
		case 1:
			exit(0);
			break;
		case 2:
		 	GAME_STARTED = 1;
			break;
		case 3:
			if(PAUSED == 0)
			{
				hockeyWorld[CURRENT_PADDLE_L].rigidBody->setLinearVelocity(ZEROS);	
				hockeyWorld[CURRENT_PADDLE_R].rigidBody->setLinearVelocity(ZEROS);
				hockeyWorld[CURRENT_PUCK].rigidBody->setLinearVelocity(ZEROS);
	
				PUCK_V = hockeyWorld[CURRENT_PADDLE_L].rigidBody->getLinearVelocity();
				PADDLE_R_V = hockeyWorld[CURRENT_PADDLE_R].rigidBody->getLinearVelocity();
				PADDLE_L_V = hockeyWorld[CURRENT_PUCK].rigidBody->getLinearVelocity();
				PAUSED = 1;
			}
			else
			{
				hockeyWorld[CURRENT_PADDLE_L].rigidBody->setLinearVelocity(PADDLE_L_V);	
				hockeyWorld[CURRENT_PADDLE_R].rigidBody->setLinearVelocity(PADDLE_R_V);
				hockeyWorld[CURRENT_PUCK].rigidBody->setLinearVelocity(PUCK_V);
				PAUSED = 0;
			}
	
			break;
		case 4:
			LEFT_SCORE = RIGHT_SCORE = 0;
			resetGame();
			break;
		case 5:
			dynamicsWorld->removeRigidBody(hockeyWorld[CURRENT_PADDLE_L].rigidBody);
			CURRENT_PADDLE_L = SQUARE_PADDLE_L;
			dynamicsWorld->addRigidBody(hockeyWorld[CURRENT_PADDLE_L].rigidBody, PADDLE, paddle);
			dynamicsWorld->removeRigidBody(hockeyWorld[CURRENT_PADDLE_R].rigidBody);
			CURRENT_PADDLE_R = SQUARE_PADDLE_R;
			dynamicsWorld->addRigidBody(hockeyWorld[CURRENT_PADDLE_R].rigidBody, PADDLE, paddle);
			resetGame();
			break;
		case 6:
			dynamicsWorld->removeRigidBody(hockeyWorld[CURRENT_PADDLE_L].rigidBody);
			CURRENT_PADDLE_L = TRIANGLE_PADDLE_L;
			dynamicsWorld->addRigidBody(hockeyWorld[CURRENT_PADDLE_L].rigidBody, PADDLE, paddle);
			dynamicsWorld->removeRigidBody(hockeyWorld[CURRENT_PADDLE_R].rigidBody);
			CURRENT_PADDLE_R = TRIANGLE_PADDLE_R;
			dynamicsWorld->addRigidBody(hockeyWorld[CURRENT_PADDLE_R].rigidBody, PADDLE, paddle);
			resetGame();
			break;
		case 7:
			dynamicsWorld->removeRigidBody(hockeyWorld[CURRENT_PADDLE_L].rigidBody);
			CURRENT_PADDLE_L = CIRCLE_PADDLE_L;
			dynamicsWorld->addRigidBody(hockeyWorld[CURRENT_PADDLE_L].rigidBody, PADDLE, paddle);
			dynamicsWorld->removeRigidBody(hockeyWorld[CURRENT_PADDLE_R].rigidBody);
			CURRENT_PADDLE_R = CIRCLE_PADDLE_R;
			dynamicsWorld->addRigidBody(hockeyWorld[CURRENT_PADDLE_R].rigidBody, PADDLE, paddle);
			resetGame();
			break;
		case 8:
			dynamicsWorld->removeRigidBody(hockeyWorld[CURRENT_PUCK].rigidBody);
			CURRENT_PUCK = TRIANGLE_PUCK;
			dynamicsWorld->addRigidBody(hockeyWorld[TRIANGLE_PUCK].rigidBody, PUCK, puck);
			resetGame();
			break;
		case 9:
			dynamicsWorld->removeRigidBody(hockeyWorld[CURRENT_PUCK].rigidBody);
			CURRENT_PUCK = STAR_PUCK;
			dynamicsWorld->addRigidBody(hockeyWorld[STAR_PUCK].rigidBody, PUCK, puck);
			resetGame();
			break;
		case 10:
			dynamicsWorld->removeRigidBody(hockeyWorld[CURRENT_PUCK].rigidBody);
			CURRENT_PUCK = CIRCLE_PUCK;
			dynamicsWorld->addRigidBody(hockeyWorld[CIRCLE_PUCK].rigidBody, PUCK, puck);
			resetGame();
			break;
		case 11:
			AI_PLAYER = true;
			hockeyWorld[CURRENT_PADDLE_R].rigidBody->applyCentralImpulse( btVector3(2.0f, 12.0f, 2.0f) );
			break;
		case 12:
			AI_PLAYER = false;
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
   	//Loop through all the objects
   	for(int i = 0; i < NUMBER_OBJECTS; i++)
   	   {
   	   	dynamicsWorld->removeRigidBody(hockeyWorld[i].rigidBody);
   	   	delete hockeyWorld[i].rigidBody->getMotionState();
   	   	delete hockeyWorld[i].rigidBody;
   	   	delete hockeyWorld[i].shape;
   	   }

   	delete []hockeyWorld;
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
	if(GAME_STARTED && !(PAUSED))
	   {
	   	//Table Dimensions: 15.69, 5.98, 8.913
	   	//Variables
	   	float moveThresh = MOUSE_SENSITIVITY;
		float mouseXDirection =  -1 * float((mouseX - MOUSE_X));
		float mouseYDirection =  -1 * float((mouseY - MOUSE_Y));

		if(calcVelocity( hockeyWorld[CURRENT_PADDLE_L].rigidBody->getLinearVelocity()) > 200.0f)
		   {
		   	moveThresh = 2;
			std::cout << "here: " << calcVelocity( hockeyWorld[CURRENT_PADDLE_L].rigidBody->getLinearVelocity()) <<   std::endl;
		   }


	   	//Add the force to the paddle
		hockeyWorld[CURRENT_PADDLE_L].rigidBody->activate(true);
		hockeyWorld[CURRENT_PADDLE_L].rigidBody->applyCentralImpulse( btVector3( mouseXDirection/moveThresh, 1.0f, mouseYDirection/moveThresh ) );


		std::cout << "Mouse = " << mouseXDirection/moveThresh << " , "<< mouseYDirection/moveThresh << std::endl;

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
	   }

   }

//Writes a bit map string passed to the screen
void writeString( char* line )
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
	glRasterPos2f( -0.30f, 0.95f ) ; // center of screen. (-1,0) is center left.
	glColor3f(0.0f, 0.0f, 1.0f);

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

	// Reset the models
	for(int j = 0; j < NUMBER_OBJECTS; j++)
	   {
	   	if(j == CURRENT_PUCK || j == CURRENT_PADDLE_L || j == CURRENT_PADDLE_R)
	   	   {
		   	//Get the matrix location
			hockeyWorld[j].rigidBody->getMotionState()->getWorldTransform(trans);
			trans.getOpenGLMatrix(m);
			float currentX = m[12];
			float currentY = m[13];
			float currentZ = m[14];

			//Set Back to the defaults
			//Remove the velocities
			btVector3 translateBody( (hockeyWorld[j].defaultX - currentX) , (hockeyWorld[j].defaultY - currentY), (hockeyWorld[j].defaultZ - currentZ));
			hockeyWorld[j].rigidBody->translate(translateBody);
                        hockeyWorld[j].rigidBody->setLinearVelocity( btVector3( 0.0, 0.0, 0.0 ) );
                        hockeyWorld[j].rigidBody->activate(true);
                        hockeyWorld[j].rigidBody->applyCentralImpulse( btVector3(0.0f, 12.0f, 0.0f) );
		   }
	   }
   }

void updateAI(){

		btVector3 linearPuckV = hockeyWorld[CURRENT_PUCK].rigidBody->getLinearVelocity();
		btVector3 linearAIV = hockeyWorld[CURRENT_PADDLE_R].rigidBody->getLinearVelocity();

		hockeyWorld[CURRENT_PUCK].rigidBody->getLinearVelocity();
		btTransform transAI;
		btTransform transPuck;
		hockeyWorld[CURRENT_PADDLE_R].rigidBody->getMotionState()->getWorldTransform(transAI);
		hockeyWorld[CURRENT_PUCK].rigidBody->getMotionState()->getWorldTransform(transPuck);
		btScalar m[16];
		btScalar n[16];


		
		//Set up the new model
		transAI.getOpenGLMatrix(m);
		transPuck.getOpenGLMatrix(n);
	//If On his side
	if( n[12] < 1)
	{
		if(linearPuckV[0] > 0 && linearPuckV[0] < 1 && (n[12] - m[12]) < .5 && (n[12] - m[12]) > .5 ) 
		{
			hockeyWorld[CURRENT_PADDLE_R].rigidBody->applyCentralImpulse( btVector3((n[12] - m[12]) * 5 , 0.0f,(n[14] - m[14]) * 5  ));
		}

		//If puck is moving towards AI Goal 
		if(linearPuckV[0] < 1)
		{

		//If behind Puck
		if(m[12] < n[12] - .5)
			{	
				//Hit Puck
				hockeyWorld[CURRENT_PADDLE_R].rigidBody->applyCentralImpulse( btVector3((n[12] - m[12]) * 5 , 0.0f,(n[14] - m[14]) * 5  ));
			}
	
		else
			{
				//Travel Behind Puck
				hockeyWorld[CURRENT_PADDLE_R].rigidBody->applyCentralImpulse( btVector3( (n[12] - 1 - m[12]) * 7 , 0.0f, 0.0f));
	
			}

		}
		



	}
else
{
hockeyWorld[CURRENT_PADDLE_R].rigidBody->applyCentralImpulse( btVector3((-9.65 - m[12]) * 2 , 0.0f,(n[14] - m[14]) * 1  ));
}

}

float calcVelocity(btVector3 vel)
{

return (vel[0]*vel[0] + vel[2]*vel[2]);
}
