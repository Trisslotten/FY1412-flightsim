#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coords;

uniform vec2 tex_offset;

uniform mat4 projection;
uniform mat4 model;

out vec2 tex;

void main()
{
    tex = tex_coords + tex_offset;
    gl_Position = projection * model * vec4(position, 1);
}	