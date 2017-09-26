#pragma once

#include "model.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "airplane.hpp"
#include "vectors.hpp"
#include "texts.hpp"

class Engine
{
	Window window;

	std::shared_ptr<Camera> camera;

	Renderer renderer;

	Vectors vectors;
	Texts texts;

	Airplane airplane;

	Model cloud;

	Timer global_time;
	Timer timer;
	float frame_accum;
	
public:
	Texts& getTexts() { return texts; }
	Vectors& getVectors() { return vectors; }

	Engine() {}

	void init();

	void update();
	
	void render();

	bool shouldExit()
	{
		return window.shouldClose();
	}
};