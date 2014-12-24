//
// Object Class
//

//Include GL Library
#include <GL/glew.h> // glew must be included before the main gl libs
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <GL/glut.h> // doing otherwise causes compiler shouting

//Include assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <vector>
#include "texture.h"

//Imagemagick
#include <ImageMagick-6/Magick++.h>

//Structure to hold the vartex and uv data
struct Vertex
   {
    GLfloat position[3];
    GLfloat uv[2];
   };

//Structure for each mesh
struct mesh
   {
    //Vertex data
    std::vector<Vertex> data;

    //Material index
    int materialIndex;

    //Faces
    int faces = 0;

    //Buffer
    GLuint vertex_buffer;
   };

//Object Class
class Object
   {
    public:
        //Necessary functions
        Object();
        Object(Object &src);
        ~Object();

        //setup object file
        bool readFile(char *fileLoc);
        bool readFile_Alternate(char *fileLoc);
        bool setMaterials(const aiScene* pScene, char *fileLoc);

        //get the size for buffer
        void render();
        int getSizeOf();
        void* getOffSetUV();
      
    private:
        std::vector<Texture*> textures;
        std::vector<Vertex> data;
        std::vector<mesh> meshes;
        Assimp::Importer object;
   };
