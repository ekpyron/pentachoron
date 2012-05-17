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
									 << ":" << std::endl << fshader.GetInfoLog () << std::endl;
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

		fshader = gl::Shader (GL_FRAGMENT_SHADER);
		if (!ReadFile (MakePath ("shaders", "gbuffer", "depthonly.txt"), src))
			 return false;
		fshader.Source (src);
		if (!fshader.Compile ())
		{
			(*logstream) << "Cannot compile "
									 << MakePath ("shaders", "gbuffer", "depthonly.txt")
									 << ":" << std::endl << fshader.GetInfoLog () << std::endl;
			return false;
		}

		depthonlyprog.Attach (vshader);
		depthonlyprog.Attach (fshader);
		if (!depthonlyprog.Link ())
		{
			(*logstream) << "Cannot link the gbuffer depth only shader:"
									 << std::endl << depthonlyprog.GetInfoLog ()
									 << std::endl;
			return false;
		}

		fshader = gl::Shader (GL_FRAGMENT_SHADER);
		if (!ReadFile (MakePath ("shaders", "gbuffer", "transparency.txt"), src))
			 return false;
		fshader.Source (src);
		if (!fshader.Compile ())
		{
			(*logstream) << "Cannot compile "
									 << MakePath ("shaders", "gbuffer", "transparency.txt")
									 << ":" << std::endl << fshader.GetInfoLog () << std::endl;
			return false;
		}

		transparencyprog.Attach (vshader);
		transparencyprog.Attach (fshader);
		if (!transparencyprog.Link ())
		{
			(*logstream) << "Cannot link the gbuffer transparency shader:"
									 << std::endl << transparencyprog.GetInfoLog ()
									 << std::endl;
			return false;
		}
	}

	width = config["gbuffer"]["width"].as<GLuint> ();
	height = config["gbuffer"]["height"].as<GLuint> ();
	width = (width + 15) & (~0xF);
	height = (height + 15) & (~0xF);

	gl::Buffer::Unbind (GL_PIXEL_UNPACK_BUFFER);

	depthsampler.Parameter (GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	depthsampler.Parameter (GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	depthsampler.Parameter (GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	depthsampler.Parameter (GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	depthbuffer.Storage (GL_DEPTH_COMPONENT32, width, height);

	colorbuffer.Image2D (GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
											 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	normalbuffer.Image2D (GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
												0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	specularbuffer.Image2D (GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
													0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	
	for (auto i = 0; i < 4; i++)
		 depthtexture[i].Image2D (GL_TEXTURE_2D, 0, GL_R32F, width, height,
															0, GL_RED, GL_FLOAT, NULL);

#ifdef DEBUG
	renderer->memory += width * height * (4 * 4 + 4 + 4 + 4 + 4);
#endif

	framebuffer[0].Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
												 colorbuffer, 0);
	framebuffer[0].Texture2D (GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
												 normalbuffer, 0);
	framebuffer[0].Texture2D (GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
												 specularbuffer, 0);
	framebuffer[0].Texture2D (GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D,
												 depthtexture[0], 0);
	framebuffer[0].Renderbuffer (GL_DEPTH_ATTACHMENT, depthbuffer);
		
	framebuffer[0].DrawBuffers ({ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
					 GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 });

	for (auto i = 1; i < 4; i++)
	{
		framebuffer[i].Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
															depthtexture[i], 0);
		framebuffer[i].Renderbuffer (GL_DEPTH_ATTACHMENT, depthbuffer);
		framebuffer[i].DrawBuffers ({ GL_COLOR_ATTACHMENT0 });
	}

	fragidx.Image2D (GL_TEXTURE_2D, 0, GL_R32I, width, height,
									 0, GL_RED_INTEGER, GL_INT, NULL);
	fraglist.Data (width * height * 4 * 4 * 4, NULL, GL_DYNAMIC_COPY);
	fraglisttex.Buffer (GL_R32UI, fraglist);

#ifdef DEBUG
	renderer->memory += width * height * (4 * 4 * 4 + 4);
#endif

	transparencyfb.Renderbuffer (GL_DEPTH_ATTACHMENT, depthbuffer);
	transparencyfb.DrawBuffers ({ });

	transparencyclearfb.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
																 fragidx, 0);
	transparencyclearfb.DrawBuffers ({ GL_COLOR_ATTACHMENT0 });

	depthmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, depthtexture[0]);
	colormem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, colorbuffer);
	normalmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, normalbuffer);
	specularmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, specularbuffer);

	fragidxmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, fragidx);
	fraglistmem = renderer->clctx.CreateFromGLBuffer
		 (CL_MEM_READ_ONLY, fraglist);

	const GLuint counters[64]
		 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	counter.Data (sizeof (GLuint) * 64, counters, GL_DYNAMIC_DRAW);

	return true;
}

GLuint GBuffer::GetWidth (void)
{
	return width;
}

GLuint GBuffer::GetHeight (void)
{
	return height;
}

void GBuffer::Render (Geometry &geometry)
{
	program["projmat"] = renderer->camera.GetProjMatrix ();
	program["viewport"] = glm::uvec2 (width, height);
	renderer->culling.SetProjMatrix (renderer->camera.GetProjMatrix ());
	program["farClipPlane"] = renderer->camera.GetFarClipPlane ();
	program["nearClipPlane"] = renderer->camera.GetNearClipPlane ();

	transparencyprog["projmat"] = renderer->camera.GetProjMatrix ();
	transparencyprog["viewport"] = glm::uvec2 (width, height);
	transparencyprog["farClipPlane"] = renderer->camera.GetFarClipPlane ();
	transparencyprog["nearClipPlane"] = renderer->camera.GetNearClipPlane ();

	gl::Enable (GL_DEPTH_TEST);
	gl::DepthMask (GL_TRUE);
	gl::DepthFunc (GL_LESS);

	program.Use ();

	framebuffer[0].Bind (GL_FRAMEBUFFER);
	gl::Viewport (0, 0, width, height);
		
	gl::ClearBufferfv (GL_COLOR, 0, (float[]) {0.0f, 0.0f, 0.0f, 1.0f} );
	gl::ClearBufferfv (GL_COLOR, 1, (float[]) {0.0f, 0.0f, 0.0f, 0.0f} );
	gl::ClearBufferfv (GL_COLOR, 2, (float[]) {0.0f, 0.0f, 0.0f, 0.0f} );
	gl::ClearBufferfv (GL_COLOR, 3, (float[]) {1.0f, 0.0f, 0.0f, 0.0f} );
	gl::ClearBufferfv (GL_DEPTH, 0, (float[]) {1.0f});

	geometry.Render (Geometry::Pass::GBuffer,
									 program, renderer->camera.GetViewMatrix ());

	transparencyprog.Use ();

	gl::DepthMask (GL_FALSE);

	transparencyclearfb.Bind (GL_FRAMEBUFFER);

	gl::ClearBufferuiv (GL_COLOR, 0, (GLuint[]) { (GLuint) -1, (GLuint) -1,
				 (GLuint) -1, (GLuint) -1 });
	
	transparencyfb.Bind (GL_FRAMEBUFFER);
	gl::Viewport (0, 0, width, height);

	fraglisttex.BindImage (0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
	fragidx.BindImage (1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);
	counter.BindBase (GL_ATOMIC_COUNTER_BUFFER, 0);
	geometry.Render (Geometry::Pass::GBufferTransparency,
									 transparencyprog, renderer->camera.GetViewMatrix ());

	GLuint *ptr = (GLuint*) counter.MapRange (0, sizeof (GLuint) * 64,
																						GL_MAP_WRITE_BIT
																						| GL_MAP_INVALIDATE_BUFFER_BIT
																						| GL_MAP_UNSYNCHRONIZED_BIT);
	for (auto i = 0; i < 64; i++)
	{
		 ptr[i] = 0;
	}
	counter.Unmap ();

	depthonlyprog.Use ();

	if (renderer->GetAntialiasing ())
	{
		for (auto i = 1; i < 4; i++)
		{
			framebuffer[i].Bind (GL_FRAMEBUFFER);
			gl::Viewport (0, 0, width, height);
			gl::ClearBufferfv (GL_COLOR, 0, (float[]) {1.0f, 0.0f, 0.0f, 0.0f} );
			gl::ClearBufferfv (GL_DEPTH, 0, (float[]) {1.0f});
			
			geometry.Render (Geometry::Pass::GBufferDepthOnly,
											 program, renderer->camera.GetViewMatrix ());
		}
	}

	gl::Framebuffer::Unbind (GL_FRAMEBUFFER);
	gl::Program::UseNone ();
	
	GL_CHECK_ERROR;
}
