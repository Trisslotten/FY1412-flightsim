#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace glm;

struct Body
{
	// accumulated from applyForce()
	dvec3 forces, torques;
	dvec3 external_forces;

	// Position and Force
	std::vector<std::pair<dvec3, dvec3>> applied_forces;

	// Primary
	dvec3 position;
	dvec3 momentum;

	dquat orientation;
	dvec3 angular_momentum;


	// Calculated. These are only valid after deriving them from primary values

	// Constants
	double mass;
	double inverse_mass;

	// moment of ... (rotational)
	dmat3 inertia;
	dmat3 inverse_inertia;

	void setMass(double _mass)
	{
		mass = _mass;
		inverse_mass = 1.f / mass;
	}
	void setInertia(dmat3 _inertia)
	{
		inertia = _inertia;
		inverse_inertia = inverse(inertia);
	}

	dmat4 getTransform();

	dvec3 velocityAt(dvec3 world_pos);

	void applyForce(dvec3 force, dvec3 world_pos);
	void applyImpuls(dvec3 impuls, dvec3 world_pos);

	void update(double dt);

};