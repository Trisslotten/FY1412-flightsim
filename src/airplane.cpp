#include "airplane.hpp"
#include "renderer.hpp"
#include <glm/gtx/transform.hpp>
#include <iostream>
#include "engine.hpp"


// assume origo is in line
glm::vec3 projectOnLine(glm::vec3 to_project, glm::vec3 v)
{
	v = normalize(v);
	return dot(to_project, v)*v;
}
// assume origo is in plane 
glm::vec3 projectToPlane(glm::vec3 to_project, glm::vec3 v1, glm::vec3 v2)
{
	glm::vec3 normal = glm::normalize(cross(v1, v2));
	glm::vec3 on_normal = projectOnLine(to_project, normal);
	return to_project - on_normal;
}


void Airplane::genInertiaTensor()
{
	// side length in pixels to draw
	const int resolution = 700;
	// length of side of cube that surrounds the model in world space
	const float length = 20;
	float pixel_size = length / resolution;
	glViewport(0, 0, resolution, resolution);

	glm::mat4 projection = glm::ortho(-length /2, length /2, -length /2, length /2, 1.f, length +1);
	glm::mat4 view = glm::lookAt(glm::vec3(0, 0, -length /2-1), glm::vec3(0,0,0), glm::vec3(0, 1, 0));

	shader.create("inertia.vert",
				  "inertia.frag");

	shader.use();
	shader.uniform("projection", projection);
	shader.uniform("view", view);
	
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	for (auto&& part : fuselage_parts)
		recursiveDraw(part.model->getRootNode(), part.transform, shader);
	for (auto&& wing : wings)
		recursiveDraw(wing.model->getRootNode(), wing.transform, shader);
	
	GLfloat* depths = new GLfloat[resolution*resolution];
	glReadPixels(0, 0, resolution, resolution, GL_DEPTH_COMPONENT, GL_FLOAT, depths);

	float Ixx = 0.f;
	float Iyy = 0.f;
	float Izz = 0.f;
	float Ixy = 0.f;
	float Ixz = 0.f;
	float Iyz = 0.f;

	// assumes all cubes have the same weight
	int num_cubes = 0;

	for (int yi = 0; yi < resolution; yi++)
	{
		for (int xi = 0; xi < resolution; xi++)
		{
			float depth = depths[xi + yi * resolution];
			if (depth == 1.f)
				continue;


			float x = xi;
			float y = yi;

			glm::vec3 pos;
			pos.x = length*(resolution - x)/ resolution - length/2;
			pos.y = length*y / resolution - length / 2;
			pos.z = depth*length - length / 2;
			
			glm::vec3 wp = pos;

			// loop through all "cubes" behind till z = 0;
			wp.z += 0.5f*pixel_size;
			while(wp.z<0)
			{
				num_cubes += 2;
				
				// assume symmetry
				glm::vec3 op(wp.x, wp.y, -wp.z);

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
	glm::mat3 tensor(Ixx, Ixy, Ixz,
					 Ixy, Iyy, Iyz,
					 Ixz, Iyz, Izz);
	tensor /= num_cubes;
	body.setInertia(body.mass*tensor);

	delete[] depths;
 }

void Airplane::calcArea()
{
	// side length in pixels to draw
	int resolution = area_resolution;
	// length of side of cube that surrounds the model in world space
	float length = 20;
	float pixel_size = length / resolution;
	glViewport(0, 0, resolution, resolution);

	glm::mat4 projection = glm::ortho(-length / 2, length / 2, -length / 2, length / 2, 1.f, length + 1);


	glm::vec3 vel = body.momentum;
	if (glm::length(vel) < 0.00001f)
		return;
	
	glm::mat4 body_transform = body.getTransform();

	glm::vec3 eye = 0.5f*length*normalize(vel);
	glm::vec3 up = glm::vec3(0, 1, 0);
	if (glm::length(cross(eye, up)) <= 0.00001f)
		up = glm::vec3(1, 0, 0);

	glm::mat4 view = glm::lookAt(eye + body.position, body.position, up);

	shader.use();
	shader.uniform("projection", projection);
	shader.uniform("view", view);

	glClear(GL_DEPTH_BUFFER_BIT);
	for (auto&& part : fuselage_parts)
		recursiveDraw(part.model->getRootNode(), body_transform * part.transform, shader);
	for (auto&& wing : wings)
		recursiveDraw(wing.model->getRootNode(), body_transform * wing.transform, shader);

	glReadPixels(0, 0, resolution, resolution, GL_DEPTH_COMPONENT, GL_FLOAT, depth_map);

	unsigned int num_pixels = 0;
	for (int i = 0; i < resolution*resolution; i++)
	{
		//std::cout << depth_map[i] << "\n";
		if (depth_map[i] < 1.f)
			num_pixels++;
	}
	float area = pixel_size*pixel_size*num_pixels;
	//std::cout << area << "\n";	
}

Airplane::~Airplane()
{
	delete[] depth_map;
}

void Airplane::init()
{
	depth_map = new GLfloat[area_resolution*area_resolution];

	std::shared_ptr<Model> wing = std::make_shared<Model>();
	std::shared_ptr<Model> fuselage = std::make_shared<Model>();

	wing->load("wing.obj");
	fuselage->load("cylinder.obj");
	

	wing->uploadToGPU();
	fuselage->uploadToGPU();

	glm::mat4 fuselage_t = glm::scale(glm::vec3(8,1,1));

	glm::mat4 l_wing_t = glm::translate(glm::vec3(-1,1,-4))*glm::scale(glm::vec3(1.3, 1, 4));
	glm::mat4 r_wing_t = glm::translate(glm::vec3(-1,1,4))*glm::scale(glm::vec3(1.3, 1, 4));

	glm::mat4 l_hori_t = glm::translate(glm::vec3(-7, 1, -2))*glm::scale(glm::vec3(1, 1, 2));
	glm::mat4 r_hori_t = glm::translate(glm::vec3(-7, 1, 2))*glm::scale(glm::vec3(1, 1, 2));
	glm::mat4 vert_t = glm::translate(glm::vec3(-7, 2, 0))*glm::rotate(glm::half_pi<float>(), glm::vec3(1,0,0))*glm::scale(glm::vec3(1, 1, 2));

	fuselage_parts.emplace_back(fuselage, fuselage_t);
	wings.emplace_back(wing, l_wing_t, 6);
	wings.emplace_back(wing, r_wing_t, 6);
	wings.emplace_back(wing, l_hori_t, -4);
	wings.emplace_back(wing, r_hori_t, -4);
	wings.emplace_back(wing, vert_t);

	body.setMass(3000);

	genInertiaTensor();

	std::cout << "Inertia:\n";
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			std::cout << body.inertia[i][j] << " | ";
		}
		std::cout << "\n\n";
	}
	body.position = glm::vec3(-400, 400, 0);
	body.applyImpuls(glm::vec3(100000, 10000, 0), body.position);
	//body.applyImpuls(glm::vec3(0, 1, 0), body.position + glm::vec3(0,0,5000));
	//body.applyImpuls(glm::vec3(0, -1, 0), body.position + glm::vec3(0,0,-5000));
}

void Airplane::update(float dt, Engine& engine)
{

	float density = 1.23;
	
	for (auto& wing : wings)
	{
		glm::mat4 world_trans = body.getTransform()*wing.transform;

		glm::vec3 diag1 = wing.transform*glm::vec4(0.5, 0, 0.5, 0);
		glm::vec3 diag2 = wing.transform*glm::vec4(-0.5, 0, 0.5, 0);
		float area = 2*length(cross(diag1, diag2));

		// wing local space base vectors in world
		glm::vec3 ex = normalize(world_trans*glm::vec4(1, 0, 0, 0));
		glm::vec3 ey = normalize(world_trans*glm::vec4(0, 1, 0, 0));
		glm::vec3 ez = normalize(world_trans*glm::vec4(0, 0, 1, 0));

		glm::vec3 wing_pos = glm::vec3(world_trans*glm::vec4(0, 0, 0, 1));
		glm::vec3 vel = body.velocityAt(wing_pos);

		glm::vec3 alpha_vel = projectToPlane(vel, ex, ey);
		glm::vec3 beta_vel = projectToPlane(vel, ez, ex);
		
		float angle_of_attack = 0;
		if (length(alpha_vel) > 0.00000000001f)
		{
			float cosa = glm::dot(normalize(alpha_vel), ex);
			float radians = std::acos(cosa);
			angle_of_attack = glm::degrees(radians);
			if (dot(alpha_vel, ey) > 0)
				angle_of_attack *= -1;
		}
			

		std::cout << "Alpha: " << angle_of_attack << "\n";

		float v = length(vel);
		float Cl = glm::two_pi<float>()*angle_of_attack + wing.Cl0;


		float lift = 0;
		if (angle_of_attack > -20 && angle_of_attack < 20)
			lift = 0.5f*density*v*v*area*Cl;

		std::cout << "Lift: " << lift << "\n\n";
		

		body.applyForce(lift*ey, wing_pos);

		engine.getVectors().addVector(wing_pos, normalize(alpha_vel), glm::vec3(1, 0, 0));

		engine.getVectors().addVector(wing_pos, ex, glm::vec3(0, 1, 1));
		engine.getVectors().addVector(wing_pos, ey, glm::vec3(0, 1, 1));
		engine.getVectors().addVector(wing_pos, ez, glm::vec3(0, 1, 1));
	}


	for (auto&& f : body.applied_forces)
	{
		engine.getVectors().addVector(f.first, f.second, glm::vec3(1, 0, 1));
	}

	//calcArea();
	body.applyForce(glm::vec3(0, -9.82*body.mass, 0), body.position);
	body.update(dt);



	

	glm::vec3 v = body.velocityAt(body.position);
	glm::vec3 pos = body.position + log(length(v))*normalize(v);
	engine.getTexts().addText(pos, std::to_string((int)length(v)) + " m/s");

	glm::vec3 w = body.inverse_inertia*body.angular_momentum;
	pos = body.position + normalize(w); log(length(w))*normalize(w);
	engine.getTexts().addText(pos, std::to_string(60.f*length(w)/glm::two_pi<float>()) + " rpm");

	engine.getVectors().addVector(body.position, w, glm::vec3(0, 1, 0));
	engine.getVectors().addVector(body.position, body.velocityAt(body.position), glm::vec3(1, 0, 0));

	
}


void Airplane::draw(Renderer & renderer)
{
	for (auto&& part : fuselage_parts)
		renderer.draw(*part.model, body.getTransform()*part.transform);
	for (auto&& wing : wings)
		renderer.draw(*wing.model, body.getTransform()*wing.transform);
}
