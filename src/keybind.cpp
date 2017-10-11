#include "keybind.hpp"

#include <glm/glm.hpp>

Keybind::Keybind(int _key1, int _key2, double _change)
{
	key1 = _key1;
	key2 = _key2;
	change = _change;
}

void Keybind::update(Window & w)
{
	double ratio = 0.03;
	bool pressed = false;
	if (w.keyDown(key1))
	{
		pressed = true;
		value -= change*ratio;
	}
	if (w.keyDown(key2))
	{
		pressed = true;
		value += change*ratio;
	}
	if (!pressed)
	{
		value -= glm::sign(value)*change*ratio;
		if (abs(value) <= change*ratio)
		{
			value = 0;
		}
	}
	value = glm::clamp(value, -change, change);
}
