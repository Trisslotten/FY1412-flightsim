#include "clouds.hpp"

#include <glm/gtx/transform.hpp>

void Clouds::init()
{
	cloud.load("cloud.obj");
	cloud.uploadToGPU();

	positions.resize(width*width);
	noise.SetNoiseType(FastNoise::NoiseType::Cubic);
	noise.SetFractalOctaves(0);
}

void Clouds::update(glm::vec3 pos, Terrain& terrain)
{
	float x_start = spacing*(((int)pos.x / (int)spacing) - width/2);
	float y_start = spacing*(((int)pos.z / (int)spacing) - width/2);
	
	for (int yi = 0; yi < width; yi++)
	{
		for (int xi = 0; xi < width; xi++)
		{
			float x = spacing*xi + x_start;
			float y = spacing*yi + y_start;

			glm::vec3 cpos;
			cpos.x = x + spacing * noise.GetNoise(x, y);
			cpos.z = y + spacing * noise.GetNoise(x + 2000, y + 2000);

			cpos.y = 2000 + terrain.heightAt(glm::vec3(x, 0, y));

			positions[xi + yi*width] = cpos;
		}
	}
	

}

void Clouds::draw(Renderer & renderer)
{
	renderer.setColor(glm::vec4(1, 1, 1, 0.7));
	for (auto&& pos : positions)
	{
		renderer.draw(cloud, glm::translate(pos) * glm::scale(glm::vec3(120)));
	}
}
