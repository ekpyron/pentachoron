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
#include "shadowpass.h"
#include "shadowmap.h"
#include "renderer.h"

ShadowPass::ShadowPass (Renderer *parent)
	: renderer (parent), shadowmap (parent)
{
}

ShadowPass::~ShadowPass (void)
{
}

const gl::Texture &ShadowPass::GetShadowMask (void)
{
	return shadowmask;
}

const cl::Memory &ShadowPass::GetShadowMaskMem (void)
{
	return shadowmaskmem;
}

bool ShadowPass::Init (void)
{
	if (!shadowmap.Init ())
		 return false;

	{
		gl::Shader obj (GL_FRAGMENT_SHADER);
		std::string source;
		if (!ReadFile (MakePath ("shaders", "shadowpass.txt"), source))
			 return false;
		obj.Source (source);
		if (!obj.Compile ())
		{
			(*logstream) << "Could not compile "
									 << MakePath ("shaders", "shadowpass.txt")
									 << ": " << std::endl << obj.GetInfoLog () << std::endl;
			 return false;
		}

		fprogram.Parameter (GL_PROGRAM_SEPARABLE, GL_TRUE);
		fprogram.Attach (obj);
		if (!fprogram.Link ())
		{
			(*logstream) << "Could not link the shader program "
									 << MakePath ("shaders", "shadowpass.txt")
									 << ": " << std::endl << fprogram.GetInfoLog ()
									 << std::endl;
			 return false;
		}
	}

	pipeline.UseProgramStages (GL_VERTEX_SHADER_BIT,
														 renderer->windowgrid.vprogram);
	pipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT, fprogram);

	shadowmask.Image2D (GL_TEXTURE_2D, 0, GL_R8, renderer->gbuffer.GetWidth (),
											renderer->gbuffer.GetHeight (), 0, GL_RED,
											GL_UNSIGNED_BYTE, NULL);

	shadowmaskmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, shadowmask);

	framebuffer.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
												 shadowmask, 0);
	framebuffer.DrawBuffers ({ GL_COLOR_ATTACHMENT0 });

	fprogram["viewport"] = glm::uvec2 (renderer->gbuffer.GetWidth (),
																		 renderer->gbuffer.GetHeight ());

	sampler.Parameter (GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	sampler.Parameter (GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	sampler.Parameter (GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	sampler.Parameter (GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return true;
}

void ShadowPass::SetSoftShadows (bool softshadows)
{
	shadowmap.SetSoftShadows (softshadows);
}

bool ShadowPass::GetSoftShadows (void)
{
	return shadowmap.GetSoftShadows ();
}

void ShadowPass::Render (GLuint shadowid, Geometry &geometry,
												 const Shadow &shadow)
{
	shadowmap.Render (shadowid, geometry, shadow);
	framebuffer.Bind (GL_FRAMEBUFFER);
	gl::Viewport (0, 0, renderer->gbuffer.GetWidth (),
								renderer->gbuffer.GetHeight ());
	fprogram["vmatinv"] = glm::inverse (renderer->camera.GetViewMatrix ());
	fprogram["projinfo"]
		 = renderer->camera.GetProjInfo ();
	fprogram["shadowmat"] = glm::mat4 (glm::vec4 (0.5, 0.0, 0.0, 0.0),
																		 glm::vec4 (0.0, 0.5, 0.0, 0.0),
																		 glm::vec4 (0.0, 0.0, 0.5, 0.0),
																		 glm::vec4 (0.5, 0.5, 0.5, 1.0))
		 * shadowmap.projmat.Get ()
		 * shadowmap.vmat;

	pipeline.Bind ();

	sampler.Bind (0);
	shadowmap.shadowmap.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
	renderer->windowgrid.sampler.Bind (1);
	renderer->gbuffer.depthtexture.Bind (GL_TEXTURE1, GL_TEXTURE_2D);
	renderer->windowgrid.Render ();

	gl::Framebuffer::Unbind (GL_FRAMEBUFFER);

	GL_CHECK_ERROR;
}
