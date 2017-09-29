#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "renderer.hpp"

class Texts : public Drawable
{
	std::vector<std::pair<glm::vec2, std::string>> to_draw2d;
	std::vector<std::pair<glm::vec3, std::string>> to_draw3d;

public:

	void addText(float x, float y, const std::string& text);
	void addText(glm::vec3, const std::string& text);

	void clear();

	virtual void draw(Renderer& renderer);
};