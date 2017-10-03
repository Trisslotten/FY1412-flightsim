#include "airplane.hpp"
#include "renderer.hpp"
#include <glm/gtx/transform.hpp>
#include <iostream>
#include "engine.hpp"

using namespace glm;

double density = 1.23;

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

	dmat4 fuselage_t = scale(dvec3(8, 1, 1));

	dmat4 l_wing_t = translate(dvec3(-1, 0, -4))*rotate(0.0, dvec3(1, 0, 0))*scale(dvec3(1.3, 1, 4));
	dmat4 r_wing_t = translate(dvec3(-1, 0, 4))*rotate(0.0, dvec3(1, 0, 0))*scale(dvec3(1.3, 1, 4));

	dmat4 l_hori_t = translate(dvec3(-7, 1, -2))*scale(dvec3(1, 1, 2));
	dmat4 r_hori_t = translate(dvec3(-7, 1, 2))*scale(dvec3(1, 1, 2));
	dmat4 vert_t = translate(dvec3(-7, 2, 0))*rotate(half_pi<double>(), dvec3(1, 0, 0))*scale(dvec3(1, 1, 1));

	fuselage_parts.emplace_back(fuselage, fuselage_t);
	wings.emplace_back(wing, l_wing_t, 1);
	wings.emplace_back(wing, r_wing_t, 1);

	wings.emplace_back(wing, l_hori_t, -1);
	wings.emplace_back(wing, r_hori_t, -1);
	wings.emplace_back(wing, vert_t);

	body.setMass(3000);
	body.position = dvec3(-400, 400, 0);
}

void Airplane::calcLift(Wing & wing)
{
	dmat4 world_trans = body.getTransform()*wing.transform;

	dvec3 diag1 = wing.transform*dvec4(0.5, 0, 0.5, 0);
	dvec3 diag2 = wing.transform*dvec4(-0.5, 0, 0.5, 0);
	dvec3 half_wing_length = wing.transform*dvec4(0, 0, 0.5, 0);
	double area = 2 * length(cross(diag1, diag2));
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
	if (length(alpha_vel) > epsilon<double>())
	{
		double cosa = dot(normalize(alpha_vel), ex);
		double radians = std::acos(cosa);
		angle_of_attack = degrees(radians);
		if (dot(alpha_vel, ey) > 0)
			angle_of_attack *= -1;
	}

	double v = length(vel);
	double Cla = 1;//two_pi<float>();
	double Cl = Cla * angle_of_attack + wing.Cl0;


	double lift = 0;
	if (angle_of_attack > -20 && angle_of_attack < 20)
	{
		lift = 0.5*density*v*v*area*Cl;

		double induced_drag = 2.0*lift*lift / (density * v * v * glm::pi<double>() * wing_length * wing_length);

		dvec3 lift_dir = ey - dot(alpha_vel, ey)*alpha_vel / dot(alpha_vel, alpha_vel);
		if (length(lift_dir) > epsilon<double>())
		{
			lift_dir = normalize(lift_dir);
			body.applyForce(lift*lift_dir, wing_pos);
		}
		if (v > epsilon<double>())
			//body.applyForce(-induced_drag*normalize(vel), wing_pos);

		wing.stalling = false;
	}
	else
	{
		wing.stalling = true;
	}

	// DEBUG
	//engine.getVectors().addVector(wing_pos, normalize(alpha_vel), dvec3(1, 0, 0));
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

	dvec3 vel = body.momentum;
	if (glm::length(vel) < epsilon<double>())
		return;

	dmat4 body_transform = body.getTransform();

	dvec3 eye = 0.5*length*normalize(vel);
	dvec3 up = vec3(0, 1, 0);
	if (glm::length(cross(eye, up)) <= epsilon<double>())
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
				//pos.x = 2.0 * x / resolution - 1.0;
				//pos.y = 2.0 * y / resolution - 1.0;
				//pos.z = 2.0 * depth - 1.0;
				pos.x = x;
				pos.y = y;
				pos.z = depth;
				accum_pos += pos;
			}
		}
	}
	dvec3 drag_center = body.position;
	double reference_area = 0;
	if (num_pixels > 0)
	{
		drag_center = accum_pos / (double)num_pixels;
		drag_center = unProject(drag_center, view, projection, vec4(0, 0, resolution, resolution));
		reference_area = pixel_size*pixel_size*num_pixels;
	}

	double speed = glm::length(body.velocityAt(body.position));
	if (speed > epsilon<double>())
	{
		double CD = 1;
		double drag_magn = 0.5 * density * speed * speed * CD * reference_area;
		dvec3 drag_force = -drag_magn * normalize(body.velocityAt(body.position));
		body.applyForce(drag_force, drag_center);
	}
}


void Airplane::update(double dt, Engine& engine)
{
	body.update(dt);

	Window& w = engine.getWindow();
	double flaps = 7;
	if (w.keyDown(GLFW_KEY_S))
	{
		wings[2].Cl0 = -1 - flaps;
		wings[3].Cl0 = -1 - flaps;
	}
	else if (w.keyDown(GLFW_KEY_W))
	{
		wings[2].Cl0 = -1 + flaps;
		wings[3].Cl0 = -1 + flaps;
	}
	else
	{
		wings[2].Cl0 = -1;
		wings[3].Cl0 = -1;
	}

	double change = 5;
	if (w.keyDown(GLFW_KEY_A))
	{
		wings[0].Cl0 = 1 - change;
		wings[1].Cl0 = 1 + change;
	}
	else if (w.keyDown(GLFW_KEY_D))
	{
		wings[0].Cl0 = 1 + change;
		wings[1].Cl0 = 1 - change;
	}
	else
	{
		wings[0].Cl0 = 1;
		wings[1].Cl0 = 1;
	}

	

	for (auto& wing : wings)
	{
		calcLift(wing);
	}
	calcDrag();

	dvec3 forward = body.getTransform()*dvec4(1, 0, 0, 0);
	//body.applyForce(100000.0*forward, body.position);

	body.applyForce(dvec3(0, -9.82*body.mass, 0), body.position);

	for (auto&& f : body.applied_forces)
	{
		engine.getVectors().addVector(f.first, f.second, vec3(1, 0, 1));
	}

	

	std::string pos_text = "Position: (";
	pos_text += std::to_string((int)body.position.y);
	pos_text += ", " + std::to_string((int)body.position.y);
	pos_text += ", " + std::to_string((int)body.position.z) + ")";
	engine.getTexts().addText(0,0, pos_text);
	
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
