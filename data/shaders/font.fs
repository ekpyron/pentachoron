#version 420 core
layout(location = 0) out vec4 output;
layout(binding = 0) uniform sampler2D texture;
in vec2 coord;
uniform vec3 color;

void main (void)
{
	float val = texture2D (texture, coord).r;
	output = vec4 (val * color, 1.0);
}
