#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


struct Body
{
	// accumulated from applyForce()
	glm::vec3 forces, torques;
	glm::vec3 external_forces;

	// Primary
	glm::vec3 position;
	glm::vec3 momentum;

	glm::quat orientation;
	glm::vec3 angular_momentum;


	// Calculated. These are only valid after deriving them from primary values

	// Constants
	float mass;
	float inverse_mass;

	// moment of ... (rotational)
	glm::mat3 inertia;
	glm::mat3 inverse_inertia;

	void setMass(float _mass)
	{
		mass = _mass;
		inverse_mass = 1.f / mass;
	}
	void setInertia(glm::mat3 _inertia)
	{
		inertia = _inertia;
		inverse_inertia = glm::inverse(inertia);
	}

	glm::mat4 getTransform();

	glm::vec3 velocityAt(glm::vec3 world_pos);

	void applyForce(glm::vec3 force, glm::vec3 world_pos);
	void applyImpuls(glm::vec3 impuls, glm::vec3 world_pos);

	void update(float dt);

};