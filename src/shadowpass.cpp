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
#include "renderer.h"

ShadowPass::ShadowPass (Renderer *parent)
	: renderer (parent), shadowmap (parent), soft_shadows (false)
{
}

ShadowPass::~ShadowPass (void)
{
}

bool ShadowPass::Init (void)
{
	std::vector<std::string> sources =
		 { MakePath ("shaders", "getpos.txt"),
			 MakePath ("shaders", "shadowpass.txt") };
	std::vector<gl::Shader> objs;

	if (!shadowmap.Init ())
		 return false;

	for (std::string &filename : sources)
	{
		std::string src;
		if (!ReadFile (filename, src))
			 return false;
		objs.emplace_back (GL_FRAGMENT_SHADER);
		objs.back ().Source (src);
		if (!objs.back ().Compile ())
		{
			(*logstream) << "Cannot compile " << filename << ":" << std::endl
									 << objs.back ().GetInfoLog () << std::endl;
			 return false;
		}
		fprogram.Attach (objs.back ());
	}

	fprogram.Parameter (GL_PROGRAM_SEPARABLE, GL_TRUE);
	if (!fprogram.Link ())
	{
		(*logstream) << "Cannot link the lightpass shader:" << std::endl
								 << fprogram.GetInfoLog () << std::endl;
		return false;
	}

	gl::Buffer::Unbind (GL_PIXEL_UNPACK_BUFFER);

	shadowmask.Image2D (GL_TEXTURE_2D, 0, GL_R8,
											renderer->gbuffer.width,
											renderer->gbuffer.height,
											0, GL_RED, GL_UNSIGNED_BYTE,
											NULL);

	framebuffer.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
												 shadowmask, 0);

	pipeline.UseProgramStages (GL_VERTEX_SHADER_BIT,
														 renderer->windowgrid.vprogram);
	pipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT, fprogram);
	
	fprogram["viewport"] = glm::uvec2 (renderer->gbuffer.width,
																		 renderer->gbuffer.height);

	framebuffer.DrawBuffers ({ GL_COLOR_ATTACHMENT0 });

	shadowmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, renderer->shadowpass.shadowmask);

	blur = renderer->filters.CreateBlur (shadowmem, 4.0f);

	return true;
}

void ShadowPass::FrameInit (void)
{
	framebuffer.Bind (GL_FRAMEBUFFER);
	gl::ClearBufferfv (GL_COLOR, 0, (float[]) {1.0f, 1.0f, 1.0f, 1.0f} );

	framebuffer.DrawBuffers ( { GL_COLOR_ATTACHMENT0 } );

	gl::Framebuffer::Unbind (GL_FRAMEBUFFER);
}

void ShadowPass::FrameFinish (void)
{
	if (soft_shadows)
	{
		blur.Apply ();
	}
}

void ShadowPass::SetSoftShadows (bool value)
{
	soft_shadows = value;
}

bool ShadowPass::GetSoftShadows (void)
{
	return soft_shadows;
}

void ShadowPass::Render (const Shadow &shadow)
{
	shadowmap.Render (renderer->geometry, shadow);

	fprogram["projinfo"] = renderer->camera.GetProjInfo ();
	fprogram["vmatinv"] = glm::inverse (renderer->camera.GetViewMatrix ());
	fprogram["shadow.mat"] = glm::mat4 (glm::vec4 (0.5, 0.0, 0.0, 0.0),
																			glm::vec4 (0.0, 0.5, 0.0, 0.0),
																			glm::vec4 (0.0, 0.0, 0.5, 0.0),
																			glm::vec4 (0.5, 0.5, 0.5, 1.0))
		 * shadowmap.projmat * shadowmap.vmat;

	framebuffer.Bind (GL_FRAMEBUFFER);
	gl::Viewport (0, 0, renderer->gbuffer.width, renderer->gbuffer.height);

	gl::Disable (GL_DEPTH_TEST);
	gl::DepthMask (GL_FALSE);

	gl::BlendEquation (GL_FUNC_REVERSE_SUBTRACT);
	gl::BlendFunc (GL_ONE, GL_ONE);
	gl::Enable (GL_BLEND);

	renderer->windowgrid.sampler.Bind (0);
	shadowmap.shadowmap.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
	renderer->windowgrid.sampler.Bind (1);
	renderer->gbuffer.depthbuffer.Bind (GL_TEXTURE1, GL_TEXTURE_2D);

	pipeline.Bind ();
	renderer->windowgrid.Render ();

	gl::DepthMask (GL_TRUE);
	gl::Disable (GL_BLEND);

	gl::Framebuffer::Unbind (GL_FRAMEBUFFER);
	GL_CHECK_ERROR;
}
