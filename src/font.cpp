#include "font.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtx/transform.hpp>
#include "renderer.hpp"

void Font::load(const std::string & path)
{
	font.load(path, TEXTURE_2D);

	shader.create("text.vert", "text.frag");

	width_pixels = 128;
	height_pixels = 256;
	width_chars = 16;
	height_chars = 16;

	float x_offset = 1.f / width_chars;
	float y_offset = 1.f / height_chars;

	GLfloat vertices[] = {
		0, 0, 0,  0,        0,
		1, 0, 0,  x_offset, 0,
		0, 1, 0,  0,        -y_offset,
		1, 1, 0,  x_offset, -y_offset,
	};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*5*4, &vertices[0], GL_STATIC_DRAW);

	unsigned int stride = sizeof(GLfloat) * 5;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat)*3));

	glBindVertexArray(0);
}

glm::vec2 Font::charOffset(unsigned char chr)
{
	int x = chr % width_chars;
	int y = chr / width_chars + 1;

	glm::vec2 offset;
	offset.x = x / (float)width_chars;
	offset.y = y / (float)height_chars;

	return offset;
}

void Font::drawText(Renderer& renderer, float x, float y, const std::string & text)
{
	glm::vec2 window = renderer.getWindowSize();
	shader.use();
	glm::mat4 projection = glm::ortho(0.f, window.x, 0.f, window.y);
	shader.uniform("projection", projection);

	font.bind(0);
	shader.uniform("font_texture", 0);

	glBindVertexArray(vao);
	float char_width = width_pixels / (float)width_chars;
	float char_height = height_pixels / (float)height_chars;

	char_width  *= 1;
	char_height *= 1;

	for (int i = 0; i < text.size(); i++)
	{
		glm::vec3 char_pos;
		char_pos.x = x + i*char_width;
		char_pos.y = y;
		glm::mat4 model = glm::translate(char_pos) * glm::scale(glm::vec3(char_width, char_height, 1));
		shader.uniform("model", model);

		glm::vec2 offset = charOffset(text.at(i));

		shader.uniform("tex_offset", offset);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	glBindVertexArray(0);
}

