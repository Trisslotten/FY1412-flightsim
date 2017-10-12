#include "airplane.hpp"
#include "renderer.hpp"
#include <glm/gtx/transform.hpp>
#include <iostream>
#include "engine.hpp"
#include "lookup.hpp"
#include "glm/gtc/matrix_transform.hpp"

using namespace glm;

const double EPSILON = 1000000.0 * epsilon<double>();
double density = 1.23;
double dynamic_viscosity = 0.0000183;

// assume origo is in line
dvec3 projectOnLine(dvec3 to_project, dvec3 v)
{
	v = normalize(v);
	return dot(to_project, v)*v;
}
// assume origo is in plane 
dvec3 projectToPlane(dvec3 to_project, dvec3 v1, dvec3 v2)
{
	dvec3 normal = normalize(cross(v1, v2));
	dvec3 on_normal = projectOnLine(to_project, normal);
	return to_project - on_normal;
}

Airplane::~Airplane()
{
	delete[] depth_map;
}

void Airplane::init()
{
	depth_map = new GLfloat[area_resolution*area_resolution];
	/*
	std::string NACA2415paths[] = { "NACA 2415 R50.txt", "NACA 2415 R100.txt","NACA 2415 R200.txt" ,"NACA 2415 R500.txt" ,"NACA 2415 R1000.txt" };
	LookUpTable lut(NACA2415paths);

	lut.lookUp(6.577, 40000);
	lut.lookUp(6.577654654645, 75000);
	lut.lookUp(6.577546546546, 300000);
	lut.lookUp(6.577, 500000);
	lut.lookUp(50, 900000);
	*/
	buildPlane();

	genInertiaTensor();
}

void Airplane::buildPlane()
{
	std::shared_ptr<Model> wing = std::make_shared<Model>();
	std::shared_ptr<Model> fuselage = std::make_shared<Model>();
	wing->load("wing.obj");
	fuselage->load("cylinder.obj");
	wing->uploadToGPU();
	fuselage->uploadToGPU();


	std::string NACA2415paths[] = { "NACA 2415 R50.txt", "NACA 2415 R100.txt","NACA 2415 R200.txt" ,"NACA 2415 R500.txt" ,"NACA 2415 R1000.txt" };
	std::shared_ptr<LookUpTable> lut = std::make_shared<LookUpTable>(NACA2415paths);
	std::string HT12paths[] = { "HT12 R50.txt", "HT12 R100.txt","HT12 R200.txt" ,"HT12 R500.txt" ,"HT12 R1000.txt" };
	std::shared_ptr<LookUpTable> back_lut = std::make_shared<LookUpTable>(HT12paths);


	keybinds["l_aileron"] = std::make_shared<Keybind>(GLFW_KEY_A, GLFW_KEY_D, glm::radians(back_lut->maxAngle(0)));
	keybinds["r_aileron"] = std::make_shared<Keybind>(GLFW_KEY_D, GLFW_KEY_A, glm::radians(back_lut->maxAngle(0)));

	keybinds["elevator"] = std::make_shared<Keybind>(GLFW_KEY_S, GLFW_KEY_W, glm::radians(lut->maxAngle(0)));


	dmat4 fuselage_t = scale(dvec3(8, 1, 1));

	WingTransform l_wing_t(translate(dvec3(-3, 0, -4))*rotate(0.0, dvec3(1, 0, 0)),
						   scale(dvec3(1, 1, 4)));
	WingTransform r_wing_t(translate(dvec3(-3, 0, 4))*rotate(-0.0, dvec3(1, 0, 0)),
						   scale(dvec3(1, 1, 4)));

	WingTransform l_aileron_t(translate(dvec3(-4.2, 0, -7))*rotate(0.0, dvec3(1, 0, 0)),
							  scale(dvec3(0.2, 0.1, 1)));
	WingTransform r_aileron_t(translate(dvec3(-4.2, 0, 7))*rotate(0.0, dvec3(1, 0, 0)),
							  scale(dvec3(0.2, 0.1, 1)));


	WingTransform l_hori_t(translate(dvec3(-7, 1, -2)),
						   scale(dvec3(1, 1, 2)));
	WingTransform r_hori_t(translate(dvec3(-7, 1, 2)),
						   scale(dvec3(1, 1, 2)));
	WingTransform vert_t(translate(dvec3(-7, 3, 0))*rotate(half_pi<double>(), dvec3(1, 0, 0)),
						 scale(dvec3(1, 1, 2)));

	fuselage_parts.emplace_back(fuselage, fuselage_t);

	wings.emplace_back(wing, l_wing_t, lut);
	wings.emplace_back(wing, r_wing_t, lut);
	wings.emplace_back(wing, l_aileron_t, lut, keybinds["l_aileron"]);
	wings.emplace_back(wing, r_aileron_t, lut, keybinds["r_aileron"]);

	wings.emplace_back(wing, l_hori_t, lut, keybinds["elevator"]);
	wings.emplace_back(wing, r_hori_t, lut, keybinds["elevator"]);
	wings.emplace_back(wing, vert_t, back_lut);

	body.setMass(5000);
	body.position = dvec3(-600, 1000, 0);
}

void Airplane::calcLift(Wing & wing)
{
	double angle = 0;
	dmat4 init_offset, return_offset;
	if (wing.keybind)
	{
		angle = wing.keybind->getValue();
	}
	dmat4 rotation = glm::rotate(angle, dvec3(0,0,1));
	wing.transform = wing.transforms.placement * rotation * wing.transforms.scale;


	dmat4 world_trans = body.getTransform()*wing.transform;

	dvec3 diag1 = wing.transform*dvec4(1, 0, 1, 0);
	dvec3 diag2 = wing.transform*dvec4(-1, 0, 1, 0);
	dvec3 half_wing_length = wing.transform*dvec4(0, 0, 1, 0);
	dvec3 half_wing_chord = wing.transform*dvec4(1, 0, 0, 0);
	double area = 2 * length(cross(diag1, diag2));
	double chord_length = 2 * length(half_wing_chord);
	double wing_length = 2 * length(half_wing_length);

	// wing local space base vectors in world
	dvec3 ex = normalize(world_trans*dvec4(1, 0, 0, 0));
	dvec3 ey = normalize(world_trans*dvec4(0, 1, 0, 0));
	dvec3 ez = normalize(world_trans*dvec4(0, 0, 1, 0));

	dvec3 wing_pos = vec3(world_trans*vec4(0, 0, 0, 1));
	dvec3 vel = body.velocityAt(wing_pos);

	dvec3 alpha_vel = projectToPlane(vel, ex, ey);
	dvec3 beta_vel = projectToPlane(vel, ez, ex);

	double angle_of_attack = 0;
	if (length(alpha_vel) > EPSILON)
	{
		double cosa = dot(normalize(alpha_vel), ex);
		double radians = 0;
		if(cosa < 1.0)
			radians = glm::acos(cosa);
		angle_of_attack = degrees(radians);
		if (dot(alpha_vel, ey) > 0)
			angle_of_attack *= -1;

		double v = length(vel);
		double Re = density*chord_length*v / dynamic_viscosity;
		Re = 100000;
		//std::cout << "Re: " << Re << "\n";

		double min_ang = wing.table->minAngle(Re);
		double max_ang = wing.table->maxAngle(Re);
		/*
		std::cout << "Min aot: " << min_ang << "\n";
		std::cout << "Max aot: " << max_ang << "\n";
		std::cout << "aot: " << angle_of_attack << "\n";
		std::cout << "\n";
		*/
		if (angle_of_attack > min_ang && angle_of_attack < max_ang)
		{
			wingData data = wing.table->lookUp(angle_of_attack, Re);

			double Cl = data.cl;
			
				
			double lift = 0.5*density*v*v*area*Cl;
			//std::cout << "alpha: " << angle_of_attack << ", Cl: " << Cl << ", lift: " <<  lift << "\n";


			double drag = 0.5*density*v*v*area*data.cd;

			dvec3 center_of_pressure = wing_pos + dvec3(body.getTransform()*dvec4(0.5*half_wing_chord, 0));

			dvec3 lift_dir = ey - dot(alpha_vel, ey)*alpha_vel / dot(alpha_vel, alpha_vel);
			if (length(lift_dir) > EPSILON)
			{
				lift_dir = normalize(lift_dir);
				body.applyForce(lift*lift_dir, center_of_pressure);
			}
			if (v > EPSILON)
				body.applyForce(-drag*normalize(vel), center_of_pressure);

			wing.stalling = false;
		}
		else
		{
			wing.stalling = true;
		}
	}

	

	// DEBUG
	//engine.getVectors().addVector(wing_pos, alpha_vel, dvec3(1, 0, 0));
	//engine.getVectors().addVector(wing_pos, ex, dvec3(0, 1, 1));
	//engine.getVectors().addVector(wing_pos, ey, dvec3(0, 1, 1));
	//engine.getVectors().addVector(wing_pos, ez, dvec3(0, 1, 1));
}


void Airplane::calcDrag()
{
	// side length in pixels to draw
	int resolution = area_resolution;
	// length of side of cube that surrounds the model in world space
	double length = 20;
	double pixel_size = length / resolution;
	glViewport(0, 0, resolution, resolution);

	dmat4 projection = ortho(-length / 2, length / 2, -length / 2, length / 2, 0.001, length);

	dvec3 vel = body.velocityAt(body.position);
	if (glm::length(vel) < EPSILON)
		return;

	dmat4 body_transform = body.getTransform();

	dvec3 eye = 0.5*length*normalize(vel);
	dvec3 up = vec3(0, 1, 0);
	if (glm::length(cross(eye, up)) <= EPSILON)
		up = vec3(1, 0, 0);

	dmat4 view = lookAt(eye + body.position, body.position, up);

	shader.use();
	shader.uniform("projection", projection);
	shader.uniform("view", view);

	glClear(GL_DEPTH_BUFFER_BIT);
	for (auto&& part : fuselage_parts)
		recursiveDraw(part.model->getRootNode(), mat4(body_transform * part.transform), shader);
	for (auto&& wing : wings)
	{
		if (wing.stalling)
			recursiveDraw(wing.model->getRootNode(), mat4(body_transform * wing.transform), shader);
	}


	glReadPixels(0, 0, resolution, resolution, GL_DEPTH_COMPONENT, GL_FLOAT, depth_map);

	double area = pixel_size*pixel_size;

	glm::dmat4 inverse = glm::inverse(projection * view);

	unsigned int num_pixels = 0;
	dvec3 accum_pos;
	for (int yi = 0; yi < resolution; yi++)
	{
		for (int xi = 0; xi < resolution; xi++)
		{
			float depth = depth_map[xi + yi * resolution];
			double x = xi;
			double y = yi;
			if (depth < 1.f)
			{
				num_pixels++;
				dvec3 pos;
				pos.x = x + 0.5;
				pos.y = y + 0.5;
				pos.z = depth;

				// TESTING
				/*
				double speed = glm::length(body.velocityAt(world));
				if (speed > epsilon<double>()) 
				{
					double CD = 0.1;
					double drag_magn = 0.5 * density * speed * speed * CD * area;
					dvec3 drag_force = -drag_magn * normalize(body.velocityAt(world));
					body.applyForce(drag_force, world, false);
				}
				*/
				// TESTING

				accum_pos += pos;
			}
		}
	}
	/**/
	dvec3 drag_center = body.position;
	double reference_area = pixel_size*pixel_size*num_pixels;
	if (num_pixels > 0)
	{

		drag_center = accum_pos / (double)num_pixels;

		glm::dvec4 tmp = glm::dvec4(drag_center, 1);
		tmp.x /= resolution;
		tmp.y /= resolution;
		tmp = tmp * 2.0 - 1.0;
		glm::dvec4 world = inverse*tmp;
		world /= world.w;

		drag_center = world;
	}

	double speed = glm::length(body.velocityAt(body.position));
	if (speed > EPSILON)
	{
		double CD = 0.1;
		double drag_magn = 0.5 * density * speed * speed * CD * reference_area;
		dvec3 drag_force = -drag_magn * normalize(body.velocityAt(drag_center));
		body.applyForce(drag_force, drag_center);
	}
}


void Airplane::update(double dt, Engine& engine)
{
	body.update(dt);

	Window& w = engine.getWindow();
	for (auto&& map_elem : keybinds)
		map_elem.second->update(w);


	if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
		int axises;
		const float* axis = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axises);
		//std::cout << "Roll: " << axis[0] << std::endl;
		//std::cout << "Pitch: " << axis[1] << std::endl;
		//std::cout << "Turn: " << axis[2] << std::endl;

		throttle = 1.f-((axis[2] + 1.f)/2);

		//std::cout << "Throttle: " << throttle << std::endl;
		//std::cout << "axis[2]: " << axis[2] << std::endl;

		if (abs(axis[0]) > 0.1f) {
			keybinds["l_aileron"]->set(sign(axis[0]) * axis[0]* axis[0]);
			keybinds["r_aileron"]->set(-sign(axis[0]) * axis[0] * axis[0]);
		}
		if (abs(axis[1]) > 0.1f) {
			keybinds["elevator"]->set(-sign(axis[1]) * axis[1] * axis[1]);
		}
	}
	


	if (w.keyDown(GLFW_KEY_LEFT_SHIFT))
	{
		throttle += 1.f / 240.f;
	}
	if (w.keyDown(GLFW_KEY_LEFT_CONTROL))
	{
		throttle -= 1.f / 240.f;
	}

	throttle = clamp(throttle, 0.f, 1.f);

	std::cout << "New Update\n";
	for (auto& wing : wings)
	{
		calcLift(wing);
	}
	calcDrag();

	dvec3 forward = body.getTransform()*dvec4(1, 0, 0, 0);

	body.applyForce(throttle*100000.0*forward, body.position);

	body.applyForce(dvec3(0, -9.82*body.mass, 0), body.position);

	for (auto&& f : body.applied_forces)
	{
		engine.getVectors().addVector(f.first, f.second, vec3(1, 0, 1));
	}

	
	/*
	std::string pos_text = "Position: (";
	pos_text +=        std::to_string(body.position.x);
	pos_text += ", " + std::to_string(body.position.y);
	pos_text += ", " + std::to_string(body.position.z) + ")";
	engine.getTexts().addText(0,0, pos_text);
	*/
	std::string gs_text = "G: " + std::to_string(glm::length(body.external_forces/(9.82*body.mass))) + " m/s^2";
	engine.getTexts().addText(0, 16, gs_text);

	std::string speed_text = "Speed: " + std::to_string(glm::length(body.velocityAt(body.position))) + " m/s";
	engine.getTexts().addText(0, 0, speed_text);

	/*
	dvec3 v = body.velocityAt(body.position);
	dvec3 pos = body.position + log(length(v))*normalize(v);
	engine.getTexts().addText(pos, std::to_string((int)length(v)) + " m/s");

	dvec3 w = body.inverse_inertia*body.angular_momentum;
	pos = body.position + normalize(w); log(length(w))*normalize(w);
	engine.getTexts().addText(pos, std::to_string(60.f*length(w)/two_pi<float>()) + " rpm");
	*/
	//engine.getVectors().addVector(body.position, w, dvec3(0, 1, 0));
	engine.getVectors().addVector(body.position, body.velocityAt(body.position), dvec3(1, 0, 0));

	
}


void Airplane::draw(Renderer & renderer)
{
	renderer.setColor(glm::vec4(0.8, 0.8, 0.8, 1));
	for (auto&& part : fuselage_parts)
		renderer.draw(*part.model, mat4(body.getTransform()*part.transform));
	for (auto&& wing : wings)
		renderer.draw(*wing.model, mat4(body.getTransform()*wing.transform));
}

void Airplane::genInertiaTensor()
{
	// side length in pixels to draw
	const int resolution = 700;
	// length of side of cube that surrounds the model in world space
	const double length = 20;
	double pixel_size = length / resolution;
	glViewport(0, 0, resolution, resolution);

	dmat4 projection = ortho(-length / 2, length / 2, -length / 2, length / 2, 0.001, length);
	dmat4 view = lookAt(dvec3(0, 0, -length / 2), dvec3(0, 0, 0), dvec3(0, 1, 0));

	shader.create("inertia.vert",
				  "inertia.frag");

	shader.use();
	shader.uniform("projection", mat4(projection));
	shader.uniform("view", mat4(view));

	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	for (auto&& part : fuselage_parts)
		recursiveDraw(part.model->getRootNode(), mat4(part.transform), shader);
	for (auto&& wing : wings)
		recursiveDraw(wing.model->getRootNode(), mat4(wing.transform), shader);

	GLfloat* depths = new GLfloat[resolution*resolution];
	glReadPixels(0, 0, resolution, resolution, GL_DEPTH_COMPONENT, GL_FLOAT, depths);

	double Ixx = 0;
	double Iyy = 0;
	double Izz = 0;
	double Ixy = 0;
	double Ixz = 0;
	double Iyz = 0;

	// assumes all cubes have the same weight
	int num_cubes = 0;

	for (int yi = 0; yi < resolution; yi++)
	{
		for (int xi = 0; xi < resolution; xi++)
		{
			double depth = depths[xi + yi * resolution];
			if (depth == 1.f)
				continue;


			double x = xi;
			double y = yi;

			dvec3 pos;
			pos.x = length*(resolution - x) / resolution - length / 2;
			pos.y = length*y / resolution - length / 2;
			pos.z = depth*length - length / 2;

			dvec3 wp = pos;

			// loop through all "cubes" behind till z = 0;
			wp.z += 0.5*pixel_size;
			while (wp.z<0)
			{
				num_cubes += 2;

				// assume symmetry
				vec3 op(wp.x, wp.y, -wp.z);

				Ixx += wp.y*wp.y + wp.z*wp.z;
				Ixx += op.y*op.y + op.z*op.z;

				Iyy += wp.x*wp.x + wp.z*wp.z;
				Iyy += op.x*op.x + op.z*op.z;

				Izz += wp.x*wp.x + wp.y*wp.y;
				Izz += op.x*op.x + op.y*op.y;

				Ixy -= wp.x*wp.y;
				Ixy -= op.x*op.y;

				Ixz -= wp.x*wp.z;
				Ixz -= op.x*op.z;

				Iyz -= wp.y*wp.z;
				Iyz -= op.y*op.z;

				wp.z += pixel_size;
			}
		}
	}
	dmat3 tensor(Ixx, Ixy, Ixz,
				 Ixy, Iyy, Iyz,
				 Ixz, Iyz, Izz);
	tensor /= num_cubes;
	body.setInertia(body.mass*tensor);

	delete[] depths;
}
