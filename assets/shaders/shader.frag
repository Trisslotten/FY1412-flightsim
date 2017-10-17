#version 410 core

out vec4 frag_color;

in vec2 frag_tex;
in vec3 frag_normal;
in vec3 frag_pos;

const vec3 sun_dir = normalize(vec3(-4,-2, 0.3));
const vec3 sun_color = vec3(1,1,0.96);

uniform vec4 mat_color;

void main()
{
	//vec3 l = normalize(light_pos - frag_pos);
	vec3 n = normalize(frag_normal);

	float i = clamp(dot(-sun_dir,n), 0, 1);;

	vec3 diffuse = mat_color.rgb*sun_color*i;
	vec3 ambient = mat_color.rgb*0.2;

	vec3 color = ambient + diffuse;

	frag_color = vec4(color, mat_color.a);
}