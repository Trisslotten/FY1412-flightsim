#include "body.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include <iostream>

glm::dmat4 Body::getTransform()
{
	return  glm::translate(glm::dmat4(), position)*glm::mat4_cast(orientation);
}

glm::dvec3 Body::velocityAt(glm::dvec3 world_pos)
{
	return momentum*inverse_mass + glm::cross(inverse_inertia*angular_momentum, world_pos - position);
}

void Body::applyForce(glm::dvec3 _force, glm::dvec3 world_pos, bool draw)
{
	torques -= glm::cross(_force, world_pos - position);
	forces += _force;
	if(draw)
		applied_forces.emplace_back(world_pos, _force);
}

void Body::applyImpuls(glm::dvec3 _impuls, glm::dvec3 world_pos)
{
	angular_momentum -= glm::cross(_impuls, world_pos - position);
	momentum += _impuls;
}

void Body::update(double dt)
{

	applied_forces.clear();

	momentum += forces*dt;
	angular_momentum += torques*dt;

	external_forces = forces;
	forces = glm::vec3();
	torques = glm::vec3();

	// från gaffer on games: https://gafferongames.com/post/physics_in_3d/
	glm::dvec3 velocity = inverse_mass*momentum;
	glm::dvec3 angular_velocity = inverse_inertia*angular_momentum;
	glm::dquat angular_velocity_q{
		0,
		angular_velocity.x,
		angular_velocity.y,
		angular_velocity.z,
	};
	glm::dquat spin = 0.5 * angular_velocity_q * orientation;


	position += velocity*dt;
	orientation += spin*dt;
	orientation = glm::normalize(orientation);
}
