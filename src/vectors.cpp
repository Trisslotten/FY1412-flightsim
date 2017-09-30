#include "vectors.hpp"

#include <glm/gtx/transform.hpp>

#define EPSILON 0.0000000000001

void Vectors::init()
{
	body.load("vectorbody.obj");
	head.load("vectorhead.obj");

	body.uploadToGPU();
	head.uploadToGPU();
}

void Vectors::addVector(glm::dvec3 pos, glm::dvec3 vector, glm::vec3 color)
{
	if(length(vector) > EPSILON)
		vectors.emplace_back(pos, vector, color);
}

void Vectors::clear()
{
	vectors.clear();
}



void Vectors::draw(Renderer & renderer)
{
	for (auto&& v : vectors)
	{
		
		glm::dvec3 axis = cross(glm::dvec3(0, 1, 0), normalize(v.vector));
		double angle = acosf(dot(glm::dvec3(0, 1, 0), normalize(v.vector)));
		glm::dmat4 rotation;
		glm::dmat4 translation = glm::translate(v.pos);
		if (length(axis) > EPSILON)
		{
			rotation = glm::rotate(angle, axis);
		}
		double len = pow(length(v.vector), 1/5.0);
		glm::dmat4 body_trans = translation*rotation*glm::scale(glm::dvec3(1,len,1));
		glm::dmat4 head_trans = translation*rotation*glm::translate(glm::dvec3(0, len, 0));
		renderer.setColor(glm::vec4(v.color.r, v.color.g, v.color.b, 1));
		renderer.draw(body, glm::mat4(body_trans));
		renderer.draw(head, glm::mat4(head_trans));
	}
}
