#version 410 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 pos[];
in vec3 normal[];
in vec2 tex[];

out vec2 frag_tex;
out vec3 frag_normal;
out vec3 frag_pos;

void main() {
	vec3 v1 = pos[1] - pos[0];
	vec3 v2 = pos[2] - pos[0];

	vec3 calc_normal = normalize(cross(v1, v2));

	int i;
	for (i = 0; i < gl_in.length(); i++)
	{
		gl_Position = gl_in[i].gl_Position;
		frag_tex = tex[i];

		frag_normal = normal[i];
		//frag_normal = calc_normal;

		frag_pos = pos[i];
		EmitVertex();
	}
	EndPrimitive();
	
}