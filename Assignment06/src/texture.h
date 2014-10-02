#include <string>
#include <GL/glew.h>
#include <ImageMagick-6/Magick++.h>

class Texture
{
public:
    Texture(GLenum TextureTarget, char *FileName);

    bool Load();

    void Bind(GLenum TextureUnit);

private:
    char* m_fileName;
    GLenum m_textureTarget;
    GLuint m_textureObj;
    Magick::Image* m_pImage;
    Magick::Blob m_blob;
};

