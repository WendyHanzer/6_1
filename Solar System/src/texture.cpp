//Include iostream
#include <iostream>
#include "texture.h"
#include <stdio.h>
#include <string.h>

//Constructor
Texture::Texture(char *file)
   {
    //Copy the file path
    fullPath = new char[strlen(file)];
    fullPath[0] = '\0';
    strcpy(fullPath, file);;
   }

//Destructor
Texture::~Texture()
   {
    delete m_pImage;
   }

//Render the data
bool Texture::render()
   {
    //Variables
    bool returnValue = false;

    //Run image magick
    try 
       {
        m_pImage = new Magick::Image(fullPath);
        m_pImage->write(&m_blob, "RGBA");
        std::cout << "Loaded Texture: " << fullPath << std::endl;
        returnValue = true;
       }
    catch (Magick::Error& Error) 
       {
        std::cout << "Error loading texture '" << fullPath << "': " << Error.what() << std::endl;
        returnValue = false;
       }

    //Initilize the data
    glGenTextures(1, &texture_buffer);
    glBindTexture(GL_TEXTURE_2D, texture_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_pImage->columns(), m_pImage->rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_blob.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //Reurn
    return returnValue;
   }

//Bind the texture
void Texture::bind()
   {
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_buffer);
   }





