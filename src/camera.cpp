#include "camera.hpp"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include "window.hpp"

#include <iostream>

void Camera::update(Window& window, glm::vec3 _follow_pos)
{
	float scroll = window.getScroll();
	zoom -= scroll;
	zoom = glm::max(zoom, 1.f);
	follow_pos = _follow_pos;
	glm::vec2 delta = window.mouseMovement();

	pitch -= delta.y * 0.008;
	yaw -= delta.x * 0.008;
	pitch = glm::clamp(pitch, -glm::half_pi<float>()+0.01f, glm::half_pi<float>()-0.01f);


	glm::mat4 pitch_transform = glm::rotate(glm::mat4(), pitch, glm::vec3(0, 0, 1));
	glm::mat4 yaw_transform = glm::rotate(glm::mat4(), yaw, glm::vec3(0, 1, 0));

	glm::vec4 look_vec(1, 0, 0, 0);
	look_vec = yaw_transform*pitch_transform*look_vec;

	pos = follow_pos - glm::vec3(look_vec)*zoom;

	view = glm::lookAt(
		pos,
		follow_pos,
		glm::vec3(0, 1, 0)
	);
}
	
glm::mat4 Camera::getView()
{
	return view;
}
