#include "model.hpp"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void Model::processAiTreeRecursive(aiNode * ainode, Node * node)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			
			node->transform[i][j] = ainode->mTransformation[j][i];
			//std::cout << node->transform[i][j] << " | ";
			
		}
		//std::cout << "\n";
	}
	//std::cout << "\n\n";
	for (int i = 0; i < ainode->mNumMeshes; i++)
	{
		unsigned int index = ainode->mMeshes[i];
		node->meshes.push_back(&meshes[index]);
	}
	node->children.resize(ainode->mNumChildren);
	for (int i = 0; i < ainode->mNumChildren; i++)
	{
		node->children[i] = new Node();
		processAiTreeRecursive(ainode->mChildren[i], node->children[i]);
	}
}

void Model::processAiScene(const aiScene* scene)
{
	if (!scene->HasMeshes())
	{
		std::cout << "ERROR: Model has no meshes, check flags\n";
		return;
	}
	if (!scene->HasMaterials())
	{
		std::cout << "ERROR: Model has no materials\n";
		return;
	}
	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		// calls constructor (Mesh(...)) with aiMesh as parameter
		meshes.emplace_back(scene->mMeshes[i]);
	}

	for (int i = 0; i < scene->mNumMaterials; i++)
	{

	}


	processAiTreeRecursive(scene->mRootNode, &root);
}

void Model::deleteNodesRecursive(Node * node)
{
	if (node->children.size() > 0)
	{
		for (int i = 0; i < node->children.size(); i++)
		{
			deleteNodesRecursive(node->children[i]);
			delete node->children[i];
		}
	}
}

Model::Model()
{
	root.transform = glm::mat4();
}

Model::~Model()
{
	deleteNodesRecursive(&root);
}

void Model::load(const std::string & file)
{
	Assimp::Importer importer;
	
	const aiScene* scene = importer.ReadFile(file, aiProcess_GenNormals |
											 aiProcess_Triangulate |
											 aiProcess_JoinIdenticalVertices |
											 aiProcess_SortByPType | 
											 aiProcess_GenUVCoords);
	if (!scene)
	{
		// error
		std::cout << "ERROR: " << importer.GetErrorString() << "\n";
		return;
	}
	processAiScene(scene);
}


Model::Node * Model::getRootNode()
{
	return &root;
}

void Model::addMesh(const Mesh & mesh)
{
	meshes.push_back(mesh);
	root.meshes.push_back(&meshes[meshes.size() - 1]);
}

void Model::addMesh(const std::vector<Vertex>& _vertices, const std::vector<GLuint>& _indices)
{
	meshes.emplace_back(_vertices, _indices);
	root.meshes.push_back(&meshes[meshes.size() - 1]);
}

void Model::uploadToGPU()
{
	if (!on_gpu)
	{
		for (int i = 0; i < meshes.size(); i++)
		{
			meshes[i].uploadToGPU();
		}
		on_gpu = true;
	}
}

void Model::removeFromGPU()
{
	if (on_gpu)
	{
		for (int i = 0; i < meshes.size(); i++)
		{
			meshes[i].removeFromGPU();
		}
		on_gpu = false;
	}
}
