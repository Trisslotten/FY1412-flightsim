#pragma once

#include <memory>
#include <vector>
#include "model.hpp"
#include "body.hpp"
#include "renderer.hpp"

class Engine;

struct Wing
{
	Wing(std::shared_ptr<Model> _model,
			 glm::mat4 _transform)
	{
		model = _model;
		transform = _transform;
	}
	std::shared_ptr<Model> model;
	glm::mat4 transform;
};

struct Fuselage
{
	Fuselage(std::shared_ptr<Model> _model,
				 glm::mat4 _transform)
	{
		model = _model;
		transform = _transform;
	}
	std::shared_ptr<Model> model;
	glm::mat4 transform;
};

class Airplane : public Drawable
{

	ShaderProgram shader;
	GLfloat* depth_map;
	int area_resolution = 100;


	std::vector<Fuselage> fuselage_parts;
	std::vector<Wing> wings;

	void genInertiaTensor();

	void calcArea();
public:
	Body body;

	~Airplane();

	void init();

	void update(float dt, Engine& engine);

	virtual void draw(Renderer& renderer);
};