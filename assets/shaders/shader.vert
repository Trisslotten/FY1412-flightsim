#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 tex;
out vec3 normal;
out vec3 pos;

void main()
{
    tex = tex_coords;
	normal = transpose(inverse(mat3(model)))*in_normal;
	pos =  vec3(model * vec4(position, 1.0f));
    gl_Position = projection * view * model * vec4(position, 1.0f);
}	