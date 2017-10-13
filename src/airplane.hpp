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
	virtual float calcPower(float throttle) { return 0.f; };

	void update_conditions(float temp, float alt, float velocity) {
		this->temp = temp;
		this->alt = alt;
		this->velocity = velocity;
	}

	//flight conditions
	float temp = 0.f; // !!!in kelvin!!!
	float alt = 0.f;
	float velocity = 0.f;

	float spec = 0.f;

	//virtual std::string engine_type() = 0;
};

struct PropEngine : public Powerplant{

	PropEngine() : Powerplant(){

	}

	float diameter = 1.905f;
	float horse_power = 160.f;
	float a = 1.83f;
	float b = -1.32f;
	float max_rpm = 2400.f;
	const float C = glm::pow(0.12f,2);

	float calcPower(float throttle) {
		return throttle*((altitude_dropoff() * engine_power()) / (RPS()*diameter))*(a + (b*(glm::pow(velocity, 2) / (glm::pow(RPS(), 2)*glm::pow(diameter, 2)))));
	}
	float engine_power() { // Pe
		return horse_power * 745.7f;
	}
	float RPS() { // n
		return max_rpm/60;
	}
	float pressure() { // p
		return 101325.f*glm::pow(1 - (0.0065f * (alt / 288.15f)), 5.25f);
	}
	float density() { // P
		return 0.00348f * (pressure() / temp);
	}
	float density_ratio() {// P/P0
		float t = density();
		float temp = alt;
		alt = 0;
		float n = density();
		alt = temp;
		return t/n;
	}
	float altitude_dropoff() {
		spec = (density_ratio() - C) / (1 - C);
		return (density_ratio()-C) / (1-C);
	}


};

struct JetEngine : public Powerplant {
	JetEngine() : Powerplant() {

	}

	float calcPower(float velocity, float throttle) {
		return 0.f;
	}
};


class Airplane : public Drawable
{
	ShaderProgram shader;
	GLuint* depth_map;
	int area_resolution = 20;

	std::vector<Fuselage> fuselage_parts;
	std::vector<Wing> wings;
	std::vector<Powerplant*> engines;

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