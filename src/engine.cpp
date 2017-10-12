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
	terrain.init();
	terrain.update(glm::vec3());


	airplane.init();
	vectors.init();
	renderer.init(&window);
	
	clouds.init();
	renderer.setNearFarPlane(10, 50000);
}

void Engine::update()
{
	double frame_time = 0.25 / 144.0;
	double timescale = 1;
	double dt = timer.restart();
	if(dt < 1)
		frame_accum += dt;
	
	while (frame_accum > frame_time)
	{
		vectors.clear();
		texts.clear();

		// update physics

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		airplane.update(frame_time*timescale, *this);

		frame_accum -= frame_time;
	}

	camera->update(window, airplane.body.getTransform());
	terrain.update(airplane.body.position);
	clouds.update(airplane.body.position, terrain);
}

void Engine::render()
{	
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	renderer.initDrawing(window);

	renderer.draw(airplane);
	renderer.draw(terrain);

	renderer.drawSkybox();

	renderer.draw(clouds);

	glClear(GL_DEPTH_BUFFER_BIT);
	renderer.draw(vectors);

	glClear(GL_DEPTH_BUFFER_BIT);
	renderer.draw(texts);
	
	window.update();
}
