/*
  The texture.h was refrenced From: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html

  Additions have been made to it from: 
                                      Nolan Burfield
                                      Andy Garcia
                                      Hardy Thrower

  This class is strictly responsible for reading in the texture of an object using the
  image magick library. It saves the ingame data in a BLOB.
*/

#ifndef TEXTURE_H
#define	TEXTURE_H

#include <string>

#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <Magick++.h>

class Texture
   {
	public:
		Texture(GLenum TextureTarget, const std::string& FileName);

		bool Load();

		void Bind(GLenum TextureUnit);

	private:
		std::string m_fileName;
		GLenum m_textureTarget;
		GLuint m_textureObj;
		Magick::Image* m_pImage;
		Magick::Blob m_blob;
   };


#endif	/* TEXTURE_H */

