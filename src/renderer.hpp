#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "window.hpp"
#include "model.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "font.hpp"

void recursiveDraw(Model::Node* node, glm::mat4 transform, ShaderProgram & shader);

class Renderer;

class Drawable
{
public:
	virtual void draw(Renderer& renderer) = 0;
};


class Renderer
{
	
	std::shared_ptr<Camera> current_camera;
	Window* window;

	Font font;

	Model skybox;
	Texture skybox_cubemap;
	ShaderProgram skybox_shader;

	ShaderProgram shader;

	glm::mat4 projection;
	glm::mat4 view;


	friend class Texts;
	// for texts to call
	void drawText(float x, float y, const std::string& text);
	void drawText(glm::vec3 world_pos, const std::string& text);


	void createSkybox();
public:

	void setCamera(std::shared_ptr<Camera> camera)
	{
		current_camera = camera;
	}

	glm::vec2 getWindowSize()
	{
		return window->getWindowSize();
	}

	void init(Window* window);

	void setColor(glm::vec4 color);

	void initDrawing(Window& window);

	void draw(Drawable& drawable);

	void draw(Model& model, glm::mat4 world = glm::mat4());


	void drawSkybox();
};