//Includes
//Glew 
#include <GL/glew.h> // glew must be included before the main gl libs
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <GL/glut.h> // doing otherwise causes compiler shouting

//Iostream
#include <iostream>

//GLM Library
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier

//Classes
#include "shader.h"
#include "object.h"

//Global Output Values
Shader vertexShader;
Shader fragmentShader;
Object test;
float SCALE = 1.0;

//GL Values
int w = 640, h = 480;
GLuint program,
       vertex_buffer,
	   texture_buffer;
GLint mvp_location,
      position_location,
	  uv_location;
	  
//Screen Matricies
glm::mat4 model,
          view,
		  projection,
		  mvp;


//Function Specifications
void display();
void update();
void keyboard(unsigned char key, int x_pos, int y_pos);
void reshape(int newWidth, int newHeight);
bool init();


//Main
//argv should have files in this order: vertexShader, fragmentShader, objectFile
int main(int argc, char *argv[])
   {
    //Variables
	char vertexLoc[] = "../bin/vertexShader.glsl",
	     fragmentLoc[] = "../bin/fragmentShader.glsl",
		 objectLoc[] = "../bin/box.obj";
		 
    bool vertexStatus, fragmentStatus, objectStatus;

    //Start GLut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(w, h);
	glutCreateWindow("Object Loader");
	
	Magick::InitializeMagick(*argv);

    //Set the files
	if(argc == 4)
	   {
	    vertexStatus = vertexShader.readFile( argv[1] );
		fragmentStatus = fragmentShader.readFile( argv[2] );
		objectStatus = test.readFile( argv[3] );
	   }
    else
	   {
	    vertexStatus = vertexShader.readFile( vertexLoc );
		fragmentStatus = fragmentShader.readFile( fragmentLoc );
		objectStatus = test.readFile( objectLoc );
	   }
   
    //Check the files
	if(vertexStatus == false)
	   {
	    std::cerr << "Vertex File not found.\n";
		return -1;
	   }
    else if(fragmentStatus == false)
	   {
	    std::cerr << "Fragment File not found.\n";
		return -1;
	   }
    else if(objectStatus == false)
	   {
	    std::cerr << "Object file not found\n";
		return -1;
	   }

    //Initilize glew
	GLenum good = glewInit();
	if( good != GLEW_OK )
	   {
	    std::cerr << "GLEW did not initilize.\n";
		return -1;
	   }
	
	//Set call backs
	glutDisplayFunc( display );
	glutReshapeFunc( reshape );
	glutIdleFunc( update );
	glutKeyboardFunc( keyboard );

    //Run program loop if init was ok
	if(init())
	   {
	    glutMainLoop();
	   }

    //End program 
	glDeleteProgram(program);
	glDeleteBuffers(1, &vertex_buffer);
	glDeleteTextures(1, &texture_buffer);
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
	
	//Multiply out the mvp
	mvp = projection * view * model;
	
	//Set the program
	glUseProgram(program);
	
	//upload the matrix to the shader
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
	
	//Set the buffer

	glEnableVertexAttribArray( position_location );
	glEnableVertexAttribArray( uv_location );
		glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, test.getSizeOf(), 0);
	glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, test.getSizeOf(), test.getOffSetUV());
	
	
    if(test.hasTexture())
	   {
	    glActiveTexture(GL_TEXTURE0);
	    glEnable(GL_TEXTURE_2D);
	    glBindTexture(GL_TEXTURE_2D, texture_buffer);
	   }
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	//Draw object
	glDrawArrays(GL_TRIANGLES, 0, test.numFaces());
	
	//Clean pointers
	glDisableVertexAttribArray(position_location);
	glDisableVertexAttribArray(uv_location);
	
	//Swap the bufer
	glutSwapBuffers();
   }
   
//glut idle
void update()
   {
    //Run a rotation to see all the object
    static float move = 0.0;
	float turn = (M_PI/2) * move;
	move += 0.01;
    model = glm::rotate(glm::mat4(1.0f), turn, glm::vec3(0.0, 0.3, 1.0)) *
	        glm::scale(glm::mat4(1.0f), glm::vec3(SCALE));
			
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
	if(key == '=')
	   {
	    SCALE += 0.25;
	   }
	   
	//if it the minus
	if(key == '-')
	   {
	    SCALE -= 0.25;
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
    //Variables
   
    //Initilize vertex buffer
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, test.bufferSize(), test.getData(), GL_STATIC_DRAW);
	
	if(test.hasTexture())
	   {
	    glGenTextures(1, &texture_buffer);
	    glBindTexture(GL_TEXTURE_2D, texture_buffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, test.getImageColumns(), test.getImageRows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, test.getImageData());
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	   }

	
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
	view = glm::lookAt( glm::vec3(0.0, 16.0, -16.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 100.0f);
	
	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	//Return true since no fails
	return true;
   }
   

