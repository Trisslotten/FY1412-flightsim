#include "shader.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>

GLuint compileShader(GLenum type, std::string name)
{

	GLuint shader = glCreateShader(type);
	std::ifstream shaderFile(name);
	std::string shaderText((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
	if (!shaderFile.is_open())
	{
		std::cout << "ERROR: Could not open file: " << name << "\n";
		system("pause");
		exit(1);
	}

	shaderFile.close();
	const char* shaderTextPtr = shaderText.c_str();

	glShaderSource(shader, 1, &shaderTextPtr, nullptr);
	glCompileShader(shader);

	// Check for compile error
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint log_size = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
		std::vector<GLchar> error(log_size);
		glGetShaderInfoLog(shader, log_size, &log_size, &error[0]);
		std::string errorstr{ &error[0] };

		std::cout << errorstr << "\n";

		glDeleteShader(shader);
		system("pause");
		exit(1);
	}

	return shader;
}

ShaderProgram::ShaderProgram()
{
	vs = 0;
	gs = 0;
	fs = 0;
}

ShaderProgram::~ShaderProgram()
{
	glDeleteShader(vs);
	glDeleteShader(gs);
	glDeleteShader(fs);
	glDeleteProgram(id);
}

void ShaderProgram::create(const std::string & vert, const std::string & geom, const std::string & frag)
{
	vs = compileShader(GL_VERTEX_SHADER, vert);
	gs = compileShader(GL_GEOMETRY_SHADER, geom);
	fs = compileShader(GL_FRAGMENT_SHADER, frag);

	id = glCreateProgram();
	glAttachShader(id, fs);
	glAttachShader(id, gs);
	glAttachShader(id, vs);
	glLinkProgram(id);
}

void ShaderProgram::create(const std::string & vert, const std::string & frag)
{
	GLuint vs = compileShader(GL_VERTEX_SHADER, vert);
	GLuint fs = compileShader(GL_FRAGMENT_SHADER, frag);

	id = glCreateProgram();
	glAttachShader(id, vs);
	glAttachShader(id, fs);
	glLinkProgram(id);
}


void ShaderProgram::use()
{
	glUseProgram(id);
}


GLuint ShaderProgram::findUniformID(const std::string & name)
{
	auto it = uniform_ids.find(name);
	GLuint uniform_id;
	if (it == uniform_ids.end())
	{
		uniform_id = glGetUniformLocation(id, name.c_str());
		uniform_ids[name] = uniform_id;
	}
	else
		uniform_id = it->second;
	return uniform_id;
}


void ShaderProgram::uniformv(const std::string & name, GLuint count, const glm::mat4* matrices)
{
	glUniformMatrix4fv(findUniformID(name), count, GL_FALSE, (GLfloat*)matrices);
}
void ShaderProgram::uniform(const std::string & name, const glm::mat4& matrix)
{
	uniformv(name, 1, &matrix);
}


void ShaderProgram::uniformv(const std::string & name, GLuint count, const GLfloat* values)
{
	glUniform1fv(findUniformID(name), count, (GLfloat*)values);
}
void ShaderProgram::uniform(const std::string & name, GLfloat value)
{
	uniformv(name, 1, &value);
}


void ShaderProgram::uniformv(const std::string & name, GLuint count, const glm::vec2* vectors)
{
	glUniform2fv(findUniformID(name), count, (GLfloat*)vectors);
}
void ShaderProgram::uniform(const std::string & name, const glm::vec2& vector)
{
	uniformv(name, 1, &vector);
}


void ShaderProgram::uniformv(const std::string & name, GLuint count, const glm::vec3* vectors)
{
	glUniform3fv(findUniformID(name), count, (GLfloat*)vectors);
}
void ShaderProgram::uniform(const std::string & name, const glm::vec3& vector)
{
	uniformv(name, 1, &vector);
}

void ShaderProgram::uniformv(const std::string & name, GLuint count, const glm::vec4* vectors)
{
	glUniform4fv(findUniformID(name), count, (GLfloat*)vectors);
}
void ShaderProgram::uniform(const std::string & name, const glm::vec4& vector)
{
	uniformv(name, 1, &vector);
}




void ShaderProgram::uniformv(const std::string & name, GLuint count, const GLint* values)
{
	glUniform1iv(findUniformID(name), count, values);
}
void ShaderProgram::uniform(const std::string & name, GLint value)
{
	uniformv(name, 1, &value);
}
