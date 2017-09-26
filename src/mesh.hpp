#pragma once

#include "vertex.hpp"
#include "texture.hpp"
#include <vector>
#include <string>
#include <GL/glew.h>
#include <assimp/mesh.h>

class Mesh
{
	std::string name = "";

	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	GLuint ebo = 0;
	GLuint vbo = 0;
	GLuint vao = 0;

public:
	std::vector<Vertex> vertices;

	Mesh(aiMesh* mesh);
	Mesh(const std::vector<Vertex>& vertices,
		 const std::vector<GLuint>& indices);

	unsigned int numIndices()
	{
		return indices.size();
	}

	void uploadToGPU();
	void removeFromGPU();

	void bind();
	void unbind();
};