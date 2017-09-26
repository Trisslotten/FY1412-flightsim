#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "timer.hpp"

	
class Window
{
	GLFWwindow* window;
	
	Timer frame_timer, second_timer;

	glm::vec2 scroll;

	glm::vec2 last_mouse_pos;
public:

	Window() {}

	~Window();

	void open(unsigned width, unsigned height, bool fullscreen);

	bool shouldClose()
	{
		return glfwWindowShouldClose(window);
	}

	double getScroll();
	glm::ivec2 getWindowSize() const;

	void update();

	void close();

	bool keyDown(int key);

	glm::vec2 mouseMovement();
	glm::vec2 mousePosition();
};

