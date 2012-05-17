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
	: renderer (parent)
{
}

Glow::~Glow (void)
{
}

bool Glow::Init (gl::Texture &glowmap, GLuint mipmap_level)
{
	width = renderer->gbuffer.GetWidth () >> (mipmap_level + 1);
	height = renderer->gbuffer.GetHeight () >> (mipmap_level + 1);
	map.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F, width, height,
							 0, GL_RGBA, GL_FLOAT, NULL);

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
}
