#version 420 core
layout(location = 0) in vec2 pos;
uniform vec2 sizefactor;
uniform vec2 texfactor;
uniform mat4 mat;
out vec2 coord;

void main (void)
{
	coord = vec2 (pos.x * texfactor.x, (1 - pos.y) * texfactor.y);
	gl_Position = mat * vec4 (pos.x * sizefactor.x,
		      	    	  pos.y * sizefactor.y,
				  0.0, 1.0);
}
