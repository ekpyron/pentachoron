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
#include "opacitypass.h"
#include "renderer.h"

OpacityPass::OpacityPass (Renderer *parent)
	: renderer (parent)
{
}

OpacityPass::~OpacityPass (void)
{
}

bool OpacityPass::Init (void)
{
	gl::Shader vshader (GL_VERTEX_SHADER),
		 fshader (GL_FRAGMENT_SHADER);
	std::string src;
	
	if (!ReadFile (MakePath ("shaders", "opacity", "vshader.txt"), src))
		 return false;
	vshader.Source (src);
	if (!vshader.Compile ())
	{
		(*logstream) << "Cannot compile "
								 << MakePath ("shaders", "opacity", "vshader.txt")
								 << ":" << std::endl << vshader.GetInfoLog () << std::endl;
		return false;
	}
	
	if (!ReadFile (MakePath ("shaders", "opacity", "fshader.txt"), src))
		 return false;
	fshader.Source (src);
	if (!fshader.Compile ())
	{
		(*logstream) << "Cannot compile "
									 << MakePath ("shaders", "opacity", "fshader.txt")
								 << ":" << std::endl << vshader.GetInfoLog () << std::endl;
		return false;
	}
	
	program.Attach (vshader);
	program.Attach (fshader);
	if (!program.Link ())
	{
		(*logstream) << "Cannot link the opacity shader:" << std::endl
								 << program.GetInfoLog () << std::endl;
		return false;
	}


	gl::Buffer::Unbind (GL_PIXEL_UNPACK_BUFFER);
	texture.Image2D (GL_TEXTURE_2D, 0, GL_RGBA8, renderer->gbuffer.width,
									 renderer->gbuffer.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
									 NULL);

	framebuffer.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
												 texture, 0);
	framebuffer.Renderbuffer (GL_DEPTH_ATTACHMENT,
														renderer->gbuffer.depthbuffer);
	framebuffer.DrawBuffers ({ GL_COLOR_ATTACHMENT0 });
	
	return true;
}

void OpacityPass::Render (void)
{
	framebuffer.Bind (GL_FRAMEBUFFER);
	gl::Viewport (0, 0, renderer->gbuffer.width, renderer->gbuffer.height);
	gl::ClearBufferfv (GL_COLOR, 0, (float[]) { 0.0f, 0.0f, 0.0f, 0.0f });

	gl::Enable (GL_DEPTH_TEST);

	program.Use ();
	program["projmat"] = renderer->camera.GetProjMatrix ();
	renderer->geometry.RenderOpaque (program, renderer->camera.GetViewMatrix ());
	gl::Program::UseNone ();

	gl::Framebuffer::Unbind (GL_FRAMEBUFFER);
	GL_CHECK_ERROR;
}
