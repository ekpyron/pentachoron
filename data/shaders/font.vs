/*  
 * This file is part of DRE.
 *
 * DRE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * DRE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DRE.  If not, see <http://www.gnu.org/licenses/>.
 */
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
