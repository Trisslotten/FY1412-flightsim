#include "window.hpp"
//#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

namespace
{
	std::map<GLFWwindow*, glm::vec2 > scrolls;

	void scrollCallback(GLFWwindow* window, double x, double y)
	{
		scrolls[window] += glm::vec2(x, y);
	}
}



Window::~Window()
{
	if(window)
		close();
}

void Window::open(unsigned width, unsigned height, bool fullscreen)
{

	if (window)
		return;

	/* Initialize the library */
	if (!glfwInit())
	{
		std::cout << "ERROR: Could not initialize glfw" << std::endl;
		exit(-1);
	}
		
	window = glfwCreateWindow(width, height, "Hello World", NULL, NULL);
	if (!window)
	{
		std::cout << "ERROR: Could not create glfw window" << std::endl;
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);

	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		// Problem: glewInit failed, something is seriously wrong.
		std::cout << "glewInit failed: " << glewGetErrorString(err) << std::endl;
		system("pause");
		exit(1);
	}
	scrolls[window] = glm::vec2(0, 0);
	glfwSetScrollCallback(window, scrollCallback);
}

double Window::getScroll()
{
	return scroll.y;
}

glm::ivec2 Window::getWindowSize() const
{
	glm::ivec2 size;
	glfwGetWindowSize(window, &size.x, &size.y);
	return size;
}

void Window::update()
{
	double frame_time = frame_timer.restart();

	if (second_timer.elapsed() >= 1)
	{
		second_timer.restart();
		int fps = glm::round(1 / frame_time);
		std::string title = "FPS: " + std::to_string(fps);
		glfwSetWindowTitle(window, title.c_str());
	}

	scroll = scrolls[window];
	scrolls[window] = glm::vec2(0,0);

	glfwSwapBuffers(window);
	glfwPollEvents();

	

}

void Window::close()
{ 
	glfwTerminate(); 
}

bool Window::keyDown(int key)
{
	return GLFW_PRESS == glfwGetKey(window, key);
}

glm::vec2 Window::mouseMovement()
{
	glm::vec2 pos = mousePosition();
	glm::vec2 result = pos - last_mouse_pos;
	last_mouse_pos = pos;
	return result;
}

glm::vec2 Window::mousePosition()
{
	glm::dvec2 pos;
	glfwGetCursorPos(window, &pos.x, &pos.y);
	return pos;
}
