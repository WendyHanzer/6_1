//Include header
#include "object.h"

//Include GL Library
#include <GL/glew.h> // glew must be included before the main gl libs
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <GL/glut.h> // doing otherwise causes compiler shouting

//GLM Library
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier

//Include iostream
#include <iostream>
#include <vector>

//Constructor
Object::Object()
   {
    //All to do is set faces to 0
    faces = 0;
   }

//Copy constructor
Object::Object(Object &src)
   {
    //Check if there is nothing
    if(src.faces == 0)
	   {
	    faces = 0;
	   }
	   
    //Else assign the data and faces
    else
	   {
	    faces = src.faces;
		data = src.data;
	   }
   }

//Destructor
Object::~Object()
   {
    //Clean up the data
	data.clear();
	faces = 0;
   }
   
//Read in the file to the data
bool Object::readFile(char *fileLoc)
   {
    //Variables
	char *mtlFile;
	int index = 0;
    glm::vec3 color;
	std::vector< glm::vec3 > temp_verticies;
	bool vnFile = false, vtFile = false;
	
    //Open the file
	FILE *source = fopen(fileLoc, "r");
	
	//Check if the file exists
	if(source == NULL)
	   {
	    return false;
	   }
	
	//Get the material file as well
	mtlFile = new char[strlen(fileLoc)];
	
    //Loop to add the path of the mtl file
	while(fileLoc[index] != '\0')
	   {
	    mtlFile[index] = fileLoc[index];
		index++;
	   }
    mtlFile[index] = '\0';
    mtlFile[index-1] = 'l';
	mtlFile[index-2] = 't';
	mtlFile[index-3] = 'm';
	
	//OPen material file
	FILE *mtlsource = fopen(mtlFile, "r");
	   
    //Loop through the file
	//The method of reading in the file came from opengl-tutorial.org
	char word[50];
	while( fscanf(source, "%s", word) != EOF)
	   {
	    //Check if the pointer is at a vertex
	    if( strcmp( word, "v" ) == 0)
		   {
		    //Set a vertex to the points read in three points
		    glm::vec3 vertex;
			fscanf(source, "%f %f %f", &vertex.x, &vertex.y, &vertex.z);
			temp_verticies.push_back(vertex);
		   }
		   
	    //Check to set the normals
		else if( strcmp( word, "vn" ) == 0)
		   {
			vnFile = true;
		   }
		   
	    //Check to set the textures
		else if( strcmp( word, "vt" ) == 0)
		   {
			vtFile = true;
		   }
		   
	    //Check for the material to use
	    else if(strcmp( word, "usemtl") == 0 && mtlsource != NULL)
		   {
			//Get the location of the diffuse
		    fscanf(mtlsource, "%s", word);
		    while(strcmp( word, "Kd") != 0)
			   {
			    fscanf(mtlsource, "%s", word);
			   }
			   
		    //set the color to the diffuse
			fscanf(mtlsource, "%f %f %f", &color.x, &color.y, &color.z);
		   }
		   
	    //Check if the file is on a face
	    else if(strcmp( word, "f") == 0)
		   {
		    //get the x, y, z of the file for each vertex
		    float x, y, z, dummy;
			
			//Read in the correct face format
			//Vertex only file
			if( !vnFile && !vtFile )
			   {
			    fscanf(source, "%f %f %f", &x, &y, &z);
			   }
			//Vertex and vertex normal
			else if( !vnFile && vtFile )
			   {
			    fscanf(source, "%f/%f %f/%f %f/%f", &x, &dummy, &y, &dummy, &z, &dummy);
			   }
			//Vertex and vertex normal
			else if( vnFile && !vtFile )
			   {
			    fscanf(source, "%f//%f %f//%f %f//%f", &x, &dummy, &y, &dummy, &z, &dummy);
			   }
			//Vertex and vertex normal
			else
			   {
			    fscanf(source, "%f/%f/%f %f/%f/%f %f/%f/%f", &x, &dummy, &dummy, &y, &dummy, &dummy, &z, &dummy, &dummy);
			   }
			
			//Set the first vertex, and push onto data
			Vertex temp_vertex = {{temp_verticies[x-1].x, temp_verticies[x-1].y, temp_verticies[x-1].z}, {color.x, color.y, color.z}};
			data.push_back(temp_vertex);
			
			//Push second on to data
			temp_vertex = {{temp_verticies[y-1].x, temp_verticies[y-1].y, temp_verticies[y-1].z}, {color.x, color.y, color.z}};
		    data.push_back(temp_vertex);
			
			//push third onto data
			temp_vertex = {{temp_verticies[z-1].x, temp_verticies[z-1].y, temp_verticies[z-1].z}, {color.x, color.y, color.z}};
		    data.push_back(temp_vertex);
			
			//Count the number of faces made (number of triangles the program has)
			faces++;
		   }
	   }
	   
    //Close the files and free memory
	fclose(mtlsource);
	fclose(source);
	delete[] mtlFile;
	mtlFile = NULL;
	
	//No error return true
	return true;
   }
   
//Set the size of the vertex and color arrays together
int Object::bufferSize()
   {
    //Return the size: *6 for num values, *3 for 3 verticies a face, *4 for GLfloat
    return faces * 6 * 3 * 4;
   }
   
//Returns the number of faces
int Object::numFaces()
   {
    //Return the number of faces *3 for 3 verticies a face
	return faces * 3;
   }
   
//Get the size of a Vertex
int Object::getSizeOf()
   {
    return sizeof(Vertex);
   }

//Get the offset of the color to vertex
void* Object::getOffSetColor()
   {
    return (void*)offsetof(Vertex,color);
   }