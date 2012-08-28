/*  
 * This file is part of Pentachoron.
 *
 * Pentachoron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pentachoron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pentachoron.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "windowgrid.h"

WindowGrid::WindowGrid (void)
{
}

WindowGrid::~WindowGrid (void)
{
}

bool WindowGrid::Init (void)
{
	std::string src;
	std::string filename (MakePath ("shaders", "passthrough.vs"));
	if (!ReadFile (filename, src))
		 return false;
	if (!vprogram.Create (GL_VERTEX_SHADER, src))
	{
		(*logstream) << "Cannot create a shader program from "
								 << filename << std::endl << ":"
								 << vprogram.GetInfoLog () << std::endl;
		 return false;
	}

	char data[] = { -1, -1, 1, -1, -1, 1, 1, 1 };
	arraybuffer.Data (sizeof (data), data, GL_STATIC_DRAW);

	vao.VertexAttribOffset (arraybuffer, 0, 2, GL_BYTE, GL_FALSE, 0, 0);
	vao.EnableVertexAttrib (0);

	return true;
}

void WindowGrid::Render (void)
{
	vao.Bind ();
	gl::DrawArrays (GL_TRIANGLE_STRIP, 0, 4);
	GL_CHECK_ERROR;
}
