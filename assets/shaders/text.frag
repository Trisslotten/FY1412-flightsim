#version 410 core

out vec4 frag_color;

uniform sampler2D font_texture;

in vec2 tex;

vec4 when_gt(vec4 x, vec4 y) {
  return max(sign(x - y), 0.0);
}
vec4 when_le(vec4 x, vec4 y) {
  return 1.0 - when_gt(x, y);
}

void main()
{
	frag_color = texture(font_texture, tex);
	frag_color += vec4(-1,-1,-1,0.5)*when_le(vec4(frag_color.a), vec4(0));
	//frag_color = vec4(0, tex, 1);
}