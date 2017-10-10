#pragma once

#include <memory>
#include <vector>
#include "model.hpp"
#include "body.hpp"
#include "renderer.hpp"
#include "lookup.hpp"

class Engine;

struct Wing
{
	Wing(std::shared_ptr<Model> _model,
			 glm::mat4 _transform, LookUpTable* _table)
	{
		model = _model;
		transform = _transform;
		stalling = false;
		table = _table;
	}
	std::shared_ptr<Model> model;
	glm::dmat4 transform;
	bool stalling;
	LookUpTable* table;
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
	glm::dmat4 transform;
};

class Airplane : public Drawable
{

	ShaderProgram shader;
	GLfloat* depth_map;
	int area_resolution = 100;

	std::vector<Fuselage> fuselage_parts;
	std::vector<Wing> wings;


	void buildPlane();

	void calcLift(Wing& wing);
	void calcDrag();

	void genInertiaTensor();
public:
	Body body;

	~Airplane();

	void init();

	void update(double dt, Engine& engine);

	virtual void draw(Renderer& renderer);

	float throttle = 0.f;
};