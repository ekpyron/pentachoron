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
	: renderer (parent)
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

	screen.Image2D (GL_TEXTURE_2D, 0, GL_RGBA8,
									renderer->gbuffer.width,
									renderer->gbuffer.height,
									0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	screenmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, screen);
	for (auto i = 0; i < GBuffer::layers; i++)
	{
		colormem[i] = renderer->clctx.CreateFromGLTexture2D
			 (CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0,
				renderer->gbuffer.colorbuffer[i]);

		normalmem[i] = renderer->clctx.CreateFromGLTexture2D
			 (CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0,
				renderer->gbuffer.normalbuffer[i]);
	
		specularmem[i] = renderer->clctx.CreateFromGLTexture2D
			 (CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0,
				renderer->gbuffer.specularbuffer[i]);
	}

	queue = renderer->clctx.CreateCommandQueue (0);

	composition.SetArg (0, screenmem);
	composition.SetArg (1, colormem[0]);
	composition.SetArg (2, colormem[1]);
	composition.SetArg (3, colormem[2]);
	composition.SetArg (4, renderer->gbuffer.depthmem[0]);
	composition.SetArg (5, renderer->gbuffer.depthmem[1]);
	composition.SetArg (6, renderer->gbuffer.depthmem[2]);
	composition.SetArg (7, normalmem[0]);
	composition.SetArg (8, normalmem[1]);
	composition.SetArg (9, normalmem[2]);
	composition.SetArg (10, specularmem[0]);
	composition.SetArg (11, specularmem[1]);
	composition.SetArg (12, specularmem[2]);
	composition.SetArg (13, renderer->shadowpass.shadowmem);

	return true;
}

void Composition::Frame (float timefactor)
{
	typedef struct ViewInfo
	{
		 glm::vec4 projinfo;
		 glm::mat4 vmatinv;
		 glm::vec4 eye;
	} ViewInfo;
	std::vector<cl::Memory> mem = { screenmem, colormem[0], colormem[1],
																	colormem[2],  normalmem[0], normalmem[1],
																	normalmem[2], specularmem[0],
																	specularmem[1], specularmem[2],
																	renderer->gbuffer.depthmem[0],
																	renderer->gbuffer.depthmem[1],
																	renderer->gbuffer.depthmem[2],
																	renderer->shadowpass.shadowmem };
	ViewInfo info;

	cl_uint num_lights = renderer->lights.size ();

	info.projinfo = renderer->camera.GetProjInfo ();
	info.vmatinv = glm::transpose
		 (glm::inverse (renderer->camera.GetViewMatrix ()));
	info.eye = glm::vec4 (renderer->camera.GetEye (), 0.0);

	composition.SetArg (14, sizeof (cl_uint), &num_lights);
	composition.SetArg (15, renderer->lightmem);
	composition.SetArg (16, sizeof (ViewInfo), &info);

	const size_t work_dim[] = { renderer->gbuffer.width,
															renderer->gbuffer.height };
	const size_t local_dim[] = { 16, 16 };

	queue.EnqueueAcquireGLObjects (mem, 0, NULL, NULL);
	queue.EnqueueNDRangeKernel (composition, 2, NULL, work_dim,
															local_dim, 0, NULL, NULL);
	queue.EnqueueReleaseGLObjects (mem, 0, NULL, NULL);
	queue.Flush ();
}
