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
	program.Build ("-cl-fast-relaxed-math");
	composition = program.CreateKernel ("composition");

	screen.Image2D (GL_TEXTURE_2D, 0, GL_RGBA8,
									renderer->gbuffer.width,
									renderer->gbuffer.height,
									0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	screenmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, screen);

	colormem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, renderer->gbuffer.colorbuffer);

	normalmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, renderer->gbuffer.normalbuffer);

	specularmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, renderer->gbuffer.specularbuffer);

	depthmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, renderer->gbuffer.depthtexture);

	queue = renderer->clctx.CreateCommandQueue (0);

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

	ViewInfo info;

	cl_uint num_lights = renderer->lights.size ();

	composition.SetArg (0, screenmem);
	composition.SetArg (1, colormem);
	composition.SetArg (2, depthmem);
	composition.SetArg (3, normalmem);
	composition.SetArg (4, specularmem);
	composition.SetArg (5, renderer->shadowpass.shadowmem);
	composition.SetArg (6, sizeof (cl_uint), &num_lights);
	composition.SetArg (7, renderer->lightmem);

	info.projinfo = renderer->camera.GetProjInfo ();
	info.vmatinv = glm::transpose
		 (glm::inverse (renderer->camera.GetViewMatrix ()));
	info.eye = glm::vec4 (renderer->camera.GetEye (), 0.0);

	composition.SetArg (8, sizeof (ViewInfo), &info);

	const size_t work_dim[] = { renderer->gbuffer.width,
															renderer->gbuffer.height };
	const size_t local_dim[] = { 16, 16 };

	queue.EnqueueAcquireGLObjects ({ screenmem, colormem, normalmem,
				 specularmem, depthmem, renderer->shadowpass.shadowmem },
		0, NULL, NULL);
	queue.EnqueueNDRangeKernel (composition, 2, NULL, work_dim,
															local_dim, 0, NULL, NULL);
	queue.EnqueueReleaseGLObjects ({ screenmem, colormem, normalmem,
				 specularmem, depthmem, renderer->shadowpass.shadowmem },
		0, NULL, NULL);
	queue.Flush ();
}
