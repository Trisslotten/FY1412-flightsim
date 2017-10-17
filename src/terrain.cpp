#include "terrain.hpp"

#include <chrono>
#include <thread>
#include "FastNoise.h"
#include "renderer.hpp"
#include "shader.hpp"
#include <iostream>
#include <future>
#include <glm/gtx/transform.hpp>


namespace
{

	float noiseFunc(glm::vec2 pos, FastNoise& noise)
	{
		float scale = 3000;
		return scale*noise.GetNoise(pos.x / scale, pos.y / scale) - 500;
	}
}


Terrain::Terrain()
{}

Terrain::~Terrain()
{
	for (auto&& c : chunks)
	{
		delete c.second;
	}
	for (auto&& c : async_added)
	{
		c.wait();
		delete c.get();
	}
}

float Terrain::heightAt(glm::vec3 pos)
{
	return noiseFunc(glm::vec2(pos.x, pos.z), noise);
}

void Terrain::init()
{
	/*
	for (int i = 0; i < log2(CHUNK_GRID_SIZE) - 2; i++)
	{
		lod_distances.push_back(CHUNK_SIZE*(0.09*i*i + 0.7*i + 1.2));
	}
	*/
	lod_distances.push_back(CHUNK_SIZE * 2);
	lod_distances.push_back(CHUNK_SIZE * 4);
	lod_distances.push_back(CHUNK_SIZE * 8);

	noise.SetSeed(1337);
	noise.SetFrequency(0.3);
	noise.SetFractalOctaves(15);
	noise.SetFractalType(FastNoise::FractalType::FBM);
	noise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);


	update(glm::vec3(0, 0, 0));
	while(async_added.size() > 0)
	{
		update(glm::vec3(0, 0, 0));
	}
}


glm::vec3 calcNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
{
	glm::vec3 v1 = p1 - p2;
	glm::vec3 v2 = p1 - p3;
	return -glm::cross(v1, v2);
}


Chunk::Chunk(glm::vec2 pos, unsigned lod_level, FastNoise& noise)
{

	float resolution = 1.f / std::powf(2.f, lod_level - 1);
	this->lod_level = lod_level;
	this->pos.x = pos.x;
	this->pos.y = pos.y;
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	int curr_grid_size = CHUNK_GRID_SIZE*resolution;

	vertices.reserve((curr_grid_size + 1)*(curr_grid_size + 1));
	indices.resize(curr_grid_size*curr_grid_size * 6);

	//std::cout << pos.x << " " << pos.y << "\n";
	for (int y = 0; y <= curr_grid_size; y++)
	{
		for (int x = 0; x <= curr_grid_size; x++)
		{
			float cell_length = CHUNK_SIZE / (curr_grid_size);
			glm::vec2 world_2d = glm::vec2(x, y)*cell_length + CHUNK_SIZE*pos;
			float height = noiseFunc(world_2d, noise);
			glm::vec3 world_pos(world_2d.x, height, world_2d.y);

			/*
			** for smooth normals
			cell_length = CHUNK_SIZE / CHUNK_GRID_SIZE;
			glm::vec3 normal;
			glm::vec3 p1(world_pos.x + cell_length, 0, world_pos.z);
			p1.y = noiseFunc(glm::vec2(p1.x, p1.z));

			glm::vec3 p2(world_pos.x, 0, world_pos.z + cell_length);
			p2.y = noiseFunc(glm::vec2(p2.x, p2.z));

			glm::vec3 p3(world_pos.x - cell_length, 0, world_pos.z);
			p3.y = noiseFunc(glm::vec2(p3.x, p3.z));

			glm::vec3 p4(world_pos.x, 0, world_pos.z - cell_length);
			p4.y = noiseFunc(glm::vec2(p4.x, p4.z));

			normal += calcNormal(world_pos, p1, p2);
			normal += calcNormal(world_pos, p2, p3);
			normal += calcNormal(world_pos, p3, p4);
			normal += calcNormal(world_pos, p4, p1);
			normal = glm::normalize(normal);
			
			vertices.emplace_back(world_pos, normal, glm::vec2());
			*/

			vertices.emplace_back(world_pos, glm::vec3(), glm::vec2());

			//std::cout << "  " << world_2d.x << " " << world_2d.y << "\n";
		}
	}

	int i = 0;
	for (int y = 0; y < curr_grid_size; y++)
	{
		for (int x = 0; x < curr_grid_size; x++)
		{
			indices[i] = x + y*(curr_grid_size + 1);
			i++;
			indices[i] = x + 1 + (y + 1)*(curr_grid_size + 1);
			i++;
			indices[i] = x + 1 + y*(curr_grid_size + 1);
			i++;

			indices[i] = x + y*(curr_grid_size + 1);
			i++;
			indices[i] = x + (y + 1)*(curr_grid_size + 1);
			i++;
			indices[i] = x + 1 + (y + 1)*(curr_grid_size + 1);
			i++;
		}
	}
	Mesh mesh{ vertices, indices };
	surface.addMesh(mesh);
}




Chunk* createChunk(glm::vec2 pos, unsigned lod_level, FastNoise& noise)
{
	return new Chunk(pos, lod_level, noise);
}

void Terrain::addChunk(int x, int y, unsigned lod_level)
{
	//std::cout << "Creating Chunk: " << x << " " << y << "\n";
	is_loading[std::make_pair(x, y)] = true;
	async_added.push_back(std::async(createChunk, glm::vec2(x, y), lod_level, noise));
}

void Terrain::keepChunk(int x, int y, unsigned lod_level)
{
	auto curr_pos = std::make_pair(x, y);
	if (chunks.count(curr_pos) == 0)
	{
		if (!is_loading[curr_pos])
			addChunk(x, y, lod_level);
	}
	else
	{
		if (chunks[curr_pos]->lod_level != lod_level)
		{
			if (!is_loading[curr_pos])
				addChunk(x, y, lod_level);
		}
	}
}

void Terrain::removeChunk(int x, int y)
{
	is_loading[std::make_pair(x, y)] = false;
	if (chunks.count(std::make_pair(x, y)) > 0)
	{
		//std::cout << "Removing Chunk: " << x << " " << y << "\n";
		Chunk* chunk = chunks[std::make_pair(x, y)];
		chunk->surface.removeFromGPU();
		delete chunk;
		chunks.erase(std::make_pair(x, y));
		is_loading.erase(std::make_pair(x, y));
	}
}

void Terrain::checkChunk(int x, int y, glm::vec3 pos)
{
	auto curr_pos = std::make_pair(x, y);
	float distance = glm::length(pos - CHUNK_SIZE*(glm::vec3(x, 0, y) + glm::vec3(0.5, 0, 0.5)));
	bool to_remove = true;
	for (int i = 0; i < lod_distances.size(); i++)
	{
		if (distance < lod_distances[i])
		{
			keepChunk(x, y, i + 1);
			to_remove = false;
			break;
		}
	}
	if (to_remove)
	{
		removeChunk(x, y);
	}
}

void Terrain::update(glm::vec3 pos)
{
	glm::vec3 chunk_pos = pos / CHUNK_SIZE;
	float view_distance = lod_distances[lod_distances.size() - 1];
	float chunk_distance = view_distance / CHUNK_SIZE;
	int xstart = chunk_pos.x - chunk_distance - 1;
	int xend = chunk_pos.x + chunk_distance + 1;
	int ystart = chunk_pos.z - chunk_distance - 1;
	int yend = chunk_pos.z + chunk_distance + 1;
	for (int y = ystart; y < yend; y++)
	{
		for (int x = xstart; x < xend; x++)
		{
			checkChunk(x, y, pos);
		}
	}

	for (int i = 0; i < async_added.size(); i++)
	{
		std::future_status status = async_added[i].wait_for(std::chrono::nanoseconds(1));

		if (status == std::future_status::ready)
		{
			Chunk* chunk = async_added[i].get();
			auto curr_pos = std::make_pair(chunk->pos.x, chunk->pos.y);
			removeChunk(curr_pos.first, curr_pos.second);

			chunks[curr_pos] = chunk;
			chunk->surface.uploadToGPU();

			async_added.erase(async_added.begin() + i);
			break;
		}
	}
}

void Terrain::draw(Renderer & renderer)
{
	renderer.setColor(glm::vec4(0.00392157, 0.556863, 0.054902, 1));
	for (auto & map_e : chunks)
	{
		Chunk* chunk = map_e.second;
		renderer.draw(chunk->surface, glm::mat4());
	}
}