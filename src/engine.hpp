#pragma once

#include "model.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "airplane.hpp"
#include "vectors.hpp"

class Engine
{
	Window window;

	std::shared_ptr<Camera> camera;

	Renderer renderer;

	Airplane airplane;

	Model cloud;

	Timer global_time;
	Timer timer;
	float frame_accum;
	
public:
	Vectors vectors;

	Engine() {}

	void init();

	void update();
	
	void render();

	bool shouldExit()
	{
		return window.shouldClose();
	}
};