#version 410 core

layout (location = 0) in vec3 position;

uniform mat4 view;
uniform mat4 projection;

out vec4 proj_pos;
out vec3 tex_coords;

void main() 
{
	proj_pos = projection * view * vec4(position, 1.0);
	gl_Position = proj_pos.xyww;
	tex_coords = position;
}