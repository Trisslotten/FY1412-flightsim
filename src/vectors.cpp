#include "vectors.hpp"

#include <glm/gtx/transform.hpp>

#define EPSILON 0.00000001f

void Vectors::init()
{
	body.load("vectorbody.obj");
	head.load("vectorhead.obj");

	body.uploadToGPU();
	head.uploadToGPU();
}

void Vectors::addVector(glm::vec3 pos, glm::vec3 vector, glm::vec3 color)
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
		
		glm::vec3 axis = cross(glm::vec3(0, 1, 0), normalize(v.vector));
		float angle = acosf(dot(glm::vec3(0, 1, 0), normalize(v.vector)));
		glm::mat4 rotation;
		glm::mat4 translation = glm::translate(v.pos);
		if (length(axis) > EPSILON)
		{
			rotation = glm::rotate(angle, axis);
		}
		float len = sqrt(length(v.vector));
		glm::mat4 body_trans = translation*rotation*glm::scale(glm::vec3(1,len,1));
		glm::mat4 head_trans = translation*rotation*glm::translate(glm::vec3(0, len, 0));
		renderer.setColor(glm::vec4(v.color.r, v.color.g, v.color.b, 1));
		renderer.draw(body, body_trans);
		renderer.draw(head, head_trans);
	}
}
