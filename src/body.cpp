#include "body.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include <iostream>

glm::mat4 Body::getTransform()
{
	return  glm::translate(glm::mat4(), position)*glm::mat4_cast(orientation);
}

glm::vec3 Body::velocityAt(glm::vec3 world_pos)
{
	return momentum*inverse_mass + glm::cross(angular_momentum, world_pos - position)*inverse_inertia;
}

void Body::applyForce(glm::vec3 _force, glm::vec3 world_pos)
{
	torques -= glm::cross(_force, world_pos - position);
	forces += _force;
	applied_forces.emplace_back(world_pos, _force);
}

void Body::applyImpuls(glm::vec3 _impuls, glm::vec3 world_pos)
{
	angular_momentum -= glm::cross(_impuls, world_pos - position);
	momentum += _impuls;
}

void Body::update(float dt)
{

	applied_forces.clear();

	momentum += forces*dt;
	angular_momentum += torques*dt;

	external_forces = forces;
	forces = glm::vec3();
	torques = glm::vec3();

	// från gaffer on games: https://gafferongames.com/post/physics_in_3d/
	glm::vec3 velocity = inverse_mass*momentum;
	glm::vec3 angular_velocity = inverse_inertia*angular_momentum;
	glm::quat angular_velocity_q{
		0,
		angular_velocity.x,
		angular_velocity.y,
		angular_velocity.z,
	};
	glm::quat spin = 0.5f * angular_velocity_q * orientation;


	position += velocity*dt;
	orientation += spin*dt;
	orientation = glm::normalize(orientation);
}
