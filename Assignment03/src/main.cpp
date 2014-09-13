#include <GL/glew.h> // glew must be included before the main gl libs
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier

//--Data types
//This object will define the attributes of a vertex(position, color, etc...)
struct Vertex
{
    GLfloat position[3];
    GLfloat color[3];
};

//--Evil Global variables
//Just for this example!
int w = 640, h = 480;// Window size
GLuint program;// The GLSL program handle
GLuint vbo_geometry;// VBO handle for our geometry
bool ROTATE_FLAG = true;
bool ROTATE_FOREWARD = true;
bool SPIN_FOREWARD = true;
float YEAR_TO_MIN = 5.0;

//uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

//attribute locations
GLint loc_position;
GLint loc_color;

//transform matrices
glm::mat4 model_earth;//obj->world each object should have its own model matrix
glm::mat4 model_moon;
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 mvp;//premultiplied modelviewprojection
glm::mat4 mvp2;//premultiplied modelviewprojection

//Text output
//glm::mat4 text = glm::mat4(1000.0f);
//glm::mat4 mvp3 = projection * view * text;

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void menu(int id);
void mouse(int button, int state, int x, int y);
void specialKeys(int key, int x_pos, int y_pos);
void printText(char text[]);

//--Resource management
bool initialize(char *vs, char *fs);
static char* readShaderSource(const char* file);
void cleanUp();

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;


//--Main
int main(int argc, char **argv)
{
    //Setup the shader file
	
	//Shader Variables
	char stdVertShader[] = "vertexShader.glsl";
	char stdFragShader[] = "fragmentShader.glsl";
    char *vertexShader;
    char *fragmentShader;
	
	//Check if there was command line files
	//--Read in as vertex shader then fragment shader--
	if(argc == 3)
       {  
	    vertexShader = argv[1];
        fragmentShader = argv[2];
	   }
	else
	   {
	    vertexShader = stdVertShader;
        fragmentShader = stdFragShader;
	   }
 
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
	glutCreateMenu(menu);
	glutAddMenuEntry("Start", 1);
	glutAddMenuEntry("Pause", 2);
	glutAddMenuEntry("Quit", 3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	glutMouseFunc(mouse);
	glutSpecialFunc(specialKeys);

    // Initialize all of our resources(shaders, geometry)
    bool init = initialize(vertexShader, fragmentShader);
    if(init)
    {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
    }

    // Clean up after ourselves
    cleanUp();
    return 0;
}

//--Implementations
void render()
{
    //--Render the scene

    //clear the screen
    glClearColor(0.0, 0.0, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //premultiply the matrix for this example
    mvp  = projection * view * model_earth;
    mvp2 = projection * view * model_moon;

    //enable the shader program
    glUseProgram(program);

    //upload the matrix to the shader
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    //set pointers into the vbo for each of the attributes(position and color)
    glVertexAttribPointer( loc_position,//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(Vertex),//stride
                           0);//offset

    glVertexAttribPointer( loc_color,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,color));

    glDrawArrays(GL_TRIANGLES, 0, 36);//mode, starting index, count
	
	//Switch to the next mvp
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp2));
    glDrawArrays(GL_TRIANGLES, 0, 36);

	//glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp3));
	
	//Add the text to string
	//char textValue[] = "Earth Rotating Counterclockwise, and spinning Counterclockwise.";
/*
    if(ROTATE_FLAG)
	   {
	    if(ROTATE_FOREWARD && SPIN_FOREWARD)
		   {
            textValue = "Earth Rotating Counterclockwise, and spinning Counterclockwise.\0";
           }
	    else if(ROTATE_FOREWARD && !SPIN_FOREWARD)
           {
            textValue = "Earth Rotating Counterclockwise, and spinning Clockwise.\0";
		   }
	    else if(!ROTATE_FOREWARD && SPIN_FOREWARD)
           {
            textValue = "Earth Rotating Clockwise, and spinning Counterclockwise.\0";
		   }
	    else if(!ROTATE_FOREWARD && !SPIN_FOREWARD)
           {
            textValue = "Earth Rotating Clockwise, and spinning Clockwise.\0";
		   }
	   }
    else
	   {
	    textValue = "Paused.\0";
	   }
	   */
	//Print   
	//printText(textValue);

glColor3f(.6, 0.0, 1.0);
glRasterPos2f(10, 10);
glutBitmapString(GLUT_BITMAP_9_BY_15, "Hello");

    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);
	
    //swap the buffers
    glutSwapBuffers();
}

void update()
{
    //total time
    static float earthRotate = 0.0;
	static float earthSpin = 0.0;
	static float moonRotate = 0.0;
	static float moonSpin = 0.0;
    float dt = getDT();// if you have anything moving, use dt.
	glm::mat4 earthTranslation;
	
	//Set the correct angle spin based on the year to minute
	//Algorithm: earthSpinAngle   = (( 365 * 360 ) / ( minutesToYear * 60 ) * degreeToRadians ) --> 365 for days in year
	//           earthRotateAngle = (( 1   * 360 ) / ( minutesToYear * 60 ) * degreeToRadians ) --> 1 for one translation in a year
	//			 moonAngle        = (( 13  * 360 ) / ( minutesToYear * 60 ) * degreeToRadians ) --> 13 for rotations, and translations in a year
		
	float degreeToRadians = M_PI / 180;
	float moonAngle = (78.0 / YEAR_TO_MIN) * degreeToRadians;
	float earthRotateAngle = (6.0 / YEAR_TO_MIN) * degreeToRadians;
	float earthSpinAngle = (2190.0 / YEAR_TO_MIN) * degreeToRadians;

    if(ROTATE_FLAG)
	   {
	    if(ROTATE_FOREWARD && SPIN_FOREWARD)
		   {
            //Update angle, Earth and Moon
            earthRotate += dt * earthRotateAngle;
			earthSpin += dt * earthSpinAngle;
		    moonRotate += dt * moonAngle;
		    moonSpin += dt * moonAngle;
			
			//Get earth Translation
			earthTranslation = glm::translate( glm::mat4(1.0f), glm::vec3(4.0 * sin(earthRotate), 0.0, 4.0 * cos(earthRotate)));

            //Rotate and translate object 
            model_earth = earthTranslation * 
		                  glm::rotate(glm::mat4(1.0f), (earthSpin), glm::vec3(0.0, 1.0, 0.0));
					
			//Have moon follow earth
			model_moon = glm::translate( earthTranslation, glm::vec3(4.0 * sin(moonRotate), 0.0, 4.0 * cos(moonRotate))) *
			             glm::rotate(glm::mat4(1.0f), (moonSpin), glm::vec3(0.0, 1.0, 0.0)) *
						 glm::scale(glm::mat4(1.0f), glm::vec3(0.27f));
           }
	    else if(ROTATE_FOREWARD && !SPIN_FOREWARD)
           {
            //Update angle
            earthSpin -= dt * earthSpinAngle; 
			earthRotate += dt * earthRotateAngle;
		    moonRotate -= dt * moonAngle;
		    moonSpin -= dt * moonAngle;

			//Get earth Translation
			earthTranslation = glm::translate( glm::mat4(1.0f), glm::vec3(4.0 * sin(earthRotate), 0.0, 4.0 * cos(earthRotate)));

            //Rotate and translate object 
            model_earth = earthTranslation * 
		                  glm::rotate(glm::mat4(1.0f), (earthSpin), glm::vec3(0.0, 1.0, 0.0));
					
			//Have moon follow earth
			model_moon = glm::translate( earthTranslation, glm::vec3(4.0 * sin(moonRotate), 0.0, 4.0 * cos(moonRotate))) *
			             glm::rotate(glm::mat4(1.0f), (moonSpin), glm::vec3(0.0, 1.0, 0.0)) *
						 glm::scale(glm::mat4(1.0f), glm::vec3(0.27f));
		   }
	    else if(!ROTATE_FOREWARD && SPIN_FOREWARD)
           {
            //Update angle
            earthRotate -= dt * earthRotateAngle;
			earthSpin += dt * earthSpinAngle;
		    moonRotate += dt * moonAngle;
		    moonSpin += dt * moonAngle;

			//Get earth Translation
			earthTranslation = glm::translate( glm::mat4(1.0f), glm::vec3(4.0 * sin(earthRotate), 0.0, 4.0 * cos(earthRotate)));

            //Rotate and translate object 
            model_earth = earthTranslation * 
		                  glm::rotate(glm::mat4(1.0f), (earthSpin), glm::vec3(0.0, 1.0, 0.0));
					
			//Have moon follow earth
			model_moon = glm::translate( earthTranslation, glm::vec3(4.0 * sin(moonRotate), 0.0, 4.0 * cos(moonRotate))) *
			             glm::rotate(glm::mat4(1.0f), (moonSpin), glm::vec3(0.0, 1.0, 0.0)) *
						 glm::scale(glm::mat4(1.0f), glm::vec3(0.27f));
		   }
	    else if(!ROTATE_FOREWARD && !SPIN_FOREWARD)
           {
            //Update angle
            earthRotate -= dt * earthRotateAngle;
			earthSpin -= dt * earthSpinAngle;
		    moonRotate -= dt * moonAngle;
		    moonSpin -= dt * moonAngle;

			//Get earth Translation
			earthTranslation = glm::translate( glm::mat4(1.0f), glm::vec3(4.0 * sin(earthRotate), 0.0, 4.0 * cos(earthRotate)));

            //Rotate and translate object 
            model_earth = earthTranslation * 
		                  glm::rotate(glm::mat4(1.0f), (earthSpin), glm::vec3(0.0, 1.0, 0.0));
					
			//Have moon follow earth
			model_moon = glm::translate( earthTranslation, glm::vec3(4.0 * sin(moonRotate), 0.0, 4.0 * cos(moonRotate))) *
			             glm::rotate(glm::mat4(1.0f), (moonSpin), glm::vec3(0.0, 1.0, 0.0)) *
						 glm::scale(glm::mat4(1.0f), glm::vec3(0.27f));
		   }
	   }
				
    // Update the state of the scene
    glutPostRedisplay();//call the display callback
}


void reshape(int n_w, int n_h)
{
    w = n_w;
    h = n_h;
    //Change the viewport to be correct
    glViewport( 0, 0, w, h);
    //Update the projection matrix as well
    //See the init function for an explaination
    projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 100.0f);

}

void keyboard(unsigned char key, int x_pos, int y_pos)
{
    // Handle keyboard input
    if(key == 27)//ESC
    {
        exit(0);
    }
	
	//reverse cube direction
	if(key == ',')
	{
	   ROTATE_FOREWARD = false;
	}
	
	//foreward cube
	if(key == '.')
	{
	   ROTATE_FOREWARD = true;
	}
	
	//reverse spin direction
	if(key == 'z')
	{
	   SPIN_FOREWARD = false;
	}
	
	//foreward spin
	if(key == 'x')
	{
	   SPIN_FOREWARD = true;
	}
	
	//Make year 1 minute
	if(key == '1')
	{
	   YEAR_TO_MIN = 1;
	}
	
	//Make year 5 minutes
	if(key == '5')
	{
	   YEAR_TO_MIN = 5;
	}
}

void specialKeys(int key, int x_pos, int y_pos)
{
   //Key Up
   if(key == GLUT_KEY_UP)
   {
       SPIN_FOREWARD = true;
   } 
   
   //Key Down
   if(key == GLUT_KEY_DOWN)
   {
       SPIN_FOREWARD = false;
   }
   
   //Key Left
   if(key == GLUT_KEY_LEFT)
   {
       ROTATE_FOREWARD = true;
   }
   
   //Key Right
   if(key == GLUT_KEY_RIGHT)
   {
       ROTATE_FOREWARD = false;
   }
}

bool initialize(char *vs, char *fs)
{
    // Initialize basic geometry and shaders for this example

    //this defines a cube, this is why a model loader is nice
    //you can also do this with a draw elements and indices, try to get that working
    Vertex geometry[] = { {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},

                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},
                          
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          
                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},
                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},

                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},

                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},

                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}},
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},
                          
                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},

                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},

                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},

                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},

                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}}
                        };
    // Create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);

    //--Geometry done

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    //Shader Sources	
	//Vertex Shader
	GLchar* vertexSource = readShaderSource(vs);
	
	//Check if correct file
	if(vertexSource == NULL)
	   {
        std::cerr << "FAILED TO FIND VERTEX SHADER FILE!" << vs << std::endl;
        return false;
	   }
	
    //Fragment Shader
	GLchar* fragmentSource = readShaderSource(fs);
	
	//Check if correct file
	if(fragmentSource == NULL)
	   {
        std::cerr << "FAILED TO FIND FRAGMENT SHADER FILE!" << std::endl;
        return false;
	   }

    //compile the shaders
    GLint shader_status;

    // Vertex shader first
    glShaderSource(vertex_shader, 1, (const GLchar**) &vertexSource, NULL);
    glCompileShader(vertex_shader);
    //check the compile status
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE VERTEX SHADER!" << std::endl;
        return false;
    }

    // Now the Fragment shader
    glShaderSource(fragment_shader, 1, (const GLchar**) &fragmentSource, NULL);
    glCompileShader(fragment_shader);
    //check the compile status
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE FRAGMENT SHADER!" << std::endl;
        return false;
    }

    //Now we link the 2 shader objects into a program
    //This program is what is run on the GPU
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    //check if everything linked ok
    glGetProgramiv(program, GL_LINK_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] THE SHADER PROGRAM FAILED TO LINK" << std::endl;
        return false;
    }

    //Now we set the locations of the attributes and uniforms
    //this allows us to access them easily while rendering
    loc_position = glGetAttribLocation(program,
                    const_cast<const char*>("v_position"));
    if(loc_position == -1)
    {
        std::cerr << "[F] POSITION NOT FOUND" << std::endl;
        return false;
    }

    loc_color = glGetAttribLocation(program,
                    const_cast<const char*>("v_color"));
    if(loc_color == -1)
    {
        std::cerr << "[F] V_COLOR NOT FOUND" << std::endl;
        return false;
    }

    loc_mvpmat = glGetUniformLocation(program,
                    const_cast<const char*>("mvpMatrix"));
    if(loc_mvpmat == -1)
    {
        std::cerr << "[F] MVPMATRIX NOT FOUND" << std::endl;
        return false;
    }
    
    //--Init the view and projection matrices
    //  if you will be having a moving camera the view matrix will need to more dynamic
    //  ...Like you should update it before you render more dynamic 
    //  for this project having them static will be fine
    view = glm::lookAt( glm::vec3(0.0, 8.0, -16.0), //Eye Position
                        glm::vec3(0.0, 0.0, 0.0), //Focus point
                        glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up

    projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100.0f); //Distance to the far plane, 

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //and its done
    return true;
}

static char* readShaderSource(const char* file)
{
    //Variables
	long size;
	char* buffer;

    //Open the file
	FILE* source = fopen(file, "r");
	
    //Check if the file did not open
	if(source == NULL)
	   {
	    return NULL;
	   }
	   
    //Get end of file, and location
	fseek(source, 0L, SEEK_END);
	size = ftell(source);
	
	//create the buffer pointer
	fseek(source, 0L, SEEK_SET);
	buffer = new char[size+1];
	
	//Read into buffer
	fread(buffer, 1, size, source);
	
	//add end to buffer
	buffer[size] = ' ';
	
	//Close file and return
	fclose(source);
	return buffer;
}

void menu(int id)
{
    //switch on id
    switch (id) 
       {
	    //Start
	    case 1:
		   ROTATE_FLAG = true;
		   break;
		
		//Pause      
	    case 2:
	       ROTATE_FLAG = false;
		   break;  
		
		//Escape
	    case 3:
	       exit(0);
		   break;
	   }
}

void mouse(int button, int state, int x, int y)
{
    //On click pause
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	   {
	    ROTATE_FOREWARD = !ROTATE_FOREWARD;
	   }
}

void cleanUp()
{
    // Clean up, Clean up
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_geometry);
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

void printText(char text[])
{
    //Set Position
	glRasterPos2f(10, 10);
	
	//Loop to null char
    for (int index = 0; text[index] != '\0'; index++)
	   {
	    glColor3d(0.6, 1.0, 0.0);
	    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, text[index]);
	   }
}
