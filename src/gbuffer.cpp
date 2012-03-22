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
#include "gbuffer.h"
#include "renderer.h"

GBuffer::GBuffer (Renderer *parent)
	: renderer (parent)
{
}

GBuffer::~GBuffer (void)
{
}

bool GBuffer::Init (void)
{
	{
		gl::Shader vshader (GL_VERTEX_SHADER),
			 fshader (GL_FRAGMENT_SHADER);
		std::string src;
		if (!ReadFile (MakePath ("shaders", "gbuffer", "vshader.txt"), src))
			 return false;
		vshader.Source (src);
		if (!vshader.Compile ())
		{
			(*logstream) << "Cannot compile "
									 << MakePath ("shaders", "gbuffer", "vshader.txt")
									 << ":" << std::endl << vshader.GetInfoLog () << std::endl;
			return false;
		}

		if (!ReadFile (MakePath ("shaders", "gbuffer", "fshader.txt"), src))
			 return false;
		fshader.Source (src);
		if (!fshader.Compile ())
		{
			(*logstream) << "Cannot compile "
									 << MakePath ("shaders", "gbuffer", "fshader.txt")
									 << ":" << std::endl << vshader.GetInfoLog () << std::endl;
			return false;
		}

		program.Attach (vshader);
		program.Attach (fshader);
		if (!program.Link ())
		{
			(*logstream) << "Cannot link the gbuffer shader:" << std::endl
									 << program.GetInfoLog () << std::endl;
			return false;
		}
		
	}

	width = config["gbuffer"]["width"].as<GLuint> ();
	height = config["gbuffer"]["height"].as<GLuint> ();

	gl::Buffer::Unbind (GL_PIXEL_UNPACK_BUFFER);

	colorbuffer.Image2D (GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
											 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	normalbuffer.Image2D (GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
												0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	specularbuffer.Image2D (GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
													0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	depthbuffer.Image2D (GL_TEXTURE_2D, 0, GL_R32F, width, height,
											 0, GL_RED, GL_FLOAT, NULL);
	depthbuffer_internal.Storage (GL_DEPTH_COMPONENT24,
																width, height);

	framebuffer.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
												 colorbuffer, 0);
	framebuffer.Texture2D (GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
												 normalbuffer, 0);
	framebuffer.Texture2D (GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
												 specularbuffer, 0);
	framebuffer.Texture2D (GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D,
												 depthbuffer, 0);
	framebuffer.Renderbuffer (GL_DEPTH_ATTACHMENT, depthbuffer_internal);

	framebuffer.DrawBuffers ({ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
				 GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 });

	return true;
}

void GBuffer::Render (const Geometry &geometry)
{
	program["projmat"] = renderer->camera.GetProjMatrix ();

	framebuffer.Bind (GL_FRAMEBUFFER);
	gl::Viewport (0, 0, width, height);

	gl::ClearBufferfv (GL_COLOR, 0, (float[]) {0.0f, 0.0f, 0.0f, 0.0f} );
	gl::ClearBufferfv (GL_COLOR, 1, (float[]) {0.0f, 0.0f, 0.0f, 0.0f} );
	gl::ClearBufferfv (GL_COLOR, 2, (float[]) {0.0f, 0.0f, 0.0f, 0.0f} );
	gl::ClearBufferfv (GL_COLOR, 3, (float[]) {0.0f, 0.0f, 0.0f, 0.0f} );
	gl::ClearBufferfv (GL_DEPTH, 0, (float[]) {1.0f});

	gl::Enable (GL_DEPTH_TEST);

	GL_CHECK_ERROR;

	program.Use ();

	geometry.Render (program, renderer->camera.GetViewMatrix ());

	gl::Program::UseNone ();
	
	gl::Framebuffer::Unbind (GL_FRAMEBUFFER);

	GL_CHECK_ERROR;
}
