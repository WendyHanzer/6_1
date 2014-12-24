/*
  The mesh.cpp was refrenced From: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html

  Additions have been made to it from: 
                                      Nolan Burfield
                                      Andy Garcia
                                      Hardy Thrower

  This Class will read in an objects verticies, and the textures to each vertex.
  Additions to it are the bullet library to implement physics on the objects.
*/


//Headers
#ifndef MESH_H
#define	MESH_H

//Main Headers
#include <map>
#include <vector>
#include "texture.h"

//GLEW 
#include <GL/glew.h> 
#include <GL/glut.h> 

//Assimp
#include <assimp/Importer.hpp> 
#include <assimp/scene.h> 
#include <assimp/postprocess.h>

//Bullet
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>


//Structure to hold all verticies and UV
struct Vertex
   {
    float position[3];
    float uv[2];
    float normal[3];
   };

//Mesh Class
class Mesh
   {
    public:
        Mesh();
        ~Mesh();

        bool LoadMesh(const char* Filename);
        void Render(GLint, GLint, GLint);
        btTriangleMesh *triMesh;

    private:
        bool InitFromScene(const aiScene* pScene, const std::string& Filename);
        void InitMesh(unsigned int Index, const aiMesh* paiMesh);
        bool InitMaterials(const aiScene* pScene, const std::string& Filename);
        void Clear();

    #define INVALID_MATERIAL 0xFFFFFFFF

        //Structure to Hold rendering data
        struct MeshEntry 
           {
            MeshEntry();
            ~MeshEntry();

            void Init(const std::vector<Vertex>& Vertices,
                      const std::vector<unsigned int>& Indices);

            GLuint VB;
            GLuint IB;
            unsigned int NumIndices;
            unsigned int MaterialIndex;
           };

        //Holds the Data in Vectors
        std::vector<MeshEntry> m_Entries;
        std::vector<Texture*> m_Textures;
   };


#endif	/* MESH_H */

