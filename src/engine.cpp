#include "engine.hpp"

// used to check for when floats become NaN for debugging
//#include <float.h>
//unsigned int fp_control_state = _controlfp_s(nullptr, _EM_INEXACT, _MCW_EM);

#include <iostream>
#include <thread>
#include "timer.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtx/transform.hpp>

void Engine::init()
{
	window.open(1280, 720, false);
	camera = std::make_shared<Camera>();
	renderer.setCamera(camera);

	airplane.init();
	vectors.init();
	renderer.init(&window);

	cloud.load("cloud.obj");
	cloud.uploadToGPU();
}

void Engine::update()
{
	double timescale = 1.0;
	double dt = timer.restart();
	frame_accum += dt;
	
	double frame_time = 1/144.0;
	while (frame_accum > frame_time)
	{
		vectors.clear();
		// update physics

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		airplane.update(frame_time*timescale, *this);

		frame_accum -= frame_time;
	}

	camera->update(window, airplane.body.position);
}

void Engine::render()
{	
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	renderer.initDrawing(window);

	renderer.setColor(glm::vec4(0.8,0.8,0.8,1));
	renderer.draw(airplane);
	
	renderer.drawSkybox();

	glm::mat4 scale = glm::scale(glm::vec3(20));
	int side = 20;
	float spacing = 300.f;
	for (int i = 0; i < side*side; i++)
	{
		int x = i % side-side/2;
		int y = i / side-side/2;
		renderer.setColor(glm::vec4(1,1,1, 0.7));
		renderer.draw(cloud, glm::translate(spacing*glm::vec3(x, 0, y))*scale);
	}

	glClear(GL_DEPTH_BUFFER_BIT);
	renderer.draw(vectors);

	glClear(GL_DEPTH_BUFFER_BIT);
	renderer.draw(texts);
	
	window.update();
}
