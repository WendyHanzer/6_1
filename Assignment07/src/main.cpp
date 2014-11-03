/*
Graphics 
Assignment07 -Solar System

This project will output a graphical solar system, involving interactive functionality.

Group:  Nolan Burfield
        Hardy Thrower
        Andy Garcia

To View code open with Tabs default to 4.
*/

//Includes
//Glew
#include <GL/glew.h> // glew must be included before the main gl libs
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <GL/glut.h> // doing otherwise causes compiler shouting

//Iostream
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <string>
#include <cmath>

//GLM Library
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier

//Classes
#include "shader.h"
#include "object.h"


//Structure to hold the moons data
struct Moon
{
	//Holds the object render data
	Object data;
	
	//Model for the planet
	glm::mat4 model;
	glm::mat4 mvp;
	
	//Spin and rotate
	float spin,
	rotate;
	
	//Meta data
	float scale,
	      spinTime,
	      rotateTime;
	
	float xDistance;
	float yDistance;
	float zDistance;
	
       Moon()
       {
		spin =  (rand()%10);
		rotate=(rand()%36);;
		spinTime = (rand()%10);
		rotateTime = (rand()%36);
       }
};

//Structure to hold the planets data
struct Planet
   {
    //Holds the object render data
    Object data;

    //Model for the plante
    glm::mat4 model;
    glm::mat4 mvp;

    //Spin and rotate
    float spin,
          rotate;

    //Meta data
    float scale,
          spinTime,
          rotateTime;
	int numMoons;
	    Moon* moons;
   };



//Global Output Values
Shader vertexShader;
Shader fragmentShader;
Planet *galaxy;
Moon	STANDARD_MOON;

//Globals to hold output data
float SCALE = 0.0;
float X_VIEW = 2.0, Y_VIEW = 9.0, Z_VIEW = -6.0;
float X_VIEW_NEXT = X_VIEW, Y_VIEW_NEXT = Y_VIEW, Z_VIEW_NEXT = Z_VIEW;
float X_FOCUS = 10.0, Y_FOCUS = -3.0, Z_FOCUS = 2.0;
float X_FOCUS_NEXT = X_FOCUS, Y_FOCUS_NEXT = Y_FOCUS, Z_FOCUS_NEXT = Z_FOCUS;
float X_FOCUS_ADJ = 0, Y_FOCUS_ADJ = 0, Z_FOCUS_ADJ = 0;
float X_VIEW_ADJ= 0, Y_VIEW_ADJ= 0, Z_VIEW_ADJ= 0;
float X_FOCUS_PREV = X_FOCUS;
float Y_FOCUS_PREV = Y_FOCUS;
float Z_FOCUS_PREV = Z_FOCUS;
int PLANET_INDEX = 1;// Initialize At Mercury
int PLANET_SIZE =0;
int MOON_SIZE = 0;
std::string CAPTION = "Caption Goes Here";
float YEAR_TO_MIN = 5.0;
bool IN_MOTION = true;
float RESUME;


//GL Values
int w = 640, h = 480;
int TEST_SIZE = 0;
GLuint program;

//Locations of shader data
GLint mvp_location,
      position_location,
      uv_location;
      
//Screen Matricies
glm::mat4 view,
          projection,
          mvp,
		  mvpText;


//View Functions


//for tonight


//Function Specifications
void display();
void update();
void keyboard(unsigned char key, int x_pos, int y_pos);
void menu(int id);
void keyboardSpecial(int key, int x_pos, int y_pos);
void reshape(int newWidth, int newHeight);
bool init();
bool parseFile(char *file);

//View Functions
glm::mat4 getView();
void slowChange(float &current, float next, float divisor);
bool isFocusGood(double threshold);
float calc3dDistance(float x1, float y1, float z1, float x2, float y2, float z2);


//Time function
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

//Main
//arguments can have fileName, vertexShader and fragmentShader
//Shaders have default shaders, and must include both or none
//File has a default
int main(int argc, char *argv[])
   {
    //Variables
    char vertexLoc[] = "../bin/vertexShader.glsl",
         fragmentLoc[] = "../bin/fragmentShader.glsl",
         objectLoc[] = "../bin/system3.txt";
         
    bool vertexStatus, fragmentStatus, objectStatus;

    //Start GLut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    glutCreateWindow("Solar System");
    Magick::InitializeMagick(*argv);

    //Initilize glew
    GLenum good = glewInit();
    if( good != GLEW_OK )
       {
        std::cerr << "GLEW did not initilize.\n";
        return -1;
       }

    //Set the files
    if(argc == 4)
       {
        vertexStatus = vertexShader.readFile( argv[1] );
        fragmentStatus = fragmentShader.readFile( argv[2] );
        objectStatus = parseFile( argv[3] );
       }
    else if(argc == 2)
       {
        vertexStatus = vertexShader.readFile( vertexLoc );
        fragmentStatus = fragmentShader.readFile( fragmentLoc );
        objectStatus = parseFile( argv[1] );
       }
    else
       {
        vertexStatus = vertexShader.readFile( vertexLoc );
        fragmentStatus = fragmentShader.readFile( fragmentLoc );
        objectStatus = parseFile( objectLoc );
       }
   
    //Check the files
    if(vertexStatus == false)
       {
        std::cerr << "Vertex Shader File not found.\n";
        return -1;
       }
    else if(fragmentStatus == false)
       {
        std::cerr << "Fragment Shader File not found.\n";
        return -1;
       }
    else if(objectStatus == false)
       {
        std::cerr << "Object Shader file not found\n";
        return -1;
       }
    
    //Set call backs
    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutIdleFunc( update );
    glutKeyboardFunc( keyboard );
	glutSpecialFunc(keyboardSpecial);
    glutCreateMenu(menu);
       glutAddMenuEntry("Start", 1);
       glutAddMenuEntry("Pause", 2);
       glutAddMenuEntry("Quit", 3);
    glutAttachMenu(GLUT_RIGHT_BUTTON);


    //Run program loop if init was ok
    if(init())
       {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
       }

    //End program 
    glDeleteProgram(program);
    return 0;
   }


//Function Implementations
//

//glut display
void display()
   {
    //Clear the buffer
    glClearColor(0.5, 0.6, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //Set the program
    glUseProgram(program);
    
    //Set the buffer
    glEnableVertexAttribArray( position_location );
    glEnableVertexAttribArray( uv_location );
    glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, galaxy[0].data.getSizeOf(), 0);
    glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, galaxy[0].data.getSizeOf(), galaxy[0].data.getOffSetUV());
	   
    //Render the galaxy
    for(int index = 0; index < PLANET_SIZE; index++)
       {
        galaxy[index].mvp = projection * view * galaxy[index].model;
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(galaxy[index].mvp));
        galaxy[index].data.render();

        //Render the moons in the planet of the galaxy
        for(int i = 0; i < galaxy[index].numMoons; i++)
           {
            galaxy[index].moons[i].mvp = projection * view * galaxy[index].moons[i].model;
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(galaxy[index].moons[i].mvp));
            galaxy[index].moons[i].data.render();
           }
		   
       }
	
	   
	
    //Clean pointers
    glDisableVertexAttribArray(position_location);
    glDisableVertexAttribArray(uv_location);
    
    //Swap the bufer
    glutSwapBuffers();
   }
   
//glut idle
void update()
   {
    //get the time for the program
     float dt = getDT();

    //For the background
    galaxy[TEST_SIZE-1].model = glm::translate( glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0)) *
                                glm::scale(glm::mat4(1.0f), glm::vec3((galaxy[TEST_SIZE-1].scale)));

    //Sun
    galaxy[0].model = glm::translate( glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0)) * 
                          glm::rotate(glm::mat4(1.0f), galaxy[0].spin, glm::vec3(0.0, 1.0, 0.0)) *
                          glm::scale(glm::mat4(1.0f), glm::vec3( SCALE + galaxy[0].scale ));

    //Set the correct angle spin based on the year to minute
    //Algorithm: SpinAngle   = (( spinTime * 360 ) / ( minutesToYear * 60 ) * degreeToRadians ) --> 365 for days in year
    //           RotateAngle = (( rotateTime   * 360 ) / ( minutesToYear * 60 ) * degreeToRadians ) --> 1 for one translation in a year

    float degreeToRadians = M_PI / 180;
    float spinAngle;
    float rotateAngle;
    glm::mat4 tempTranslation;

    //Loop through each of the plantes
    for(int index = 1; index < PLANET_SIZE; index++)
       {
        //Update rotate and spin
        spinAngle = ( (galaxy[index].spinTime * 360) / (YEAR_TO_MIN * 60) ) * degreeToRadians;
        rotateAngle = ( (galaxy[index].rotateTime * 360) / (YEAR_TO_MIN * 60) ) * degreeToRadians;
        galaxy[index].spin += dt * spinAngle;
        galaxy[index].rotate += dt* rotateAngle;
       
        //Set a separate point for each model
        tempTranslation = glm::translate( galaxy[0].model, glm::vec3((index+index)*sin(galaxy[index].rotate), 0.0, (index+index)*cos(galaxy[index].rotate)));
        galaxy[index].model = tempTranslation *
							  glm::rotate(glm::mat4(1.0f), galaxy[index].spin, glm::vec3(0.0, 1.0, 0.0)) *
                              glm::scale(glm::mat4(1.0f), glm::vec3(SCALE + (galaxy[index].scale)));

		if(galaxy[index].numMoons > 0)
		   {
			   
			   for(int i = 0; i <  galaxy[index].numMoons; i++)
			   {
				   Moon currentMoon = galaxy[index].moons[i];
				   spinAngle = ((currentMoon.spinTime * 360) / (YEAR_TO_MIN * 60) ) * degreeToRadians;
				   rotateAngle = (( currentMoon.rotateTime * 360) / (YEAR_TO_MIN * 60) ) * degreeToRadians;
				   currentMoon.spin += dt * spinAngle;
				   currentMoon.rotate += dt* rotateAngle;
				   galaxy[index].moons[i].model = glm::translate( galaxy[index].model, glm::vec3(currentMoon.xDistance*sin(currentMoon.rotate), 0, currentMoon.zDistance*cos(currentMoon.rotate))) *
				                                  glm::rotate(glm::mat4(1.0f), currentMoon.spin, glm::vec3(0.0, 1.0, 0.0)) *
				                                  glm::scale(glm::mat4(1.0f), glm::vec3( SCALE + currentMoon.scale ));
			   }
		   }
       }


	//Set next position for camera
    X_VIEW_NEXT = galaxy[PLANET_INDEX].model[3][0] + X_VIEW_ADJ;
    Y_VIEW_NEXT = galaxy[PLANET_INDEX].model[3][1] + Y_VIEW_ADJ;
    Z_VIEW_NEXT = galaxy[PLANET_INDEX].model[3][2] + Z_VIEW_ADJ;
    X_FOCUS_NEXT = galaxy[PLANET_INDEX].model[3][0]+ X_FOCUS_ADJ;
    Y_FOCUS_NEXT = galaxy[PLANET_INDEX].model[3][1]+ Y_FOCUS_ADJ;
    Z_FOCUS_NEXT = galaxy[PLANET_INDEX].model[3][2]+ Z_FOCUS_ADJ;
	   
    //View of the camera
    view = getView();

    //Redisplay the object
    glutPostRedisplay();
   }
   
//glut keyboard press
void keyboard(unsigned char key, int x_pos, int y_pos)
   {
    //Check escape
    if(key == 27)
       {
        exit(0);
       }
       
    //if it is plus
    if(key == 'b')
       {
        if(SCALE == 0.0)
           {
            SCALE = 0.25;
           }
        else
           {
            SCALE = 0.0;
           }
       }
	   if(key == ' ')
	   {
		   PLANET_INDEX = 0;
		   Z_VIEW_ADJ = 10;
		   Y_VIEW_ADJ = 5;
		   X_VIEW_ADJ = 5;
		
	   }
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
    //left
    if(key == 'a')
       {
        X_FOCUS_ADJ -= 1.0;
       }
    //right
    if(key == 's')
       {
        X_FOCUS_ADJ += 1.0;
       }
    //up
    if(key == 'd')
       {
        Y_FOCUS_ADJ -= 1.0;
       }
    //down
    if(key == 'f')
       {
        Y_FOCUS_ADJ += 1.0;
       }

    if(key == 'g')
       {
        Z_FOCUS -= 1.0;
       }

    if(key == 'h')
	{
		Z_FOCUS += 1.0;
	}
	   if(key >= '0' && key <= '9')
	   {
		   PLANET_INDEX = key-48;
		   X_FOCUS_ADJ = 0, Y_FOCUS_ADJ = 0, Z_FOCUS_ADJ = 0;
		   X_VIEW_ADJ= 0, Y_VIEW_ADJ= 0, Z_VIEW_ADJ= 0;
		   X_FOCUS_PREV = X_FOCUS;
		   Y_FOCUS_PREV = Y_FOCUS;
		   Z_FOCUS_PREV = Z_FOCUS;
	   }
	      //Print the new updated values
    printf("VIEW: %f, %f, %f\nFOCUS: %f, %f, %f\n", X_VIEW, Y_VIEW, Z_VIEW, X_FOCUS, Y_FOCUS, Z_FOCUS);
    if(SCALE == 0.0)
       {
        printf("SCALE OFF\n");
       }
    else
       {
        printf("SCALE ON\n");
       }
   }

void keyboardSpecial(int key, int x_pos, int y_pos)
{
	
	//Left Arrow
	if(key == GLUT_KEY_LEFT)
	{
		//Navigate Through Moons
		PLANET_INDEX--;
		if(PLANET_INDEX < 0)
		{
			PLANET_INDEX = 9;
		}
		X_FOCUS_ADJ = 0, Y_FOCUS_ADJ = 0, Z_FOCUS_ADJ = 0;
		X_VIEW_ADJ= 0, Y_VIEW_ADJ= 0, Z_VIEW_ADJ= 0;
		
	}
	
	//Right Arrow
	if(key == GLUT_KEY_RIGHT)
	{
		//Navigate Through Moons
		PLANET_INDEX++;
		
		if(PLANET_INDEX > 9)
		{
			
			PLANET_INDEX = 0;
		}
		X_FOCUS_ADJ = 0, Y_FOCUS_ADJ = 0, Z_FOCUS_ADJ = 0;
		X_VIEW_ADJ= 0, Y_VIEW_ADJ= 0, Z_VIEW_ADJ= 0;
	}

    if(key == GLUT_KEY_UP)
       {
        if( YEAR_TO_MIN <= 10.0 )
           {
            YEAR_TO_MIN += 1.0;
           }
       }

    if(key == GLUT_KEY_DOWN)
       {
        if( YEAR_TO_MIN > 1 )
           {
            YEAR_TO_MIN -= 1.0;
           }
       }
	
}

void menu(int id)
   {
    //switch on id
    switch (id) 
       {
        //Start
        case 1:
           YEAR_TO_MIN = RESUME;
           IN_MOTION = true;
           break;
        
        //Pause      
        case 2:
           RESUME = YEAR_TO_MIN;
           YEAR_TO_MIN = 1000.0;
           IN_MOTION = false;
           break;  
        
        //Escape
        case 3:
           exit(0);
           break;
       }
   }
   
//glut reshape
void reshape(int newWidth, int newHeight)
   {
    //Set new w and h
    w = newWidth;
    h = newHeight;
    
    //Set viewport
    glViewport(0, 0, w, h);
    
    //set projection
    projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 100.0f);
   }
   
//program initilizer
bool init()
   {
    //Create Shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    
    //Set the shader source for compiling
    GLchar* vertexSource = vertexShader.getShaderSource();
    GLchar* fragmentSource = fragmentShader.getShaderSource();
    
    //Compile Shaders
    GLint shader_status;
    
    //Vertex Shader
    char buffer[512];
    glShaderSource(vertex_shader, 1, (const GLchar**) &vertexSource, NULL);
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
    position_location = glGetAttribLocation(program, const_cast<const char*>("v_position"));
    if(position_location == -1)
       {
        std::cerr << "Position not found.\n";
        return false;
       }
       
    //color
    uv_location = glGetAttribLocation(program, const_cast<const char*>("texcoord"));
    if(uv_location == -1)
       {
        std::cerr << "Texture not found.\n";
        return false;
       }
       
    //mvp
    mvp_location = glGetUniformLocation(program, const_cast<const char*>("mvpMatrix"));
    if(mvp_location == -1)
       {
        std::cerr << "MVP matrix not found.\n";
        return false;
       }
    
    //Set the view and projetion
    view = glm::lookAt( glm::vec3(X_VIEW, Y_VIEW, Z_VIEW),     //Eye position
                        glm::vec3(0.0, 0.0, 0.0),       //Focus Point
                        glm::vec3(0.0, 1.0, 0.0));      //Position Y is up

    projection = glm::perspective( 45.0f,               //the FoV
                                   float(w)/float(h),   //Makes things stay consistent
                                   0.01f,               //Distance to the near plane
                                   100.0f);             //Distanc to the far plane
    
    //Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    //Return true since no fails
    return true;
   }
   
//Parse the object file
bool parseFile(char *file)
   {
    //Open file
    FILE *objectData = fopen(file, "r");

    //Get the lines in the file
    int numLines = 0;
    int readC;
    while( (readC=fgetc(objectData)) != EOF)
       {
        if(readC == '\n')
           {
            numLines++;
           }
       }

    //Create the memory and read in the files
    fseek( objectData, 0L, SEEK_SET );
    char objectFile[200];
    galaxy = new Planet[numLines];
    TEST_SIZE = numLines;
	   int tempPlanet_Index = 0;
	   
    for(int index = 0; index < numLines; index++)
       {
        char type;
        fscanf(objectData, "%c\t", &type);

        if(type == 'p')
           {
            fscanf(objectData, "%s\t%f\t%f\t%f\t%d\n", objectFile, &galaxy[tempPlanet_Index].scale, &galaxy[tempPlanet_Index].rotateTime, &galaxy[tempPlanet_Index].spinTime, &galaxy[tempPlanet_Index].numMoons);
            PLANET_SIZE++;
            galaxy[tempPlanet_Index].data.readFile(objectFile);
            tempPlanet_Index++;
           }

        if(type == 'm')
           {
            fscanf(objectData, "%s\t%f\t%f\t%f\n", objectFile, &STANDARD_MOON.scale, &STANDARD_MOON.rotateTime, &STANDARD_MOON.spinTime);
            STANDARD_MOON.data.readFile(objectFile);
            MOON_SIZE++;
           }
	   
       }
	   
    srand(time(NULL));
    //SET MOONS
    for(int index = 0; index< PLANET_SIZE; index++)
       {
        if( galaxy[index].numMoons != 0)
           {
            galaxy[index].moons = new Moon[galaxy[index].numMoons];
            if(index != 3 )
               {
                //Loop through the moons in the planet
                for( int i = 0; i < galaxy[index].numMoons; i++)
                   {
                    galaxy[index].moons[i].data = STANDARD_MOON.data;
                    galaxy[index].moons[i].scale = .03 * galaxy[index].scale * (rand() % 75);
                    if(galaxy[index].scale < .1)
                       {
                        galaxy[index].scale = .1;
                       }
                    galaxy[index].moons[i].xDistance = galaxy[index].scale + .5 +(.1 * (rand() % 75));
                    galaxy[index].moons[i].yDistance = galaxy[index].scale + .5 +(.1 * (rand() % 75));
                    galaxy[index].moons[i].zDistance = galaxy[index].scale + .5 +(.1 * (rand() % 75));
                   }
               }

            else
               {
                galaxy[index].moons[0].data = STANDARD_MOON.data;
                galaxy[index].moons[0].spin = 0;
                galaxy[index].moons[0].rotate = 0;
                galaxy[index].moons[0].scale = STANDARD_MOON.scale;
                galaxy[index].moons[0].spinTime = STANDARD_MOON.spinTime;
                galaxy[index].moons[0].rotateTime = STANDARD_MOON.rotateTime;
                galaxy[index].moons[0].xDistance = 4;
                galaxy[index].moons[0].yDistance = 4;
                galaxy[index].moons[0].zDistance = 4;
               }
           }
       }

    //Return
    return true;
   }

//Function to get the time
float getDT()
   {
    float ret;
    t2 = std::chrono::high_resolution_clock::now();
    ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
    t1 = std::chrono::high_resolution_clock::now();
    return ret;
   }

glm::mat4 getView()
{
	float thresholdX = 400;
	if(!isFocusGood( calc3dDistance(X_FOCUS_PREV,Y_FOCUS_PREV,Z_FOCUS_PREV,X_FOCUS_NEXT,Y_FOCUS_NEXT,Z_FOCUS_NEXT)*.90 ))
	{
		
	//adjust Zoom and focus
		if(fabs(X_FOCUS - X_VIEW) < .1  || (fabs(Y_FOCUS - Y_VIEW) < .1 ) || (fabs(Z_FOCUS - Z_VIEW) < .1) )
		{
			thresholdX = 800;
			
		}
		else
		{
			slowChange(thresholdX,20, 1.05);
		}
			slowChange(X_FOCUS,X_FOCUS_NEXT,thresholdX);
			slowChange(Y_FOCUS,Y_FOCUS_NEXT,thresholdX);
			slowChange(Z_FOCUS,Z_FOCUS_NEXT,thresholdX);

		
	}
	else if(!isFocusGood(calc3dDistance(X_FOCUS_PREV,Y_FOCUS_PREV,Z_FOCUS_PREV,X_FOCUS_NEXT,Y_FOCUS_NEXT,Z_FOCUS_NEXT)*.8 ) )
	{

		slowChange(thresholdX,40,1.1);
		//adjust Zoom and focus
		slowChange(X_FOCUS,X_FOCUS_NEXT,thresholdX);
		slowChange(Y_FOCUS,Y_FOCUS_NEXT,thresholdX);
		slowChange(Z_FOCUS,Z_FOCUS_NEXT,thresholdX);
		
	}
	else
	{
	//Finish Final Camera Rotation
	if(!isFocusGood(0))
		{

			
			slowChange(thresholdX,2,1.03);
			slowChange(X_FOCUS,X_FOCUS_NEXT,thresholdX);
			slowChange(Y_FOCUS,Y_FOCUS_NEXT,thresholdX);
			slowChange(Z_FOCUS,Z_FOCUS_NEXT,thresholdX);
		}
	//Reposition Camera
	slowChange(X_VIEW,X_VIEW_NEXT,20);
	slowChange(Y_VIEW,Y_VIEW_NEXT,20);
	slowChange(Z_VIEW,Z_VIEW_NEXT,20);
	}
	
	float finalXView = X_VIEW  + X_VIEW_ADJ + (galaxy[PLANET_INDEX].scale * 4) + .05;
	float finalYView = Y_VIEW  + Y_VIEW_ADJ + (galaxy[PLANET_INDEX].scale * 4) + .05;
	float finalZView = Z_VIEW  + Z_VIEW_ADJ + (galaxy[PLANET_INDEX].scale * 4) + .05;
	
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
	
	glm::mat4 tempView = glm::lookAt( glm::vec3(finalXView,finalYView,finalZView), //Eye position
						 glm::vec3(X_FOCUS , Y_FOCUS, Z_FOCUS ),//Focus Point
					     glm::vec3(0.0, 1.0, 0.0));         //Position Y is up
	
	
	return tempView;
	
}

void slowChange(float &current, float next, float divisor)
{
	float change;
		change = ((next-current)/divisor);
	
	current+=change;
	
}

bool isFocusGood(double threshold)
{
	
	double xComp = fabs(X_FOCUS_NEXT - X_FOCUS);
	double yComp = fabs(Y_FOCUS_NEXT - Y_FOCUS);
	double zComp = fabs(Z_FOCUS_NEXT - Z_FOCUS);
	
	if(xComp > threshold)
	   {
		return 0;
	   }
	if(yComp > threshold )
		{
			return 0;
		}
	
	if(zComp > threshold )
		{
		return 0;
		}
	
	return 1;
}

float calc3dDistance(float x1, float y1, float z1, float x2, float y2, float z2)
{
	return sqrt(pow(x1-x2,2) + pow(y1-y2,2) + pow(z1-z2,2));
}



