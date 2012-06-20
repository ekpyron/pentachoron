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
#include "composition.h"
#include "renderer.h"

Composition::Composition (Renderer *parent)
	: renderer (parent), glow (parent), shadow_alpha (0.7),
		luminance_threshold (0.75)
{
}

Composition::~Composition (void)
{
}

bool Composition::Init (void)
{
	std::string src;
	if (!ReadFile (MakePath ("kernels", "composition.cl"), src))
		 return false;

	program = renderer->clctx.CreateProgramWithSource (src);
	{
		std::string options ("-cl-fast-relaxed-math -cl-mad-enable "
												 "-cl-no-signed-zeros");
		if (renderer->clctx.IsExtensionSupported ("cl_nv_compiler_options"))
		{
			options.append (" -cl-nv-maxrregcount=32");
		}
		program.Build (options);
	}
	composition = program.CreateKernel ("composition");

	screen.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F,
									renderer->gbuffer.GetWidth (),
									renderer->gbuffer.GetHeight (),
									0, GL_RGBA, GL_FLOAT, NULL);
	glowmap.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F,
								renderer->gbuffer.GetWidth (),
								renderer->gbuffer.GetHeight (),
								0, GL_RGBA, GL_FLOAT, NULL);

	glowmap.GenerateMipmap (GL_TEXTURE_2D);

#ifdef DEBUG
	renderer->memory += renderer->gbuffer.GetWidth ()
		 * renderer->gbuffer.GetHeight () * (4 * 2 + 4 * 2 + 4);
	renderer->memory += (renderer->gbuffer.GetWidth () >> 1) *
		 (renderer->gbuffer.GetHeight () >> 1) * 4;
	renderer->memory += (renderer->gbuffer.GetWidth () >> 1) *
		 (renderer->gbuffer.GetHeight () >> 2) * 4;
  /* ignore smaller mipmap levels... */
#endif

	if (!glow.Init (screen, glowmap,
									config["glow"]["mipmaplevel"].as<unsigned int> (2)))
		 return false;

	screenmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, screen);
	glowmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, glowmap);

	composition.SetArg (0, screenmem);
	composition.SetArg (1, glowmem);
	composition.SetArg (2, renderer->gbuffer.colormem);
	composition.SetArg (3, renderer->gbuffer.depthmem);
	composition.SetArg (4, renderer->gbuffer.normalmem);
	composition.SetArg (5, renderer->gbuffer.specularmem);
	composition.SetArg (6, renderer->shadowmap.GetMem ());
	composition.SetArg (7, renderer->gbuffer.fragidxmem);
	composition.SetArg (8, renderer->gbuffer.fraglistmem);

	cl_uint num_parameters = renderer->GetNumParameters ();
	composition.SetArg (11, sizeof (cl_uint), &num_parameters);
	composition.SetArg (12, renderer->GetParameterMem ());

	glow.SetSize (0);

	return true;
}

Glow &Composition::GetGlow (void)
{
	return glow;
}

void Composition::SetLuminanceThreshold (float threshold)
{
	luminance_threshold = threshold;
}

float Composition::GetLuminanceThreshold (void)
{
	return luminance_threshold;
}

void Composition::SetShadowAlpha (float alpha)
{
	if (alpha < 0)
		 alpha = 0;
	if (alpha > 1)
		 alpha = 1;
	shadow_alpha = alpha;
}

float Composition::GetShadowAlpha (void)
{
	return shadow_alpha;
}

const gl::Texture &Composition::GetScreen (void)
{
	return screen;
}

#define NUM_COMPOSITIONMODES   3

void Composition::SetMode (GLuint m)
{
	mode = m;
	if (mode >= NUM_COMPOSITIONMODES)
		 mode = NUM_COMPOSITIONMODES - 1;
}

GLuint Composition::GetMode (void)
{
	return mode;
}

void Composition::Frame (float timefactor)
{
	typedef struct Info
	{
		 glm::vec4 projinfo;
		 glm::mat4 vmatinv;
		 glm::mat4 shadowmat;
		 glm::vec4 eye;
		 glm::vec4 center;
		 GLfloat luminance_threshold;
		 GLfloat shadow_alpha;
		 GLuint mode;
		 cl_uint num_lights;
		 struct
		 {
				GLuint size;
				float limit;
				float exponent;
				GLuint padding;
		 } glow;
	} Info;
	std::vector<cl::Memory> mem = { screenmem, glowmem,
																	renderer->gbuffer.colormem,
																	renderer->gbuffer.normalmem,
																	renderer->gbuffer.specularmem,
																	renderer->gbuffer.depthmem,
																	renderer->gbuffer.fraglistmem,
																	renderer->gbuffer.fragidxmem,
																	renderer->shadowmap.GetMem () };
	Info info;

	info.num_lights = renderer->GetNumLights ();

	info.projinfo = renderer->camera.GetProjInfo ();
	info.vmatinv = glm::transpose
		 (glm::inverse (renderer->camera.GetViewMatrix ()));
	info.shadowmat = glm::transpose
		 (renderer->shadowmap.GetMat ());
	info.eye = glm::vec4 (renderer->camera.GetEye (), 0.0);
	info.center = glm::vec4 (renderer->camera.GetCenter (), 0.0);
	info.luminance_threshold = luminance_threshold;
	info.shadow_alpha = shadow_alpha;
	info.glow.size = glow.GetSize ();
	info.glow.limit = glow.GetLimit ();
	info.glow.exponent = glow.GetExponent ();
	info.mode = mode;

	composition.SetArg (9, renderer->GetLightMem ());
	composition.SetArg (10, sizeof (Info), &info);

	const size_t work_dim[] = { renderer->gbuffer.GetWidth (),
															renderer->gbuffer.GetHeight () };
	const size_t local_dim[] = { 16, 16 };

	renderer->queue.EnqueueAcquireGLObjects (mem, 0, NULL, NULL);
	renderer->queue.EnqueueNDRangeKernel (composition, 2, NULL, work_dim,
																				local_dim, 0, NULL, NULL);
	renderer->queue.EnqueueReleaseGLObjects (mem, 0, NULL, NULL);

	if (glow.GetSize () > 0)
	{
		glowmap.GenerateMipmap (GL_TEXTURE_2D);
		glow.Apply ();
	}
}
