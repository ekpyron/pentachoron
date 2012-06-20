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
#include "shadowmap.h"
#include "renderer.h"

ShadowMap::ShadowMap (Renderer *parent)
	: renderer (parent)
{
}

ShadowMap::~ShadowMap (void)
{
}

bool ShadowMap::Init (void)
{
	{
		gl::Shader vshader (GL_VERTEX_SHADER),
			 fshader (GL_FRAGMENT_SHADER);
		std::string src;
		
		if (!ReadFile (MakePath ("shaders", "shadowmap", "vshader.txt"), src))
			 return false;
		vshader.Source (src);
		if (!vshader.Compile ())
		{
			(*logstream) << "Cannot compile "
									 << MakePath ("shaders", "shadowmap", "vshader.txt")
									 << ":" << std::endl << vshader.GetInfoLog () << std::endl;
			return false;
		}

		if (!ReadFile (MakePath ("shaders", "shadowmap", "fshader.txt"), src))
			 return false;
		fshader.Source (src);
		if (!fshader.Compile ())
		{
			(*logstream) << "Cannot compile "
									 << MakePath ("shaders", "shadowmap", "fshader.txt")
									 << ":" << std::endl << vshader.GetInfoLog () << std::endl;
			return false;
		}

		program.Attach (vshader);
		program.Attach (fshader);
		if (!program.Link ())
		{
			(*logstream) << "Cannot link the shadow map shader program:" << std::endl
									 << vshader.GetInfoLog () << std::endl;
			return false;
		}
	}
	{
		gl::Shader obj (GL_FRAGMENT_SHADER);
		std::string source;
		if (!ReadFile (MakePath ("shaders", "shadowmap", "hblur.txt"), source))
			 return false;
		obj.Source (source);
		if (!obj.Compile ())
		{
			(*logstream) << "Could not compile "
									 << MakePath ("shaders", "shadowmap", "hblur.txt")
									 << ": " << std::endl << obj.GetInfoLog () << std::endl;
			return false;
		}

		hblurprog.Parameter (GL_PROGRAM_SEPARABLE, GL_TRUE);
		hblurprog.Attach (obj);
		if (!hblurprog.Link ())
		{
			(*logstream) << "Could not link the shader program "
									 << MakePath ("shaders", "shadowmap", "hblur.txt")
									 << ": " << std::endl << hblurprog.GetInfoLog ()
									 << std::endl;
			 return false;
		}
	}
	{
		gl::Shader obj (GL_FRAGMENT_SHADER);
		std::string source;
		if (!ReadFile (MakePath ("shaders", "shadowmap", "vblur.txt"), source))
			 return false;
		obj.Source (source);
		if (!obj.Compile ())
		{
			(*logstream) << "Could not compile "
									 << MakePath ("shaders", "shadowmap", "vblur.txt")
									 << ": " << std::endl << obj.GetInfoLog () << std::endl;
			return false;
		}

		vblurprog.Parameter (GL_PROGRAM_SEPARABLE, GL_TRUE);
		vblurprog.Attach (obj);
		if (!vblurprog.Link ())
		{
			(*logstream) << "Could not link the shader program "
									 << MakePath ("shaders", "shadowmap", "vblur.txt")
									 << ": " << std::endl << vblurprog.GetInfoLog ()
									 << std::endl;
			 return false;
		}
	}

	hblurpipeline.UseProgramStages (GL_VERTEX_SHADER_BIT,
																	renderer->windowgrid.vprogram);
	hblurpipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT, hblurprog);
	vblurpipeline.UseProgramStages (GL_VERTEX_SHADER_BIT,
																	renderer->windowgrid.vprogram);
	vblurpipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT, vblurprog);

	width = config["shadowmap"]["width"].as<GLuint> ();
	height = config["shadowmap"]["height"].as<GLuint> ();

	hblurprog["invviewport"] = glm::vec2 (1.0f / width, 1.0f / height);
	vblurprog["invviewport"] = glm::vec2 (1.0f / width, 1.0f / height);

	sampler.Parameter (GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	sampler.Parameter (GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	sampler.Parameter (GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	sampler.Parameter (GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	shadowmap.Image2D (GL_TEXTURE_2D, 0, GL_RG32F, width, height,
										 0, GL_RG, GL_FLOAT, NULL);

	gl::Buffer::Unbind (GL_PIXEL_UNPACK_BUFFER);

	shadowmem = renderer->clctx.CreateFromGLTexture2D (CL_MEM_READ_ONLY,
																										 GL_TEXTURE_2D, 0,
																										 shadowmap);

	tmpstore.Image2D (GL_TEXTURE_2D, 0, GL_RG32F, width, height,
										0, GL_RG, GL_FLOAT, NULL);

	depthbuffer.Image2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
											 width, height, 0, GL_DEPTH_COMPONENT,
											 GL_FLOAT, NULL);

#ifdef DEBUG
	renderer->memory += width * height * (2 * 4 + 2 * 4 + 4);
#endif

	framebuffer.Texture2D (GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
												 depthbuffer, 0);
	framebuffer.DrawBuffers ({ });

	hblurfb.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
										 tmpstore, 0);
	hblurfb.DrawBuffers ({ GL_COLOR_ATTACHMENT0 });
	vblurfb.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
										 shadowmap, 0);
	vblurfb.DrawBuffers ({ GL_COLOR_ATTACHMENT0 });

	projmat = gl::SmartUniform<glm::mat4> (program["projmat"], glm::mat4(1));

	return true;
}

cl::Memory &ShadowMap::GetMem (void)
{
	return shadowmem;
}

gl::Texture &ShadowMap::GetMap (void)
{
	return shadowmap;
}

glm::mat4 ShadowMap::GetMat (void)
{
	return glm::mat4 (glm::vec4 (0.5, 0.0, 0.0, 0.0),
										glm::vec4 (0.0, 0.5, 0.0, 0.0),
										glm::vec4 (0.0, 0.0, 0.5, 0.0),
										glm::vec4 (0.5, 0.5, 0.5, 1.0))
		 * projmat.Get () * vmat;
}

void ShadowMap::Render (GLuint shadowid, Geometry &geometry,
												const Shadow &shadow)
{
	vmat = glm::lookAt (glm::vec3 (shadow.position),
											glm::vec3 (shadow.position + shadow.direction),
											glm::vec3 (1, 0, 0));
	projmat.Set (glm::perspective (2 * shadow.spot.angle
																 * 180.0f / float (DRE_PI),
																 (float) width / (float) height,
																 3.0f, 500.0f));
	renderer->culling.SetProjMatrix (projmat.Get ());

	framebuffer.Bind (GL_FRAMEBUFFER);

	program.Use ();

	gl::DepthMask (GL_TRUE);

	gl::ClearBufferfv (GL_DEPTH, 0, (const float[]) {1.0f});
	gl::Viewport (0, 0, width, height);

	gl::Enable (GL_DEPTH_TEST);

	GL_CHECK_ERROR;

	geometry.Render (Geometry::Pass::ShadowMap + shadowid * 0x00010000,
									 program, vmat);

	gl::DepthMask (GL_FALSE);

	gl::Program::UseNone ();

	hblurfb.Bind (GL_FRAMEBUFFER);
	hblurpipeline.Bind ();

	depthbuffer.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
	sampler.Bind (0);
	renderer->windowgrid.Render ();
	
	gl::Framebuffer::Unbind (GL_FRAMEBUFFER);

	vblurfb.Bind (GL_FRAMEBUFFER);
	vblurpipeline.Bind ();
		
	tmpstore.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
	sampler.Bind (0);
	renderer->windowgrid.Render ();
		
	gl::Framebuffer::Unbind (GL_FRAMEBUFFER);

	GL_CHECK_ERROR;
}

GLuint ShadowMap::GetWidth (void) const
{
	return width;
}

GLuint ShadowMap::GetHeight (void) const
{
	return height;
}
