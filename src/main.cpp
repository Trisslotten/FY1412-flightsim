#include "window.hpp"
#include "engine.hpp"
#include <iostream>


int main(void)
{
	Engine engine;
	engine.init();
	while (!engine.shouldExit())
	{
		engine.update();
		engine.render(); 
	}
	exit(0);
	return 0;
}