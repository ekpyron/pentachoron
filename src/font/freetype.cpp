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
#include "font/freetype.h"
#include "font/font.h"

namespace gl {

Freetype::Freetype (void)
{
	if (FT_Init_FreeType (&library))
		 throw std::runtime_error ("FT_Init_FreeType failed");
}

Freetype::~Freetype (void)
{
	FT_Done_FreeType (library);
}

bool Freetype::Init (void)
{
	std::string src;
	if (!ReadFile (MakePath ("shaders", config["font"]["fshader"]
													 .as<std::string> ()), src))
		 return false;
	if (!fshader.Create (GL_FRAGMENT_SHADER, src))
	{
	}
	if (!ReadFile (MakePath ("shaders", config["font"]["vshader"]
													 .as<std::string> ()), src))
		 return false;
	if (!vshader.Create (GL_VERTEX_SHADER, src))
	{
		return false;
	}

	pipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT, fshader);
	pipeline.UseProgramStages (GL_VERTEX_SHADER_BIT, vshader);

	GLbyte vertices[] = { 0, 0, 1, 0, 0, 1, 1, 1 };
	buffer.Data (sizeof (vertices), vertices, GL_STATIC_DRAW);


	vertexarray.VertexAttribOffset (buffer, 0, 2, GL_BYTE, GL_FALSE, 0, 0);
	vertexarray.EnableVertexAttrib (0);

	sampler.Parameter (GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	sampler.Parameter (GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	sampler.Parameter (GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	sampler.Parameter (GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GL_CHECK_ERROR;

	return true;
}

void Freetype::Render (const glm::vec2 &texfactor,
											 const glm::vec2 &sizefactor,
											 const glm::mat4 &mat,
											 const glm::vec3 &color)
{
	fshader["color"] = color;
	vshader["sizefactor"] = sizefactor;
	vshader["texfactor"] = texfactor;
	vshader["mat"] = mat;

	gl::Disable (GL_DEPTH_TEST);
	gl::DepthMask (GL_FALSE);
	gl::Enable (GL_BLEND);
	gl::BlendFunc (GL_ONE, GL_ONE);
	gl::BlendEquation (GL_FUNC_ADD);
	if (glfwGetKey (GLFW_KEY_RALT))
		 gl::BlendEquation (GL_FUNC_REVERSE_SUBTRACT);

	vertexarray.Bind ();
	pipeline.Bind ();
	sampler.Bind (0);

	gl::DrawArrays (GL_TRIANGLE_STRIP, 0, 4);
	gl::DepthMask (GL_TRUE);
	gl::Disable (GL_BLEND);

	GL_CHECK_ERROR;
}

bool Freetype::Load (Font &font, const std::string &name)
{
	return font.Load (this, name);
}

} /* namespace gl */
