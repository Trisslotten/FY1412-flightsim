#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <GL/glew.h>
#include "window.hpp"

class Camera {

	GLfloat zoom = 1.f;

	glm::vec3 follow_pos;

	glm::mat4 view;
	
	// left right
	GLfloat yaw;

	// up down
	GLfloat pitch;

	glm::vec3 pos{ 0,0,0 };

public:
	
	
	void update(Window& window, glm::vec3 follow_pos);

	glm::vec3 getPos()
	{
		return pos;
	}

	glm::mat4 getView();

};