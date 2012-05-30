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
#include "glow.h"
#include "renderer.h"

Glow::Glow (Renderer *parent)
	: renderer (parent), limit (1.0f), exponent (1.0f)
{
}

Glow::~Glow (void)
{
}

bool Glow::Init (gl::Texture &screenmap, gl::Texture &glowmap,
								 GLuint mipmap_level)
{
	{
		gl::Shader obj (GL_FRAGMENT_SHADER);
		std::string source;
		if (!ReadFile (MakePath ("shaders", "blendglow.txt"), source))
			 return false;
		obj.Source (source);
		if (!obj.Compile ())
		{
			(*logstream) << "Could not compile "
									 << MakePath ("shaders", "blendglow.txt")
									 << ": " << std::endl << obj.GetInfoLog () << std::endl;
			 return false;
		}

		fprogram.Parameter (GL_PROGRAM_SEPARABLE, GL_TRUE);
		fprogram.Attach (obj);
		if (!fprogram.Link ())
		{
			(*logstream) << "Could not link the shader program "
									 << MakePath ("shaders", "blendglow.txt")
									 << ": " << std::endl << fprogram.GetInfoLog ()
									 << std::endl;
			 return false;
		}
	}
	pipeline.UseProgramStages (GL_VERTEX_SHADER_BIT,
														 renderer->windowgrid.vprogram);
	pipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT, fprogram);
	fprogram["viewport"] = glm::uvec2 (renderer->gbuffer.GetWidth (),
																		 renderer->gbuffer.GetHeight ());

	width = renderer->gbuffer.GetWidth () >> (mipmap_level + 1);
	height = renderer->gbuffer.GetHeight () >> (mipmap_level + 1);
	map.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F, width, height,
							 0, GL_RGBA, GL_FLOAT, NULL);

	sampler.Parameter (GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	sampler.Parameter (GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	sampler.Parameter (GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	sampler.Parameter (GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	framebuffer.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
												 screenmap, 0);
	framebuffer.DrawBuffers ({ GL_COLOR_ATTACHMENT0 });

#ifdef DEBUG
	renderer->memory += width * height * 4 * 2;
#endif

	source.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
										glowmap, mipmap_level);
	destination.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
												 map, 0);
	destination.DrawBuffers({ GL_COLOR_ATTACHMENT0 });

	mem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, map);

	SetSize (0);

	return true;
}

void Glow::SetSize (GLuint size)
{
	blur = renderer->filters.CreateBlur (mem, width, height, size);
}

GLuint Glow::GetSize (void)
{
	return blur.GetSize ();
}

GLfloat Glow::GetExponent (void)
{
	return exponent;
}

void Glow::SetExponent (GLfloat exp)
{
	exponent = exp;
}

GLfloat Glow::GetLimit (void)
{
	return limit;
}

void Glow::SetLimit (GLfloat l)
{
	limit = l;
}

const gl::Texture &Glow::GetMap (void)
{
	return map;
}

void Glow::Apply (void)
{
	if (GetSize () == 0)
		 return;

	source.Bind (GL_READ_FRAMEBUFFER);
	destination.Bind (GL_DRAW_FRAMEBUFFER);
	gl::ReadBuffer (GL_COLOR_ATTACHMENT0);

	gl::BlitFramebuffer (0, 0, width << 1, height << 1, 0, 0, width, height,
											 GL_COLOR_BUFFER_BIT, GL_LINEAR);
	
	gl::Framebuffer::Unbind (GL_DRAW_FRAMEBUFFER);
	gl::Framebuffer::Unbind (GL_READ_FRAMEBUFFER);

	blur.Apply ();

	framebuffer.Bind (GL_FRAMEBUFFER);
	gl::Viewport (0, 0, renderer->gbuffer.GetWidth (),
								renderer->gbuffer.GetHeight ());
	pipeline.Bind ();
	map.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
	sampler.Bind (0);
	gl::Enable (GL_BLEND);
	gl::BlendFunc (GL_ONE, GL_ONE);
	gl::BlendEquation (GL_FUNC_ADD);
	renderer->windowgrid.Render ();
	gl::Disable (GL_BLEND);

	gl::Framebuffer::Unbind (GL_FRAMEBUFFER);
	GL_CHECK_ERROR;
}
