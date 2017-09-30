#pragma once

#include <glm/glm.hpp>
#include "renderer.hpp"
#include "model.hpp"

class Vectors : public Drawable
{
	struct Vector
	{
		glm::dvec3 pos;
		glm::dvec3 vector;
		glm::vec3 color;
		Vector(glm::dvec3 pos, glm::dvec3 vector, glm::vec3 color) : pos(pos), vector(vector), color(color) {}
	};

	std::vector<Vector> vectors;

	Model body, head;

public:
	void init();

	void addVector(glm::dvec3 pos, glm::dvec3 vector, glm::vec3 color);

	void clear();

	virtual void draw(Renderer& renderer);
};