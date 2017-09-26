#include "airplane.hpp"
#include "renderer.hpp"
#include <glm/gtx/transform.hpp>
#include <iostream>
#include "engine.hpp"

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

	shader.create("assets/inertia.vert",
				  "assets/inertia.frag");

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

				Ixz -= wp.x*wp.y;
				Ixz -= op.x*op.y;

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

	wing->load("assets/wing.obj");
	fuselage->load("assets/cylinder.obj");
	

	wing->uploadToGPU();
	fuselage->uploadToGPU();

	glm::mat4 fuselage_t = glm::scale(glm::vec3(8,1,1));

	glm::mat4 l_wing_t = glm::translate(glm::vec3(0,0,-4))*glm::scale(glm::vec3(1.3, 1, 4));
	glm::mat4 r_wing_t = glm::translate(glm::vec3(0,0,4))*glm::scale(glm::vec3(1.3, 1, 4));

	glm::mat4 l_hori_t = glm::translate(glm::vec3(-7, 0, -2))*glm::scale(glm::vec3(1, 1, 2));
	glm::mat4 r_hori_t = glm::translate(glm::vec3(-7, 0, 2))*glm::scale(glm::vec3(1, 1, 2));
	glm::mat4 vert_t = glm::translate(glm::vec3(-7, 2, 0))*glm::rotate(glm::half_pi<float>(), glm::vec3(1,0,0))*glm::scale(glm::vec3(1, 1, 2));

	fuselage_parts.emplace_back(fuselage, fuselage_t);
	wings.emplace_back(wing, l_wing_t);
	wings.emplace_back(wing, r_wing_t);
	wings.emplace_back(wing, l_hori_t);
	wings.emplace_back(wing, r_hori_t);
	wings.emplace_back(wing, vert_t);

	body.setMass(2000);

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
	
	body.applyImpuls(glm::vec3(150000, 0, 0), body.position + glm::vec3(0, 0, 0));
	body.applyImpuls(glm::vec3(0, 150000, 0), body.position + glm::vec3(0, 0, 0.2));
}

void Airplane::update(float dt, Engine& engine)
{

	//calcArea();

	body.applyForce(glm::vec3(0, -9.82*body.mass, 0), body.position);
	body.update(dt);


	engine.vectors.addVector(body.position, body.forces, glm::vec3(0, 0, 1));
	engine.vectors.addVector(body.position, body.velocityAt(body.position), glm::vec3(1, 0, 0));
}


void Airplane::draw(Renderer & renderer)
{
	for (auto&& part : fuselage_parts)
		renderer.draw(*part.model, body.getTransform()*part.transform);
	for (auto&& wing : wings)
		renderer.draw(*wing.model, body.getTransform()*wing.transform);
}
