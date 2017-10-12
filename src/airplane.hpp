#pragma once

#include <memory>
#include <vector>
#include "model.hpp"
#include "body.hpp"
#include "renderer.hpp"
#include "lookup.hpp"
#include "keybind.hpp"

class Engine;


struct WingTransform
{
	WingTransform() {}

	WingTransform(glm::dmat4 _placement,
				  glm::dmat4 _scale)
	{
		placement = _placement;
		scale = _scale;
	}
	glm::dmat4 placement;
	glm::dmat4 scale;
};

struct Wing
{
	Wing(std::shared_ptr<Model> _model,
		 WingTransform _transforms,
		 std::shared_ptr<LookUpTable> _table, 
		 std::shared_ptr<Keybind> _keybind = nullptr)
	{
		model = _model;
		stalling = true;
		transforms = _transforms;
		table = _table;
		keybind = _keybind;
	}
	std::shared_ptr<Model> model;
	WingTransform transforms;

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

struct Powerplant {
	Powerplant() {

	};
	float calcPower(float velocity);
};

struct PropEngine {

	PropEngine() {

	}

	float diameter = 1.905f;
	float horse_power = 160.f;
	float a = 1.83f;
	float b = -1.32f;
	float max_rpm = 3000.f;
	float calcPower(float velocity, float throttle) {
		//return((efficency(advance_ratio(velocity, RPS())) * engine_power()) / (advance_ratio(velocity, RPS())*RPS()*diameter));
		return throttle*(engine_power() / (RPS()*diameter))*(a + (b*(glm::pow(velocity, 2) / (glm::pow(RPS(), 2)*glm::pow(diameter, 2)))));
		//float advance_ratio_var = advance_ratio(velocity);
		//return throttle * engine_power() * ((a + b * glm::pow(advance_ratio_var, 2)) / (RPS() * diameter));
	}
	float efficency(float advance_ratio) {
		return (a * advance_ratio) - (b * glm::pow(advance_ratio, 3));
	}
	float engine_power() {
		return horse_power * 750;
	}
	float RPS() {
		return (max_rpm)*(3.1415f / 30.f);
	}
	float advance_ratio(float velocity) {
		return velocity / (RPS() * diameter);
	}
};


class Airplane : public Drawable
{
	ShaderProgram shader;
	GLuint* depth_map;
	int area_resolution = 20;

	std::vector<Fuselage> fuselage_parts;
	std::vector<Wing> wings;
	std::vector<PropEngine*> engines;

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