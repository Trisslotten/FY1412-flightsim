#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "renderer.hpp"

class Texts : public Drawable
{
	std::vector<std::pair<glm::dvec2, std::string>> to_draw2d;
	std::vector<std::pair<glm::dvec3, std::string>> to_draw3d;

public:

	void addText(double x, double y, const std::string& text);
	void addText(glm::dvec3, const std::string& text);

	void clear();

	virtual void draw(Renderer& renderer);
};