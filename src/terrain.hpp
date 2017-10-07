#pragma once

#include <queue>
#include <map>
#include <unordered_map>
#include <future>
#include "model.hpp"
#include "renderer.hpp"
#include "shader.hpp"

#define Key std::pair<int, int>

const int CHUNK_GRID_SIZE = 64;
const float CHUNK_SIZE = 1024.f * 4;

struct Chunk
{
	struct Parameters
	{
		glm::vec2 pos;
		unsigned lod_level;
	};

	glm::ivec2 pos;
	unsigned lod_level = 1;
	Model surface;
	Chunk(glm::vec2 pos, unsigned lod_level);
};

class Terrain : public Drawable
{
	std::map<Key, Chunk*> chunks;
	std::map<Key, bool> is_loading;

	std::vector<float> lod_distances;

	void addChunk(int x, int y, unsigned lod_level);
	void keepChunk(int x, int y, unsigned lod_level);
	void removeChunk(int x, int y);

	void checkChunk(int x, int y, glm::vec3 pos);
public:
	std::vector<std::future<Chunk*>> async_added;

	Terrain();

	void init();

	void update(glm::vec3 pos);

	virtual void draw(Renderer& renderer);

};
