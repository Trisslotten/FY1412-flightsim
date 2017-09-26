#pragma once

#include "texture.hpp"
#include "shader.hpp"
#include <glm/glm.hpp>

class Renderer;

class Font
{
	
	GLuint vao;
	GLuint vbo;

	int width_pixels;
	int height_pixels;
	int width_chars;
	int height_chars;

	Texture font;

	ShaderProgram shader;

	glm::vec2 charOffset(unsigned char chr);

public:

	void load(const std::string& path);

	void drawText(Renderer& renderer, float x, float y, const std::string& text);
};