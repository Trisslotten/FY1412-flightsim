#version 410 core

layout(location = 0) out vec3 out_color;

uniform samplerCube tex_cubemap;

in vec4 proj_pos;
in vec3 tex_coords;

void main () {
	vec2 pos = (proj_pos.xy/proj_pos.w)*0.5+0.5;
	out_color = texture(tex_cubemap, tex_coords).rgb;
}

