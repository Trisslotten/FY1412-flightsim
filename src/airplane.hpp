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
			 glm::mat4 _transform, double _Cl0 = 0)
	{
		model = _model;
		transform = _transform;
		Cl0 = _Cl0;
	}
	std::shared_ptr<Model> model;
	glm::dmat4 transform;
	double Cl0;
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

	double reference_area = 0;
	glm::dvec3 drag_center;

	std::vector<Fuselage> fuselage_parts;
	std::vector<Wing> wings;

	void genInertiaTensor();

	void calcArea();
public:
	Body body;

	~Airplane();

	void init();

	void update(double dt, Engine& engine);

	virtual void draw(Renderer& renderer);
};