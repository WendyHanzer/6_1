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

//Include assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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
    //Setup the assimp scene
	const aiScene* scene = object.ReadFile(fileLoc, aiProcess_Triangulate);

	//Check for a false file
	if( !scene )
	   {
        return false;
	   }

	//Read the file info
	for(unsigned int outIndex = 0; outIndex < scene->mNumMeshes; outIndex++)
	   { 
   		//Make mesh to work on
		const aiMesh* objMesh = scene->mMeshes[outIndex];
		const aiMaterial* mat;
		aiColor4D objColor;
		
		//Checks if there is a file
		if(outIndex < scene->mNumMaterials)
		   {
		    mat = scene->mMaterials[outIndex+1];
		    if(AI_SUCCESS != mat->Get(AI_MATKEY_COLOR_DIFFUSE , objColor))
		       {
	            objColor.r = 1.0;
			    objColor.g = 1.0;
			    objColor.b = 1.0;
		       }
	       }
	    else
		   {
			objColor.r = 1.0;
			objColor.g = 1.0;
			objColor.b = 1.0;
		   }

		//loop through the mesh to get the verticies and color
		for(unsigned int indexMesh = 0; indexMesh < objMesh->mNumFaces; indexMesh++)
		   {
			//Temp Vertex to be assigned
			Vertex temp;

			//Update faces
			faces++;

			//Get the face with vertex coordinates
			const aiFace& face = objMesh->mFaces[indexMesh];

			//Loop through all three faces
			for(int index = 0; index < 3; index++)
			   {
				//Get the next face vertex data
				const aiVector3D* position = &(objMesh->mVertices[face.mIndices[index]]);

				//Get the 3 verticies of the current face
				temp.position[0] = position->x;
				temp.position[1] = position->y;
				temp.position[2] = position->z;
				temp.color[0] = objColor.r;
				temp.color[1] = objColor.g;
				temp.color[2] = objColor.b;

				//Push on the vertex
				data.push_back(temp);
			   }
		   }
	   }
	   
    //Set the materials
	if(!setMaterials(scene, fileLoc))
	   {
	    return false;
	   }

    //Return true: no errors
    return true;
   }

//Setup the materials
bool Object::setMaterials(const aiScene* pScene, char *fileLoc)
   {
    //Variables
	int slashLoc = 0;
	char *path = new char[strlen(fileLoc)];
	bool returnValue = true;
	
    //Get the directory of the files
	for(int index = 0; fileLoc[index] != '\0'; index++)
	   {
	    //Increment for the slash
	    if(fileLoc[index] == '/')
		   {
		    slashLoc++;
		   }
	   }
	   
    //set the path
	for(int index = 0, slshCount = 0; slshCount < slashLoc; index++ )
	   {
	    if(fileLoc[index] == '/')
		   {
		    slshCount++;
		   }
	    path[index] = fileLoc[index];
	   }
	   
    //Loop through the materials
	for(int index = 0; index < pScene->mNumMaterials; index++)
	   {
	    //Set the material
		const aiMaterial* materials = pScene->mMaterials[index];
		
		//assign to null 
	   }
	
	//Return 
	return returnValue;
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

//Return the data
Vertex* Object::getData()
   {
    return &data[0];
   }
