#pragma once

#include <GL/glew.h>
#include <string>

enum TextureType : GLenum
{
	TEXTURE_2D = GL_TEXTURE_2D,
	TEXTURE_CUBEMAP = GL_TEXTURE_CUBE_MAP,
};

class Texture
{
	GLuint id = 0;
	TextureType type;

	void loadCubemap(const std::string& path);
	void load2D(const std::string& path);
public:
	
	void load(const std::string& path, TextureType type);

	void bind(size_t slot);
};