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
    
   }

//Copy constructor
Object::Object(Object &src)
   {

   }

//Destructor
Object::~Object()
   {
    //Clean up the data
    data.clear();
   }
   
//Read in the file to the data
bool Object::readFile(char *fileLoc)
   {
    //Setup the assimp scene
    const aiScene* scene = object.ReadFile(fileLoc, aiProcess_Triangulate);
    meshes.resize(scene->mNumMeshes);

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
        mesh tempMesh;

        //Assign material index
        tempMesh.materialIndex = objMesh->mMaterialIndex;

        //loop through the mesh to get the verticies and color
        for(unsigned int indexMesh = 0; indexMesh < objMesh->mNumFaces; indexMesh++)
           {
            //Temp Vertex to be assigned
            Vertex temp;

            //Update faces
            tempMesh.faces++;
            //meshes[outIndex].faces++;

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
                tempMesh.data.push_back(temp);
                //meshes[outIndex].data.push_back(temp);
               }
           }

        //Push on mesh
        (meshes[outIndex]) = tempMesh;

        //Setup the vertex buffer
        glGenBuffers(1, &meshes[outIndex].vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, meshes[outIndex].vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, (meshes[outIndex].faces * 6 * 3 * 4), &((meshes[outIndex]).data[0]), GL_STATIC_DRAW);
       }
       
    //Set the materials
    if(!setMaterials(scene, fileLoc))
       {
        std::cout<<"Materials did not load."<<std::endl; 
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
    textures.resize(pScene->mNumMaterials);
    for(unsigned int index = 0; index < pScene->mNumMaterials; index++)
       {
        //Set the material
        const aiMaterial* materials = pScene->mMaterials[index];
        textures[index] = NULL;

        if(materials->GetTextureCount(aiTextureType_DIFFUSE) > 0)
           {
            aiString aiPath;
            if(materials->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
               {
                char *fullPath = new char[(strlen(path) + strlen(aiPath.data))];
		fullPath[0] = '\0';
                strcat(fullPath, path);
                strcat(fullPath, aiPath.data);
                textures[index] = new Texture(fullPath);
                textures[index]->render();
               }
           }
       }
    
    //Return 
    return returnValue;
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

//Render the data
void Object::render()
   {
    //Loop through all the meshes
    for(unsigned int index = 0; index < meshes.size(); index++)
       {
        //Bind buffer
        glBindBuffer(GL_ARRAY_BUFFER, meshes[index].vertex_buffer);

        //Set texture
        if(meshes[index].materialIndex < (int)(textures.size()))
           {
            textures[(meshes[index].materialIndex)]->bind();
           }

        //Draw object
        glDrawArrays(GL_TRIANGLES, 0, meshes[index].faces * 3);
       }
   }







