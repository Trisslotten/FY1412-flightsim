#include "renderer.hpp"

#include <GL/glew.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace
{
	GLfloat skybox_verts[] = {
		// Positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};
}

void recursiveDraw(Model::Node* node, glm::mat4 transform, ShaderProgram & shader)
{
	glm::mat4 current_transform = transform*node->transform;

	shader.uniform("model", current_transform);

	for (int i = 0; i < node->meshes.size(); i++)
	{
		node->meshes[i]->bind();
		glDrawElements(GL_TRIANGLES, node->meshes[i]->numIndices(), GL_UNSIGNED_INT, (void*)0);
		
	}
	for (int i = 0; i < node->children.size(); i++)
	{
		recursiveDraw(node->children[i], current_transform, shader);
		
	}
}

void Renderer::createSkybox()
{
	std::vector<Vertex>	vertices;
	vertices.reserve(108 / 3);
	std::vector<GLuint> indices;
	for (int i = 0; i < 108; i+=3)
	{
		Vertex v;
		v.position.x = skybox_verts[i];
		v.position.y = skybox_verts[i+1];
		v.position.z = skybox_verts[i+2];
		vertices.push_back(v);
		indices.push_back(i);
		indices.push_back(i+1);
		indices.push_back(i+2);
	}
	skybox.addMesh(vertices, indices);
	skybox.uploadToGPU();

	skybox_cubemap.load("assets/cubemap/", TEXTURE_CUBEMAP);
}

void Renderer::init(Window* _window)
{
	window = _window;
	shader.create("shader.vert", "shader.geom", "shader.frag");
	skybox_shader.create("skybox.vert", "skybox.frag");
	createSkybox();
	
	near = 1;
	far = 10000;

	glm::vec2 size = window->getWindowSize();
	glViewport(0, 0, size.x, size.y);


	font.load("assets/font.png");

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void Renderer::setColor(glm::vec4 color)
{
	shader.use();
	shader.uniform("mat_color", color);
}

void Renderer::setNearFarPlane(float near, float far)
{
	glm::vec2 size = getWindowSize();
	this->near = near;
	this->far = far;
	projection = glm::perspective<double>(fov, size.x / size.y, near, far);
	view = current_camera->getView();
	shader.use();
	shader.uniform("projection", projection);
	skybox_shader.use();
	skybox_shader.uniform("projection", projection);
}

void Renderer::initDrawing(Window& window)
{
	glm::vec2 size = getWindowSize();
	glViewport(0, 0, size.x, size.y);

	projection = glm::perspective<double>(fov, size.x / size.y, near, far);
	view = current_camera->getView();
	shader.use();
	shader.uniform("projection", projection);
	skybox_shader.use();
	skybox_shader.uniform("projection", projection);
}

void Renderer::draw(Drawable& drawable) 
{
	drawable.draw(*this);
}

void Renderer::draw(Model & model, glm::mat4 world)
{
	shader.use();
	shader.uniform("view", current_camera->getView());
	recursiveDraw(model.getRootNode(), world, shader);
}

void Renderer::drawText(float x, float y, const std::string & text)
{
	font.drawText(*this, x, y, text);
}

void Renderer::drawText(glm::vec3 world_pos, const std::string & text)
{
	glm::vec4 transformed = projection * view * glm::vec4(world_pos, 1);
	transformed /= transformed.w;
	transformed += 1;
	transformed *= 0.5f;
	glm::vec2 size = getWindowSize();
	transformed.x *= size.x;
	transformed.y *= size.y;
	drawText(round(transformed.x), round(transformed.y), text);
}

void Renderer::drawSkybox()
{
	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	skybox_cubemap.bind(0);
	skybox_shader.use();
	glm::mat4 view(glm::mat3(current_camera->getView()));
	skybox_shader.uniform("view", view);
	recursiveDraw(skybox.getRootNode(), glm::mat4(), skybox_shader);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
}