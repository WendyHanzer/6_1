//Include GL Library
#include <GL/glew.h> // glew must be included before the main gl libs
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <GL/glut.h> // doing otherwise causes compiler shouting

//Imagemagick
#include <ImageMagick-6/Magick++.h>

//Include iostream
#include <iostream>

//Texture class 
class Texture
   {
    public:
        Texture(char * file);
        ~Texture();

        bool render();
        void bind();
    
    private:
        Magick::Image* m_pImage;
        Magick::Blob m_blob;
        char *fullPath;
        GLuint texture_buffer;
   };