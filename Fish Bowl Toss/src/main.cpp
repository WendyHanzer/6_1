/*

Carnival Fish Bowl Game
Built for CS 480/680 Graphics Class at UNR

Group:
Nolan Burfield
Andy Garcia
Hardy Thrower

*/

// Headers
// Glew
#include <GL/glew.h> 
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <GL/glut.h> 

// IO Stream
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <time.h>
#include <math.h>
#include <queue> 	
using namespace std;

// Include Classes
#include "texture.h"
#include "shaderLoader.h"
#include "mesh.h"
#include "camera.h"
#include "wind.h"

// GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Bullet
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#define BIT(x) (1<<(x))

// Sound
#include <irrKlang.h>
#if defined(WIN32)
#include <conio.h>
#else
#include "./conio.h"
#endif
using namespace irrklang;

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
GLclampf BG_R = 0.0;
GLclampf BG_G = 0.0;
GLclampf BG_B = 0.0;

// Camera
Camera camera;

// Object Structure
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

	Object()
	   {
	   	shape = NULL;
	   	motion = NULL;
	   	rigidBody = NULL;
	   }
	
	//Movement Data
	float linVelocity[3];
	float angVelocity[3];
	float gravity[3];
	void saveMovement()
	   {
		btVector3 temp;
		btVector3 ZEROS(0,0,0);
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

	void restoreMovement()
	   {
		rigidBody->setLinearVelocity(btVector3(linVelocity[0],linVelocity[1],linVelocity[2]));
		rigidBody->setAngularVelocity(btVector3(angVelocity[0],angVelocity[1],angVelocity[2]));
		rigidBody->setGravity(btVector3(gravity[0],gravity[1],gravity[2]));
	   };

	float x()
	   {
		return model[3][0];
	   };
	float y()
	   {
		return model[3][1];
	   };
	float z()
	   {
		return model[3][2];
	   };

	// Get the current model, and set new mvp
	void updateModels(btTransform trans)
	   {
	   	// Variable
		btScalar m[16];

		//Set up the new model
		trans.getOpenGLMatrix(m);
		glm::vec4 one = glm::vec4(m[0], m[1], m[2], m[3]);
		glm::vec4 two = glm::vec4(m[4], m[5], m[6], m[7]);
		glm::vec4 three = glm::vec4(m[8], m[9], m[10], m[11]);
		glm::vec4 four = glm::vec4(m[12], m[13], m[14], m[15]);
		glm::mat4 temp = glm::mat4(one, two, three, four);
		model = temp;

		//Set the mvp in the object
		mvp = projection * view * model;
	   };

	   void updateModels3(glm::mat4 trans)
	   {
	   	//Update Model
		model = trans;

		//Set the mvp in the object
		mvp = projection * view * model;
	   };

	// Reset the object to the default location
	void reset(btTransform trans)
	   {
		//Variables
		btScalar m[16];

		//Get the matrix location
		trans.getOpenGLMatrix(m);
		float currentX = m[12];
		float currentY = m[13];
		float currentZ = m[14];

		//Set Back to the defaults
		//Remove the velocities
		btVector3 translateBody( (defaultX - currentX) , (defaultY - currentY), (defaultZ - currentZ));
		rigidBody->translate(translateBody);
		rigidBody->activate(true);
		rigidBody->setLinearVelocity( btVector3( 0.0, 0.0, 0.0 ) );
		rigidBody->setAngularVelocity(btVector3(0.0, 0.0, 0.0));
		rigidBody->applyCentralImpulse( btVector3(0.0f, 0.0f, 0.0f) );
		currentX = m[12];
		currentY = m[13];
		currentZ = m[14];
		//printf("Default(x,y,z): %f, %f, %f \n",defaultX,defaultY,defaultZ);
		//printf("Current(x,y,z): %f, %f, %f \n",currentX,currentY,currentZ);

	   }
   };

// Struct for the ball
struct Ball : Object
   {
   	// For tossing the ball
   	float x_impulse;
   	float y_impulse;
   	float z_impulse;
   	// If it was tossed
   	bool tossed;
   };

// Struct to hold bowl information
struct Bowl : Object
   {
   	Object collision;
   	bool hit;
   };


struct Replay
	{
		struct timeStamp { float s_time; glm::mat4 s_impulse;};
		btRigidBody* rigidBody;
		bool isCapturing;
		bool isCaptured;
		bool isPlaying;
		bool isFirst;
		float delay = 1;
		std::queue<timeStamp> impulses;
		std::queue<timeStamp> playlist;
		std::chrono::time_point<std::chrono::high_resolution_clock> tCurrent, tInit, playTime;
		Replay(){ isCapturing =  isCaptured = isPlaying = isFirst = false;}
		void startRec()
			{
				if(!isPlaying && !isCapturing)
				{
					if(isCaptured)
					{
						clear();
					}	
					isCapturing = true;
					tCurrent = tInit = std::chrono::high_resolution_clock::now();
					isFirst = true;
					std::cout <<"Begin Recording" << std::endl;

				}
			};
		void capture(glm::mat4 impulse)
			{
				if(isCapturing)
				{
				//Capture Time
				tCurrent = std::chrono::high_resolution_clock::now();
				float tNew = std::chrono::duration_cast< std::chrono::duration<float> >(tCurrent-tInit).count() + delay;
				//Capture Impulse
				timeStamp newStamp;
				newStamp.s_time = tNew;
				newStamp.s_impulse = impulse;
				impulses.push(newStamp);
					if(isFirst)
					{
						for(int i = 0; i < delay; i++)
						{
							impulses.push(newStamp);
						}
						isFirst = false;
					}
				}
			};
		void endRec()
		{
			isCapturing = false;
			isFirst = false;
			isCaptured = true;
			playlist = impulses;
			std::cout <<"End Recording" << std::endl;
		};
		bool playRec()
		{
			if(!isCapturing && !isPlaying && isCaptured  )
			{	
				//Play Recording
				isPlaying = true;
				playTime = std::chrono::high_resolution_clock::now();
				std::cout << "Beginning Playback" << std::endl; 
			}

			if(isPlaying)
			{
				timeStamp stamp = playlist.front();
				tCurrent = std::chrono::high_resolution_clock::now();
				float tNew = std::chrono::duration_cast< std::chrono::duration<float> >(tCurrent-playTime).count();
				if(!playlist.empty())
				{
					while (stamp.s_time <= tNew)
					{

						playlist.pop();
						stamp = playlist.front();
						if(playlist.empty())
						{
							break;
						}

					} 
				}
				if(playlist.empty())
					{ std::cout << "Playback Q Empty" << std::endl; isPlaying = false; return false; }
				return true;
			}

			std::cout << "Playback Failed" << std::endl;
			return false;
		};

		void clear()
		{
			if(!isCapturing && !isPlaying)
			 while (!impulses.empty())
			 {
			    impulses.pop();
			 }
			 isCaptured = false;
		};

		float timeLeft()
		{

			return playlist.size();
		};

		void restart()
		{
			playTime = std::chrono::high_resolution_clock::now();
			playlist = impulses;
		}

	};

// Glut Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void menu(int id);
void keyboardSpecial(int key, int x_pos, int y_pos);
void mouseClick(int button, int state, int x, int y);

// Game functions
bool initialize();
void writeString(char* line, float x, float y,int type);
void cleanUp();
void resetGame();
bool loadGame();
bool loadBall();
bool loadBowl();
void cleanUp();
bool checkWin();
void onWin();
void triggerExplosion();

// Sound
irrklang::ISoundEngine* engine = createIrrKlangDevice();

// Wind
btVector3 WIND_DIRECTION = btVector3(-1,0,.5);
Wind wind(5,15,200,WIND_DIRECTION);


// Time Keeping
std::chrono::time_point<std::chrono::high_resolution_clock> t0,t1,t2,tWin,tNow;
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

// The Bullet World
btBroadphaseInterface* broadphase;
btDefaultCollisionConfiguration* collisionConfiguration;
btCollisionDispatcher* dispatcher;
btSequentialImpulseConstraintSolver* solver;
btDiscreteDynamicsWorld* dynamicsWorld;
int collision;

// Game Stuff
std::vector<Object> gameWorld;
std::vector<Bowl> bowl;
Ball ball;
Replay replay;

bool REPLAY = false;
bool INTRO = true;
bool WINNING = false;
bool WIND = true;
static float angle = 0.0;
float dtShip = getDT();
int MODEL_HIT = -1;

// Main Game Function
int main(int argc, char **argv)
   {
	// Initialize glut
	Magick::InitializeMagick(*argv);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(w, h);
	srand(time(NULL));
	// Name and create the Window
	glutCreateWindow("Fish Bowl Toss");

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

	// Set all of the callbacks to GLUT that we need
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutIdleFunc(update);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboardSpecial);
	glutMouseFunc(mouseClick);
	glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
	//int sub = glutCreateMenu(menu);
	//	glutAddMenuEntry("Add One", 5);
	//	glutAddMenuEntry("Remove One", 6);
	//int sub2 = glutCreateMenu(menu);
	//	glutAddMenuEntry("Level 1", 7);
	//	glutAddMenuEntry("Level 2", 8);
	//int sub3 = glutCreateMenu(menu);
	//	glutAddMenuEntry("Slow", 9);
	//	glutAddMenuEntry("Moderate", 10);
	//	glutAddMenuEntry("Fast", 11);
	glutCreateMenu(menu);
		glutAddMenuEntry("Quit", 1);
		glutAddMenuEntry("Restart", 2);
	//	glutAddSubMenu("Balls", sub);
	//	glutAddSubMenu("Levels", sub2);
	//	glutAddSubMenu("Difficulty", sub3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	//Begin the program
	bool init = initialize();
	if( init )
	   {
	   	// Initiate the times
		t1 = std::chrono::high_resolution_clock::now();
		t0 = std::chrono::high_resolution_clock::now();

		// *Sound* Play Sound
		irrklang::ISound* snd = engine->play2D("../src/media/theme.wav", true, false, true);
		snd->setVolume(0.3);
		glutMainLoop();

		// Clean up after ourselves
		cleanUp();
	   }

   	return 0;
   }

// This is the render call, it will put the objects to the screen
void render()
   {
	//clear the screen
	glClearColor(BG_R, BG_G, BG_B, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//enable the shader program
	glUseProgram(program);

	//Get the time change
	static float timeChange = 0;
	float dt = getDT();
	timeChange += dt;

	//Write Buffer
	char buffer[250];

	//Variables
	btTransform trans;

	//Update the current world
	dynamicsWorld->stepSimulation(1.f/60.f, 1);

	// Put in the lights
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

	//Loop through all the world objects
	for(unsigned int i = 0; i < gameWorld.size(); i++)
	   {
	   	if( gameWorld[i].rigidBody != NULL)
	   	   {
			//Get the new transformation
			gameWorld[i].rigidBody->getMotionState()->getWorldTransform(trans);
			gameWorld[i].updateModels(trans);
		   }
		else
		   {
		   	gameWorld[i].mvp = projection * view * gameWorld[i].model;
		   }

		//upload the matrix to the shader
		glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(gameWorld[i].mvp));
		glUniformMatrix4fv(loc_modelView, 1, GL_FALSE, glm::value_ptr(gameWorld[i].model));

		// Render Objects
		gameWorld[i].objectMesh.Render(loc_position, loc_color, loc_normal);			
	   }

	//Loop through all the walls
	for(unsigned int i = 0; i < bowl.size(); i++)
	   {
	   	if( !bowl[i].hit )
	   	   {
			//Get the new transformation
			bowl[i].rigidBody->getMotionState()->getWorldTransform(trans);
			bowl[i].updateModels(trans);

			//Output Location
			//std::cout<<"Model " << i << " Y: "<< trans.getOrigin().getY() << " X: " << trans.getOrigin().getX() << " Z: " << trans.getOrigin().getZ() << std::endl;

			//upload the matrix to the shader
			glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(bowl[i].mvp));
			glUniformMatrix4fv(loc_modelView, 1, GL_FALSE, glm::value_ptr(bowl[i].model));

			// Render Objects
			bowl[i].objectMesh.Render(loc_position, loc_color, loc_normal);
		   }
	   }

	// All for that ball


	// Check if the ball is thrown
	if(ball.y() < -1)
	   {
		//Variables
		btTransform trans;

		// Reset the ball
		ball.rigidBody->getMotionState()->getWorldTransform(trans);
		ball.rigidBody->setGravity(btVector3(0, 0, 0));
		ball.tossed = false;

		ball.reset(trans);

		ball.tossed = false;
		replay.endRec();
	   }

	// Render Objects
	if (INTRO)
		{
		camera.setThresh(1,10);
		camera.setObj(50.0*sin(angle)+14.0, 7.0,50.0*cos(angle)+13.0 );
		camera.setMode(4);
		camera.viewUpdate();

		sprintf(buffer, "Click To Begin");
		writeString(buffer, -0.1, 0.5, 3);
		}
	else if(!REPLAY)
	   {
		// Get the new transformation
		ball.rigidBody->getMotionState()->getWorldTransform(trans);
		if(ball.tossed)
		   {
			replay.capture(ball.model);
		   }
		ball.updateModels(trans);
	   }
	else 
	   {
		camera.setObj(ball.x(),ball.y(),ball.z());
		camera.viewUpdate();

		ball.tossed = false;
		ball.model = replay.playlist.front().s_impulse;

		if(!replay.playRec())
		   {
			REPLAY = false;
			if(MODEL_HIT != -1)
			{
				triggerExplosion();
			}

			camera.setMode(0);
			camera.viewUpdate();
		   }
		ball.updateModels3(ball.model);
		sprintf(buffer, "REPLAY");
		writeString(buffer, -0.1, 0.5, 0);
	   }



	// upload the matrix to the shader
	glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(ball.mvp));
	glUniformMatrix4fv(loc_modelView, 1, GL_FALSE, glm::value_ptr(ball.model));
	ball.objectMesh.Render(loc_position, loc_color, loc_normal);
	
	if(!INTRO)
	{
		// Setup and output the scores
		sprintf(buffer, "Toss Power %4.2f", fabs(ball.z_impulse));
		writeString(buffer, -0.9, 0.75, 0);
		if(WIND)
			{sprintf(buffer, "Wind: X: %4.2f,Y: %4.2f,Z: %4.2f", wind.getForce().x(),wind.getForce().y(),wind.getForce().z());}
		else
			{sprintf(buffer,"Wind: OFF");}
		writeString(buffer, -0.9, 0.65, 2);
	}

	// Check to see if there was a win
	if( checkWin() )
	   {
		onWin();
		ball.tossed = false;
		ball.rigidBody->setGravity(btVector3(0, 0, 0));
		ball.rigidBody->getMotionState()->getWorldTransform(trans);
		ball.reset(trans);
	   }
 
	if(WINNING)
	{
		sprintf(buffer, "            YOU WIN          ");
		writeString(buffer, -0.9, 0.45, 1);
		tNow = std::chrono::high_resolution_clock::now();
		if(std::chrono::duration_cast< std::chrono::duration<float> >(tNow-tWin).count() > 1)
		{
			WINNING = false;
			engine->play2D("../src/media/explosion.wav", false);
			REPLAY = true;
	   		camera.setMode(3);
	  	 	camera.setThresh(20,10);
	   		replay.restart();
		}
	}
	//swap the buffers
	glutSwapBuffers();
   }

//Move the objects through time, and update the models
void update()
   {
	// Add wind to ball only if it was thrown
	wind.calcWind();
	if(ball.tossed)
	   {
	        if(WIND)
		{
	            ball.rigidBody->applyCentralImpulse(wind.getForce());
		}
	   }
	if(REPLAY)
	   {
		if(replay.timeLeft() < 100)
		   {
		   }
	   }

	// Move the ship
	dtShip = getDT();
	angle += dtShip*M_PI/4;
	gameWorld[1].model = glm::translate(glm::mat4(1.0f), glm::vec3(50.0*sin(angle)+14.0, 5.0, 50.0*cos(angle)+13.0))*
                             glm::rotate(glm::mat4(1.0f), angle-89.0f, glm::vec3(0.0, 4.0, 0.0))*
                             glm::scale(glm::mat4(1.0f), glm::vec3(0.05));


	view = camera.getView();
	updateSpotLights();

	//Redisplay the object
	glutPostRedisplay();
   }

//Get the input that comes from the keyboard
void keyboard(unsigned char key, int x_pos, int y_pos)
   {
	// Escape Key
	if(key == 27)
	   {
	   	cleanUp();
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
	   	camera.setMode(0);
	   	camera.viewUpdate();
		engine->play2D("../src/media/bell.wav");
	   }

	// Game reset
	if(key == 'p')
	   {
	   	resetGame();
	   	camera.setMode(0);
	   	camera.viewUpdate();
	   }
	  // Game Replay
	if(key == '1')
	   {
	   	camera.setMode(3);
	   	camera.setThresh(20,10);
	   	replay.restart();
	   	REPLAY = true;
	   }
    //Wind Toggle
	if(key == '2')
	   {
        WIND = !WIND;
	   }

    //Wind Random
	if(key == '3')
	   {
           float randX = ((rand() % 200) - 100.0)/100.0;
           float randY = ((rand() % 200) - 100.0)/100.0;
           float randZ = ((rand() % 200) - 100.0)/100.0;
	   WIND_DIRECTION = btVector3( randX, randY,randZ);
           wind.set(5,15,200, WIND_DIRECTION);
	   }

	// Check for the change of input
	if((key == 'q') || (key == 'w') || (key == 'e') || (key == 'r') || (key == 't') || (key == 'y') || (key == 'a') || (key == 's') || (key == 'd') || (key == 'f') || (key == 'g') || (key == 'h'))
	   {
	   	camera.cameraMove(key);
	   }

	// Set the power to throw the ball
	if(key == '=')
	   {
		ball.z_impulse -= 10;
	   }
	if(key == '-')
	   {
		ball.z_impulse += 10;
	   }
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

	// Dynamics
	dynamicsWorld->setGravity(btVector3(0, 0, 0));

	//Set at least one ball and first level
	bool check1 = loadGame();
	check1 = loadBall() && check1;
	check1 = loadBowl() && check1;

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
	view = camera.getView();
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

// Clean Up all the bullet pointers
void cleanUp()
   {
	// Loop through all the bowls
	for(unsigned int i = 0; i < bowl.size(); i++)
	   {
		dynamicsWorld->removeRigidBody(bowl[i].rigidBody);
		delete bowl[i].rigidBody->getMotionState();
		delete bowl[i].rigidBody;
		delete bowl[i].shape;
		dynamicsWorld->removeRigidBody(bowl[i].collision.rigidBody);
		delete bowl[i].collision.rigidBody->getMotionState();
		delete bowl[i].collision.rigidBody;
		delete bowl[i].collision.shape;
	   }

	// Loop through all the game objects
	for(unsigned int i = 0; i < gameWorld.size(); i++)
	   {
	   	if(gameWorld[i].rigidBody != NULL)
	   	   {
			dynamicsWorld->removeRigidBody(gameWorld[i].rigidBody);
			delete gameWorld[i].rigidBody->getMotionState();
			delete gameWorld[i].rigidBody;
			delete gameWorld[i].shape;
		   }
	   }

	// Remove Ball
	dynamicsWorld->removeRigidBody(ball.rigidBody);
	delete ball.rigidBody->getMotionState();
	delete ball.rigidBody;
	delete ball.shape;

	delete solver;
	delete dispatcher;
	delete collisionConfiguration;
	delete broadphase;
	delete dynamicsWorld;
	glDeleteProgram(program);

	//*Sound* Delete Sound Object
	engine->drop();
   }

// Reads the movement of the mouse on the screen
void mouseClick(int button, int state, int x, int y)
   {
	if(button == GLUT_LEFT_BUTTON && state == GLUT_UP && !ball.tossed && !REPLAY &&!WINNING && !INTRO)
	   {
	   	// Set the x based off the center of the screen
	   	x = x - (w/2);
		//printf("%d",w);
		// Reset the ball Gravity
		ball.tossed = true;
		ball.rigidBody->setGravity(btVector3(0, -9.81, 0));

		// Show the location of the click
		//printf("On Click: X: %d, Y: %d\n", x, y);

		// Apply a force to the ball based off the click location
		ball.rigidBody->activate(true);
		ball.rigidBody->applyCentralImpulse(btVector3(ball.x_impulse * x, ball.y_impulse * y, ball.z_impulse));
		replay.startRec();
		replay.capture(ball.model);
		engine->play2D("../src/media/pop.wav", false);

	   }
	if(button == GLUT_RIGHT_BUTTON && state == GLUT_UP && !ball.tossed && !REPLAY &&!WINNING && !INTRO)
	{


		x = 173;
		y = 923;
		WIND = false;


		// Reset the ball Gravity
		ball.tossed = true;
		ball.rigidBody->setGravity(btVector3(0, -9.81, 0));

		// Apply a force to the ball based off the click location
		ball.rigidBody->activate(true);
		ball.rigidBody->applyCentralImpulse(btVector3(ball.x_impulse * x, ball.y_impulse * y, -800));
		replay.startRec();
		replay.capture(ball.model);
		engine->play2D("../src/media/pop.wav", false);

	}

	if(button == GLUT_LEFT_BUTTON && state == GLUT_UP && INTRO)
		{
			camera.setMode(0);
			camera.setThresh(20,10);
		   	camera.viewUpdate();
	
			INTRO = false;
			
		}
   }

//Writes a bit map string passed to the screen
void writeString( char* line, float x, float y, int type )
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

	switch(type)
	{
	case 0:
		glColor3f(1.0f, 1.0f, 0.0f);
	break;

	case 1:
		glColor3f(.01f, 1.0f, .01f);
	break;

	case 2:
		glColor3f(1.0f, 1.0f, .01f);
	break;

	case 3:
		glColor3f( abs(sin(angle) * 2), abs(sin(angle) * 2), abs(sin(angle))* 2) ;
			//glColor3f(1.0f, 1.0f, .01f);
	break;
	}

		glRasterPos2f( x, y); 
   	// Output the text
	if (type <= 1)
	{
		for( int x = 0; line[x] != '\0'; x++ )
		   {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, line[x]);
		   }
	}

	if (type >= 2)
	{
		for( int x = 0; line[x] != '\0'; x++ )
		   {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, line[x]);
		   }
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

	// Reset the ball
	ball.rigidBody->getMotionState()->getWorldTransform(trans);
	ball.rigidBody->setGravity(btVector3(0, 0, 0));
	ball.tossed = false;
	WINNING = false;
	ball.reset(trans);

   	// Set all the bowls back
	for(unsigned int j = 0; j < bowl.size(); j++)
	   {
		if( bowl[j].hit )
		   {
			bowl[j].hit = false;
			dynamicsWorld->addRigidBody(bowl[j].collision.rigidBody);
			dynamicsWorld->addRigidBody(bowl[j].rigidBody);
		   }
	   }
	REPLAY = false;

   	replay.endRec();

   }

void lightsEnabled()
   {
	std::cout << "Ambient:" << ENABLE_AMB << std::endl;
	std::cout << "DL:" << ENABLE_DL << std::endl;
	std::cout << "PL:" << ENABLE_PL << std::endl;
	std::cout << "SL:" << ENABLE_SL << std::endl;
   }

void updateSpotLights() 
   {
	float ret;
	light1x = ball.x();
	light1y = ball.y();
	light1z = ball.z();

	ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t0).count();
	light2x = sin(ret);

	// Check if there is a bowl first
	if(bowl.size() > 0)
	   {
		light2y = bowl[0].y();
		light2z = bowl[0].z();
	   }
   }

// Checks if there is a win
bool checkWin() 
   {
	int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i=0;i<numManifolds;i++)
	   {
		btPersistentManifold* contactManifold =  dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* obA = (contactManifold->getBody0());
		const btCollisionObject* obB = (contactManifold->getBody1());

		if(obA->getCollisionShape() == ball.shape || obB->getCollisionShape() == ball.shape)
		   {
		   	for(unsigned int j = 0; j < bowl.size(); j++)
		   	   {
		   	   	if(obA->getCollisionShape() == bowl[j].collision.shape || obB->getCollisionShape() == bowl[j].collision.shape)
		   	   	   {
					MODEL_HIT = j;
					dynamicsWorld->removeRigidBody(bowl[j].collision.rigidBody);			
					return true;
		   	   	   }
		   	   }
		   }
	   }
	return false;
   }

// When there is a ball in a cup
void onWin()
   {
	// Output Win
	engine->play2D("../src/media/explosion.wav", false);
	WINNING = true;
	tWin = std::chrono::high_resolution_clock::now();
	replay.endRec();
	
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

   }

// Menu
void menu(int id)
   {
	//switch on id
	switch (id) 
	   {
		//Escape
		case 1:
			cleanUp();
			exit(0);
			break;

		case 2:
			resetGame();
			break;

	   }
		
   }

// Load in all the game world data
bool loadGame()
   {
	// Set the size of the world
	gameWorld.resize(7);

	// Load in the board
	// Lelev 1
	if(!gameWorld[0].objectMesh.LoadMesh("../bin/objects/stand.obj"))
	   {
		std::cerr<<"Main Board Failed to Load\n";
		return false;
	   }

	btScalar mass = 1.0;
	btVector3 fallInertia(0, 1, 0);
	gameWorld[0].shape = new btBvhTriangleMeshShape(gameWorld[0].objectMesh.triMesh, false);
	gameWorld[0].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(14, -5, 13)));
	gameWorld[0].shape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo CI(mass, gameWorld[0].motion, gameWorld[0].shape, fallInertia);
	CI.m_restitution = 0.7;
	gameWorld[0].rigidBody = new btRigidBody(CI);
	gameWorld[0].rigidBody->setActivationState(DISABLE_DEACTIVATION);
	dynamicsWorld->addRigidBody(gameWorld[0].rigidBody);
	gameWorld[0].rigidBody->setLinearFactor(btVector3(0, 0, 0));
	gameWorld[0].rigidBody->setAngularFactor(btVector3(1, 0, 1));
	gameWorld[0].rigidBody->setDamping(0, 0);
	gameWorld[0].rigidBody->setGravity(btVector3(0, 0, 0));

	if(!gameWorld[1].objectMesh.LoadMesh("../bin/objects/black_pearl.obj"))
	   {
		std::cerr<<"Ship Failed to Load\n";
		return false;
	   }

	if(!gameWorld[2].objectMesh.LoadMesh("../bin/objects/background2.obj"))
	   {
		std::cerr<<"Ground Failed to Load\n";
		return false;
	   }

	btScalar massOfGround = 0;
	btVector3 fallInertiaOfGround(0, 0, 0);
	gameWorld[2].shape = new btBoxShape(btVector3(18.55, 17.55, 0));
	gameWorld[2].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(18.55, 17.55, 0)));
	gameWorld[2].shape->calculateLocalInertia(massOfGround, fallInertiaOfGround);
	btRigidBody::btRigidBodyConstructionInfo CIground(massOfGround, gameWorld[2].motion, gameWorld[2].shape, fallInertiaOfGround);
	CIground.m_restitution = 0.7;
	gameWorld[2].rigidBody = new btRigidBody(CIground);
	gameWorld[2].rigidBody->setActivationState(DISABLE_DEACTIVATION);
	dynamicsWorld->addRigidBody(gameWorld[2].rigidBody);
	gameWorld[2].rigidBody->setLinearFactor(btVector3(0, 0, 0));

		if(!gameWorld[3].objectMesh.LoadMesh("../bin/objects/bowlpart1.obj"))
			   {
				std::cerr<<"A bowl Failed to Load\n";
				return false;
			   }

			btScalar mass4 = 100.0;
			btVector3 fallInertia4(0,1,0);
			gameWorld[3].defaultX = (0);
			gameWorld[3].defaultY = (0);
			gameWorld[3].defaultZ = (0);
			gameWorld[3].shape = new btBvhTriangleMeshShape(gameWorld[3].objectMesh.triMesh, false);
			gameWorld[3].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(gameWorld[3].defaultX, gameWorld[3].defaultY, gameWorld[3].defaultZ)));
			gameWorld[3].shape->calculateLocalInertia(mass4, fallInertia4);
			btRigidBody::btRigidBodyConstructionInfo CI4(mass4, gameWorld[3].motion, gameWorld[3].shape, fallInertia4);
			CI4.m_restitution = 0.7;
			CI4.m_friction = 0.3;
			gameWorld[3].rigidBody = new btRigidBody(CI4);
			dynamicsWorld->addRigidBody(gameWorld[3].rigidBody);

			if(!gameWorld[4].objectMesh.LoadMesh("../bin/objects/bowlpart2.obj"))
			   {
				std::cerr<<"A bowl Failed to Load\n";
				return false;
			   }

			gameWorld[4].defaultX = (0);
			gameWorld[4].defaultY = (0);
			gameWorld[4].defaultZ = (0);
			gameWorld[4].shape = new btBvhTriangleMeshShape(gameWorld[4].objectMesh.triMesh, false);
			gameWorld[4].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(gameWorld[4].defaultX, gameWorld[4].defaultY, gameWorld[4].defaultZ)));
			gameWorld[4].shape->calculateLocalInertia(mass4, fallInertia4);
			btRigidBody::btRigidBodyConstructionInfo CI5(mass4, gameWorld[4].motion, gameWorld[4].shape, fallInertia4);
			CI5.m_restitution = 0.7;
			CI5.m_friction = 0.3;
			gameWorld[4].rigidBody = new btRigidBody(CI5);
			dynamicsWorld->addRigidBody(gameWorld[4].rigidBody);

			if(!gameWorld[5].objectMesh.LoadMesh("../bin/objects/bowlpart3.obj"))
			   {
				std::cerr<<"A bowl Failed to Load\n";
				return false;
			   }

			gameWorld[5].defaultX = (0);
			gameWorld[5].defaultY = (0);
			gameWorld[5].defaultZ = (0);
			gameWorld[5].shape = new btBvhTriangleMeshShape(gameWorld[5].objectMesh.triMesh, false);
			gameWorld[5].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(gameWorld[5].defaultX, gameWorld[5].defaultY, gameWorld[5].defaultZ)));
			gameWorld[5].shape->calculateLocalInertia(mass4, fallInertia4);
			btRigidBody::btRigidBodyConstructionInfo CI6(mass4, gameWorld[5].motion, gameWorld[5].shape, fallInertia4);
			CI6.m_restitution = 0.7;
			CI6.m_friction = 0.3;
			gameWorld[5].rigidBody = new btRigidBody(CI6);
			dynamicsWorld->addRigidBody(gameWorld[5].rigidBody);

			if(!gameWorld[6].objectMesh.LoadMesh("../bin/objects/bowlpart4.obj"))
			   {
				std::cerr<<"A bowl Failed to Load\n";
				return false;
			   }

			gameWorld[6].defaultX = (0);
			gameWorld[6].defaultY = (0);
			gameWorld[6].defaultZ = (0);
			gameWorld[6].shape = new btBvhTriangleMeshShape(gameWorld[6].objectMesh.triMesh, false);
			gameWorld[6].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(gameWorld[6].defaultX, gameWorld[6].defaultY, gameWorld[6].defaultZ)));
			gameWorld[6].shape->calculateLocalInertia(mass4, fallInertia4);
			btRigidBody::btRigidBodyConstructionInfo CI7(mass4, gameWorld[6].motion, gameWorld[6].shape, fallInertia4);
			CI7.m_restitution = 0.7;
			CI7.m_friction = 0.3;
			gameWorld[6].rigidBody = new btRigidBody(CI7);
			dynamicsWorld->addRigidBody(gameWorld[6].rigidBody);

	// Return Success
	return true;
   }

// Setup the ball in the game
bool loadBall()
   {
	if(!ball.objectMesh.LoadMesh("../bin/objects/ball4.obj"))
	   {
		std::cerr<<"Ball Failed to Load\n";
		return false;
	   }

	btScalar mass = 100.0;
	btVector3 fallInertia(0,1,0);
	// Prev Defaults 0 , 8.0, 21.0
	ball.defaultX = camera.nextX();
	ball.defaultY = camera.nextY() - 7;
	ball.defaultZ = camera.nextZ() - 5;
	ball.shape = new btSphereShape(1.5);
	ball.motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(ball.defaultX, ball.defaultY, ball.defaultZ)));
	ball.shape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo CI(mass, ball.motion, ball.shape, fallInertia);
	CI.m_restitution = 0.7;
	ball.rigidBody = new btRigidBody(CI);
	ball.rigidBody->setGravity(btVector3(0, 0, 0));
	dynamicsWorld->addRigidBody(ball.rigidBody);

	ball.x_impulse = 1.0;
	ball.y_impulse = 1.0;
	ball.z_impulse = -1500.0;
	ball.tossed = false;
	replay.rigidBody = ball.rigidBody;

	// Return Success
	return true;
   }

// Load all the bowls in the game
bool loadBowl()
   {
	// Set the size of fish bowls
	bowl.resize(16);

	// Read in the bowls
	for( unsigned int i = 0; i < bowl.size()/4; i++)
	   {
	   	for( unsigned int j = 0; j < bowl.size()/4; j++)
	   	   {
			if(!bowl[(i*4)+j].objectMesh.LoadMesh("../bin/objects/lgfb.obj"))
			   {
				std::cerr<<"A bowl Failed to Load\n";
				return false;
			   }

			// If the bowls are the center ones make them taller
			if( (i*4)+j == 5 || (i*4)+j == 6 || (i*4)+j == 9 || (i*4)+j == 10 )
			   {
				bowl[(i*4)+j].defaultY = 5;
			   }
			else
			   {
			   	bowl[(i*4)+j].defaultY = 1.5;
			   }

			btScalar mass = 100.0;
			btVector3 fallInertia(0,1,0);
			bowl[(i*4)+j].defaultX = ((j*13));
			bowl[(i*4)+j].defaultZ = ((i*13));
			bowl[(i*4)+j].shape = new btBvhTriangleMeshShape(bowl[(i*4)+j].objectMesh.triMesh, false);
			bowl[(i*4)+j].motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(bowl[(i*4)+j].defaultX, bowl[(i*4)+j].defaultY, bowl[(i*4)+j].defaultZ)));
			bowl[(i*4)+j].shape->calculateLocalInertia(mass, fallInertia);
			btRigidBody::btRigidBodyConstructionInfo CI(mass, bowl[(i*4)+j].motion, bowl[(i*4)+j].shape, fallInertia);
			CI.m_restitution = 0.7;
			CI.m_friction = 0.3;
			bowl[(i*4)+j].rigidBody = new btRigidBody(CI);
			bowl[(i*4)+j].rigidBody->setLinearFactor(btVector3(0, 0, 0));
			dynamicsWorld->addRigidBody(bowl[(i*4)+j].rigidBody);
	
			if(!bowl[i].collision.objectMesh.LoadMesh("../bin/objects/ball3.obj"))
			   {
				std::cerr<<"Bowl Collision Failed to Load\n";
				return false;
			   }

			btScalar mass2 = 100.0;
			btVector3 fallInertia2(0,1,0);
			bowl[(i*4)+j].collision.defaultX = ((j*13));
			bowl[(i*4)+j].collision.defaultY = 2;
			bowl[(i*4)+j].collision.defaultZ = ((i*13));
			bowl[(i*4)+j].collision.shape = new btSphereShape(1);
			bowl[(i*4)+j].collision.motion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(bowl[(i*4)+j].collision.defaultX, bowl[(i*4)+j].collision.defaultY, bowl[(i*4)+j].collision.defaultZ)));
			bowl[(i*4)+j].collision.shape->calculateLocalInertia(mass2, fallInertia2);
			btRigidBody::btRigidBodyConstructionInfo CI2(mass2, bowl[(i*4)+j].collision.motion, bowl[(i*4)+j].collision.shape, fallInertia);
			bowl[(i*4)+j].collision.rigidBody = new btRigidBody(CI2);
			bowl[(i*4)+j].collision.rigidBody->setLinearFactor(btVector3(0, 0, 0));
			dynamicsWorld->addRigidBody(bowl[(i*4)+j].collision.rigidBody);
	
			//Init Struct Members
			btVector3 temp = dynamicsWorld->getGravity();
			bowl[(i*4)+j].gravity[0]= temp.x();
			bowl[(i*4)+j].gravity[1]= -6.81;
			bowl[(i*4)+j].gravity[2]= temp.z();
			bowl[(i*4)+j].hit = false;
		   }
	   }

	// Return Success
	return true;
   }

void triggerExplosion()
{
	btTransform trans;
	bowl[MODEL_HIT].hit = true;
	dynamicsWorld->removeRigidBody(bowl[MODEL_HIT].rigidBody);
	gameWorld[3].defaultX = bowl[MODEL_HIT].x();
	gameWorld[3].defaultY = bowl[MODEL_HIT].y();
	gameWorld[3].defaultZ = bowl[MODEL_HIT].z();
	gameWorld[4].defaultX = bowl[MODEL_HIT].x();
	gameWorld[4].defaultY = bowl[MODEL_HIT].y();
	gameWorld[4].defaultZ = bowl[MODEL_HIT].z();
	gameWorld[5].defaultX = bowl[MODEL_HIT].x();
	gameWorld[5].defaultY = bowl[MODEL_HIT].y();
	gameWorld[5].defaultZ = bowl[MODEL_HIT].z();
	gameWorld[6].defaultX = bowl[MODEL_HIT].x();
	gameWorld[6].defaultY = bowl[MODEL_HIT].y();
	gameWorld[6].defaultZ = bowl[MODEL_HIT].z();

	// Reset the bowl
	gameWorld[3].rigidBody->getMotionState()->getWorldTransform(trans);
	gameWorld[3].reset(trans);
	gameWorld[4].rigidBody->getMotionState()->getWorldTransform(trans);
	gameWorld[4].reset(trans);
	gameWorld[5].rigidBody->getMotionState()->getWorldTransform(trans);
	gameWorld[5].reset(trans);
	gameWorld[6].rigidBody->getMotionState()->getWorldTransform(trans);
	gameWorld[6].reset(trans);
	gameWorld[3].rigidBody->setGravity( btVector3(0.0f, -9.81f, 0.0f) );
	gameWorld[4].rigidBody->setGravity( btVector3(0.0f, -9.81f, 0.0f) );
	gameWorld[5].rigidBody->setGravity( btVector3(0.0f, -9.81f, 0.0f) );
	gameWorld[6].rigidBody->setGravity( btVector3(0.0f, -9.81f, 0.0f) );
	gameWorld[3].rigidBody->applyCentralImpulse( btVector3(2500.0f, 3200.0f, 0.0f) );
	gameWorld[4].rigidBody->applyCentralImpulse( btVector3(-2500.0f, 3200.0f, 2400.0f) );
	gameWorld[5].rigidBody->applyCentralImpulse( btVector3(0.0f, 3200.0f, 1900.0f) );
	gameWorld[6].rigidBody->applyCentralImpulse( btVector3(0.0f, 3200.0f, -2400.0f) );
	engine->play2D("../src/media/explosion.wav", false, false);
	MODEL_HIT = -1;
}
