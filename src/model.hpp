#pragma once

#include "mesh.hpp"
#include "material.hpp"
#include <unordered_map>

#include <assimp/scene.h>
class Model
{
public:
	struct Node
	{
		// meshes taken from the vector<Mesh> in Model
		std::vector<Mesh*> meshes;

		std::vector<Node*> children;

		// relative to parent
		glm::mat4 transform;
	};

	friend class Renderer;

private:
	std::vector<Mesh> meshes;
	//std::vector<GLuint> meshes;
	Node root;

	bool on_gpu = false;

	void processAiTreeRecursive(aiNode* ainode, Node* node);
	void processAiScene(const aiScene*);

	void deleteNodesRecursive(Node* node);
public:

	Model();
	~Model();

	void load(const std::string& file);

	Node* getRootNode();

	void addMesh(const Mesh& mesh);
	void addMesh(const std::vector<Vertex>& _vertices, const std::vector<GLuint>& _indices);

	void uploadToGPU();
	void removeFromGPU();
};