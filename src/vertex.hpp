#pragma	once

#include <glm/glm.hpp>
#include <assimp/vector3.h>
#include <assimp/vector2.h>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coords;
	
	Vertex() {}

	Vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 tex);

	Vertex(aiVector3D pos, aiVector3D norm, aiVector2D texs)
	{
		position.x = pos.x;
		position.y = pos.y;
		position.z = pos.z;

		normal.x = norm.x;
		normal.y = norm.y;
		normal.z = norm.z;

		tex_coords.x = texs.x;
		tex_coords.y = texs.y;
	}
};