#include "camera.hpp"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include "window.hpp"

#include <iostream>

void Camera::update(Window& window, glm::dmat4 plane_trans)
{
	double scroll = window.getScroll();
	zoom -= scroll;
	zoom = glm::max(zoom, 1.f);
	
	//follow_pos = _follow_pos;



	glm::vec2 delta = window.mouseMovement();

	pitch -= delta.y * 0.008;
	yaw -= delta.x * 0.008;
	pitch = glm::clamp(pitch, -glm::half_pi<double>()+0.1, glm::half_pi<double>()-0.1);


	glm::dmat4 pitch_transform = glm::rotate(glm::dmat4(), pitch, glm::dvec3(0, 0, 1));
	glm::dmat4 yaw_transform = glm::rotate(glm::dmat4(), yaw, glm::dvec3(0, 1, 0));

	glm::vec3 look_vec = plane_trans*yaw_transform*pitch_transform*glm::vec4(zoom*glm::vec3(-1, 0, 0), 1);
	glm::vec3 up = plane_trans*glm::dvec4(glm::vec3(0, 1, 0), 0);
	glm::vec3 plane_pos = plane_trans*glm::dvec4(glm::vec3(0, 0, 0), 1);

	view = glm::lookAt(
		look_vec,
		plane_pos,
		up
	);
}
	
glm::mat4 Camera::getView()
{
	return view;
}
