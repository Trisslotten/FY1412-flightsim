#pragma once

#include <map>
#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>


class ShaderProgram
{
	struct Paths
	{
		std::string vert, geom, frag;
	} paths;
	

	GLuint id;

	GLuint vs, gs, fs;

	GLuint findUniformID(const std::string& name);
	std::map<std::string, GLuint> uniform_ids;
public:
	ShaderProgram();
	~ShaderProgram();

	void create(const std::string& vert, 
				const std::string& geom,
				const std::string& frag);

	void create(const std::string& vert,
				const std::string& frag);

	//void reload();

	void use();

	void uniformv(const std::string& name, GLuint count, const glm::mat4* matrices);
	void uniform(const std::string& name, const glm::mat4& matrix);

	void uniformv(const std::string& name, GLuint count, const GLfloat* values);
	void uniform(const std::string& name, const GLfloat value);

	void uniformv(const std::string& name, GLuint count, const glm::vec2* vectors);
	void uniform(const std::string& name, const glm::vec2& vector);

	void uniformv(const std::string& name, GLuint count, const glm::vec3* vectors);
	void uniform(const std::string& name, const glm::vec3& vector);

	void uniformv(const std::string& name, GLuint count, const glm::vec4* vectors);
	void uniform(const std::string& name, const glm::vec4& vector);

	void uniformv(const std::string& name, GLuint count, const GLint* values);
	void uniform(const std::string& name, const GLint value); 
};