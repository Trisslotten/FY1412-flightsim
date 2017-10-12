#pragma once


#include "window.hpp"
#include <glm/glm.hpp>

class Keybind
{

	int key1, key2;
	double change;


	double value = 0;
public:

	Keybind(int key1, int key2, double change);

	void update(Window& window);

	void set(double val)
	{
		value = glm::clamp(val, -1.0, 1.0)*change;
	}

	double getValue()
	{
		return value;
	}
};