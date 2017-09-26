#version 410 core

out vec4 frag_color;

in vec2 frag_tex;
in vec3 frag_normal;
in vec3 frag_pos;

const vec3 sun_dir = vec3(-4,-2, 0.3);
const vec3 sun_color = vec3(1,1,1);

uniform vec4 mat_color;

void main()
{
	//vec3 l = normalize(light_pos - frag_pos);
	vec3 n = normalize(frag_normal);

	float i = dot(-sun_dir,n);

	vec3 diffuse = clamp(mat_color.xyz*sun_color*i, 0, 1);
	vec3 ambient = mat_color.xyz*0.4;

	vec3 color = ambient + diffuse;

	frag_color = vec4(color, mat_color.a);
}