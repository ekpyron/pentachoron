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
		luminance_threshold (0.75),
		antialiasing (0)
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
	program.Build ("-cl-fast-relaxed-math -cl-mad-enable -cl-no-signed-zeros");
	composition = program.CreateKernel ("composition");

	screen.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F,
									renderer->gbuffer.GetWidth (),
									renderer->gbuffer.GetHeight (),
									0, GL_RGBA, GL_FLOAT, NULL);
	edgemap.Image2D (GL_TEXTURE_2D, 0, GL_R16F,
									 renderer->gbuffer.GetWidth (),
									 renderer->gbuffer.GetHeight (),
									 0, GL_RED, GL_FLOAT, NULL);
	glowmap.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F,
								renderer->gbuffer.GetWidth (),
								renderer->gbuffer.GetHeight (),
								0, GL_RGBA, GL_FLOAT, NULL);
	glowmap.Parameter (GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
	glowmap.GenerateMipmap (GL_TEXTURE_2D);

	if (!glow.Init (glowmap, 1))
		 return false;

	screenmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, screen);
	edgemem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, edgemap);
	glowmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, glowmap);

	SetAntialiasing (antialiasing);

	composition.SetArg (0, screenmem);
	composition.SetArg (1, glowmem);
	composition.SetArg (2, renderer->gbuffer.colormem);
	composition.SetArg (3, renderer->gbuffer.depthmem);
	composition.SetArg (4, renderer->gbuffer.normalmem);
	composition.SetArg (5, renderer->gbuffer.specularmem);
	composition.SetArg (6, renderer->shadowmap.shadowmapmem);

	cl_uint num_parameters = renderer->parameters.size ();
	composition.SetArg (10, sizeof (cl_uint), &num_parameters);
	composition.SetArg (11, renderer->parametermem);

	SetGlowSize (0);

	return true;
}

void Composition::SetGlowSize (GLuint size)
{
	glow.SetSize (size);
}

GLuint Composition::GetGlowSize (void)
{
	return glow.GetSize ();
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

void Composition::SetAntialiasing (GLuint size)
{
	if (size > 0)
	{
		freichen =  renderer->filters.CreateFreiChen
			 (screenmem, edgemem, renderer->gbuffer.GetWidth (),
				renderer->gbuffer.GetHeight ());
	}
	else
	{
		freichen = FreiChen ();
	}
	antialiasing = size;
}

GLuint Composition::GetAntialiasing (void)
{
	return antialiasing;
}

const gl::Texture &Composition::GetScreen (void)
{
	return screen;
}

const gl::Texture &Composition::GetGlowMap (void)
{
	return glow.GetMap ();
}

const gl::Texture &Composition::GetEdgeMap (void)
{
	return edgemap;
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
		 GLuint glowsize;
		 GLfloat padding;
	} Info;
	std::vector<cl::Memory> mem = { screenmem, glowmem,
																	renderer->gbuffer.colormem,
																	renderer->gbuffer.normalmem,
																	renderer->gbuffer.specularmem,
																	renderer->gbuffer.depthmem,
																	renderer->shadowmap.shadowmapmem };
	Info info;

	cl_uint num_lights = renderer->lights.size ();

	info.projinfo = renderer->camera.GetProjInfo ();
	info.vmatinv = glm::transpose
		 (glm::inverse (renderer->camera.GetViewMatrix ()));
	info.shadowmat = glm::transpose (glm::mat4 (glm::vec4 (0.5, 0.0, 0.0, 0.0),
																							glm::vec4 (0.0, 0.5, 0.0, 0.0),
																							glm::vec4 (0.0, 0.0, 0.5, 0.0),
																							glm::vec4 (0.5, 0.5, 0.5, 1.0))
																	 * renderer->shadowmap.projmat
																	 * renderer->shadowmap.vmat);
	info.eye = glm::vec4 (renderer->camera.GetEye (), 0.0);
	info.center = glm::vec4 (renderer->camera.GetCenter (), 0.0);
	info.luminance_threshold = luminance_threshold;
	info.shadow_alpha = shadow_alpha;
	info.glowsize = glow.GetSize ();

	composition.SetArg (7, sizeof (cl_uint), &num_lights);
	composition.SetArg (8, renderer->lightmem);
	composition.SetArg (9, sizeof (Info), &info);

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

	if (antialiasing > 0)
	{
		 freichen.Apply ();
	}
}
