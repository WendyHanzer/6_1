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
	textures = false;
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
				const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
				const aiVector3D* position = &(objMesh->mVertices[face.mIndices[index]]);
				const aiVector3D* uv = objMesh->HasTextureCoords(0) ? &(objMesh->mTextureCoords[0][face.mIndices[index]]) : &Zero3D;

				//Get the 3 verticies of the current face
				temp.position[0] = position->x;
				temp.position[1] = position->y;
				temp.position[2] = position->z;
				temp.uv[0] = uv->x;
				temp.uv[1] = uv->y;

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
	int loc = 0;
	
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
	int index, slshCount;
	for( index = 0, slshCount = 0; slshCount < slashLoc; index++ )
	   {
	    if(fileLoc[index] == '/')
		   {
		    slshCount++;
		   }
	    path[index] = fileLoc[index];
		loc++;
	   }
	   path[index] = '\0';
	   	   
    //Loop through the materials
	for(int index = 0; index < pScene->mNumMaterials; index++)
	   {
	    //Set the material
		const aiMaterial* materials = pScene->mMaterials[index];

		if(materials->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		   {
		    aiString aiPath;
		    if(materials->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			   {
			    char *fullPath = new char[(strlen(path) + strlen(aiPath.data))];
				fullPath = strcat(path, aiPath.data);
				std::cout<<fullPath<<std::endl;
				textures = true;

			   try 
				  {
				   m_pImage = new Magick::Image(fullPath);
				   m_pImage->write(&m_blob, "RGBA");
				   returnValue = true;
				  }
			   catch (Magick::Error& Error) 
				  {
				   std::cout << "Error loading texture '" << fullPath << "': " << Error.what() << std::endl;
				   return false;
				  }
			    
			   }
		   }
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
void* Object::getOffSetUV()
   {
    return (void*)offsetof(Vertex,uv);
   }

//Return the data
Vertex* Object::getData()
   {
    return &data[0];
   }

const GLvoid* Object::getImageData()
   {
    return m_blob.data();
   }
   
GLsizei Object::getImageColumns()
   {
    return m_pImage->columns();
   }

GLsizei Object::getImageRows()
   {
    return m_pImage->rows();
   }
   
bool Object::hasTexture()
   {
    return textures;
   }