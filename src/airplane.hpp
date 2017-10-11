#pragma once

#include <memory>
#include <vector>
#include "model.hpp"
#include "body.hpp"
#include "renderer.hpp"
#include "lookup.hpp"
#include "keybind.hpp"

class Engine;

struct Wing
{
	Wing(std::shared_ptr<Model> _model,
			 glm::mat4 _transform, std::shared_ptr<LookUpTable> _table, std::shared_ptr<Keybind> _keybind = nullptr)
	{
		model = _model;
		transform = _transform;
		stalling = false;
		table = _table;
		keybind = _keybind;
	}
	std::shared_ptr<Model> model;
	glm::dmat4 transform;
	bool stalling;
	std::shared_ptr<LookUpTable> table;
	std::shared_ptr<Keybind> keybind;
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

	std::unordered_map<std::string, std::shared_ptr<Keybind>> keybinds;

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