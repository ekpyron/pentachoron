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
	std::string src;
	if (!ReadFile (MakePath ("kernels", "shadow.cl"), src))
		 return false;

	program = renderer->clctx.CreateProgramWithSource (src);
	program.Build ("-cl-fast-relaxed-math -cl-mad-enable -cl-no-signed-zeros");
	genshadow = program.CreateKernel ("genshadow");

	queue = renderer->clctx.CreateCommandQueue (0);

	if (!shadowmap.Init ())
		 return false;

	shadowmapmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_ONLY, GL_TEXTURE_RECTANGLE, 0, shadowmap.shadowmap);

	gl::Buffer::Unbind (GL_PIXEL_UNPACK_BUFFER);
	shadowmask.Image2D (GL_TEXTURE_2D, 0, GL_RGBA8,
											renderer->gbuffer.width,
											renderer->gbuffer.height,
											0, GL_RED, GL_UNSIGNED_BYTE,
											NULL);
	shadowmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, shadowmask);

	blur = renderer->filters.CreateBlur (shadowmem, 4.0f);

	return true;
}

void ShadowPass::FrameInit (void)
{
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
	typedef struct ViewInfo
	{
		 glm::vec4 projinfo;
		 glm::mat4 vmatinv;
		 glm::mat4 shadowmat;
	} ViewInfo;
	ViewInfo info;
	std::vector<cl::Memory> mem = {
		shadowmem, shadowmapmem, renderer->gbuffer.depthmem[0],
		renderer->gbuffer.depthmem[1], renderer->gbuffer.depthmem[2],
		renderer->gbuffer.depthmem[3]
	};

	shadowmap.Render (renderer->geometry, shadow);

	info.projinfo = renderer->camera.GetProjInfo ();
	info.vmatinv = glm::transpose (glm::inverse
																 (renderer->camera.GetViewMatrix ()));
	info.shadowmat = glm::transpose (glm::mat4 (glm::vec4 (0.5, 0.0, 0.0, 0.0),
																							glm::vec4 (0.0, 0.5, 0.0, 0.0),
																							glm::vec4 (0.0, 0.0, 0.5, 0.0),
																							glm::vec4 (0.5, 0.5, 0.5, 1.0))
																	 * shadowmap.projmat * shadowmap.vmat);

	const size_t work_dim[] = { renderer->gbuffer.width,
															renderer->gbuffer.height };
	const size_t local_dim[] = { 16, 16 };

	genshadow.SetArg (0, shadowmem);
	genshadow.SetArg (1, renderer->gbuffer.depthmem[0]);
	genshadow.SetArg (2, renderer->gbuffer.depthmem[1]);
	genshadow.SetArg (3, renderer->gbuffer.depthmem[2]);
	genshadow.SetArg (4, renderer->gbuffer.depthmem[3]);
	genshadow.SetArg (5, shadowmapmem);
	genshadow.SetArg (6, sizeof (ViewInfo), &info);

	
	queue.EnqueueAcquireGLObjects (mem, 0, NULL, NULL);
	queue.EnqueueNDRangeKernel (genshadow, 2, NULL, work_dim,
															local_dim, 0, NULL, NULL);

	queue.EnqueueReleaseGLObjects (mem, 0, NULL, NULL);
}
