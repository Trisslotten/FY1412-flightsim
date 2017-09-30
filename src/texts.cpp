#include "texts.hpp"

void Texts::addText(double x, double y, const std::string & text)
{
	to_draw2d.push_back(std::make_pair(glm::vec2(x, y), text));
}

void Texts::addText(glm::dvec3 pos, const std::string & text)
{
	to_draw3d.push_back(std::make_pair(pos, text));
}

void Texts::draw(Renderer & renderer)
{
	for (auto&& t : to_draw2d)
		renderer.drawText((float)t.first.x, (float)t.first.y, t.second);
	for (auto&& t : to_draw3d)
		renderer.drawText((glm::vec3)t.first, t.second);

}

void Texts::clear()
{
	to_draw2d.clear();
	to_draw3d.clear();
}


