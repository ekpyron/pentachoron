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
	: renderer (parent), luminance_threshold (0.75),
		shadow_alpha (0.7)
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
									renderer->gbuffer.width,
									renderer->gbuffer.height,
									0, GL_RGBA, GL_FLOAT, NULL);
	glow.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F,
									renderer->gbuffer.width,
									renderer->gbuffer.height,
									0, GL_RGBA, GL_FLOAT, NULL);
	glow.Parameter (GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 2);
	glow.GenerateMipmap (GL_TEXTURE_2D);

	screenmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, screen);
	glowmem_full = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, glow);
	glowmem_downsampled = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_WRITE, GL_TEXTURE_2D, 2, glow);

	composition.SetArg (0, screenmem);
	composition.SetArg (1, glowmem_full);
	composition.SetArg (2, renderer->gbuffer.colormem[0]);
	composition.SetArg (3, renderer->gbuffer.colormem[1]);
	composition.SetArg (4, renderer->gbuffer.colormem[2]);
	composition.SetArg (5, renderer->gbuffer.colormem[3]);
	composition.SetArg (6, renderer->gbuffer.depthmem[0]);
	composition.SetArg (7, renderer->gbuffer.depthmem[1]);
	composition.SetArg (8, renderer->gbuffer.depthmem[2]);
	composition.SetArg (9, renderer->gbuffer.depthmem[3]);
	composition.SetArg (10, renderer->gbuffer.normalmem[0]);
	composition.SetArg (11, renderer->gbuffer.normalmem[1]);
	composition.SetArg (12, renderer->gbuffer.normalmem[2]);
	composition.SetArg (13, renderer->gbuffer.normalmem[3]);
	composition.SetArg (14, renderer->gbuffer.specularmem[0]);
	composition.SetArg (15, renderer->gbuffer.specularmem[1]);
	composition.SetArg (16, renderer->gbuffer.specularmem[2]);
	composition.SetArg (17, renderer->gbuffer.specularmem[3]);
	composition.SetArg (18, renderer->shadowmap.shadowmapmem);

	blur = renderer->filters.CreateBlur (glowmem_downsampled,
																			 renderer->gbuffer.width >> 2,
																			 renderer->gbuffer.height >> 2, 8);

	return true;
}

void Composition::Frame (float timefactor)
{
	typedef struct Info
	{
		 glm::vec4 projinfo;
		 glm::mat4 vmatinv;
		 glm::mat4 shadowmat;
		 glm::vec4 eye;
		 GLfloat luminance_threshold;
		 GLfloat shadow_alpha;
		 GLfloat padding[2];
	} Info;
	std::vector<cl::Memory> mem = { screenmem, glowmem_full,
																	glowmem_downsampled,
																	renderer->gbuffer.colormem[0],
																	renderer->gbuffer.colormem[1],
																	renderer->gbuffer.colormem[2],
																	renderer->gbuffer.colormem[3],
																	renderer->gbuffer.normalmem[0],
																	renderer->gbuffer.normalmem[1],
																	renderer->gbuffer.normalmem[2],
																	renderer->gbuffer.normalmem[3],
																	renderer->gbuffer.specularmem[0],
																	renderer->gbuffer.specularmem[1],
																	renderer->gbuffer.specularmem[2],
																	renderer->gbuffer.specularmem[3],
																	renderer->gbuffer.depthmem[0],
																	renderer->gbuffer.depthmem[1],
																	renderer->gbuffer.depthmem[2],
																	renderer->gbuffer.depthmem[3],
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
	info.luminance_threshold = luminance_threshold;
	info.shadow_alpha = shadow_alpha;

	composition.SetArg (19, sizeof (cl_uint), &num_lights);
	composition.SetArg (20, renderer->lightmem);
	composition.SetArg (21, sizeof (Info), &info);

	cl_uint num_parameters = renderer->parameters.size ();
	composition.SetArg (22, sizeof (cl_uint), &num_parameters);
	composition.SetArg (23, renderer->parametermem);

	const size_t work_dim[] = { renderer->gbuffer.width,
															renderer->gbuffer.height };
	const size_t local_dim[] = { 16, 16 };

	renderer->queue.EnqueueAcquireGLObjects (mem, 0, NULL, NULL);
	renderer->queue.EnqueueNDRangeKernel (composition, 2, NULL, work_dim,
																				local_dim, 0, NULL, NULL);
	renderer->queue.EnqueueReleaseGLObjects (mem, 0, NULL, NULL);

	glow.GenerateMipmap (GL_TEXTURE_2D);

	blur.Apply ();
}
