#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <GL/glew.h>
#include "window.hpp"

class Camera {

	GLfloat zoom = 22.f;

	glm::vec3 follow_pos;

	glm::mat4 view;
	
	// left right
	double yaw;

	// up down
	double pitch;

	glm::vec3 pos{ 0,0,0 };

public:


	void update(Window& window, glm::dmat4 plane_trans);

	glm::vec3 getPos()
	{
		return pos;
	}

	glm::mat4 getView();

};