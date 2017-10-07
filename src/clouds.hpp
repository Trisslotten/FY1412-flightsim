#pragma once

#include <vector>
#include "model.hpp"
#include "renderer.hpp"
#include "FastNoise.h"
#include "terrain.hpp"

class Clouds : public Drawable
{
	Model cloud;

	int width = 30;
	std::vector<glm::vec3> positions;
	
	FastNoise noise;

	float spacing = 4000;

public:

	void init();


	void update(glm::vec3 pos, Terrain& terrain);


	virtual void draw(Renderer& renderer);
};