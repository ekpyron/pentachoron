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
#include "filters.h"
#include "renderer.h"

Filters::Filters (Renderer *parent)
	: renderer (parent)
{
}

Filters::~Filters (void)
{
}

bool Filters::Init (void)
{
	std::string src;
	if (!ReadFile (MakePath ("kernels", "blur.cl"), src))
		 return false;
	clblur = renderer->clctx.CreateProgramWithSource (src);
	clblur.Build ("-cl-fast-relaxed-math -cl-mad-enable -cl-no-signed-zeros");
	hblur = clblur.CreateKernel ("hblur");
	vblur = clblur.CreateKernel ("vblur");

	return true;
}

Blur Filters::CreateBlur (cl::Memory &memory, GLuint width,
													GLuint height, float sigma)
{
	cl::Memory weights;
	cl::Memory storage;
	std::vector<float> weights_data;
	for (int i = 0; i < ceil (2 * sigma); i++)
	{
		weights_data.push_back ((1.0f / (sqrtf (2.0f * M_PI * sigma * sigma)))
														* expf (-(i * i) / (2.0f * sigma * sigma)));
	}

	storage = renderer->clctx.CreateBuffer (CL_MEM_READ_WRITE,
																					width * height
																					* sizeof (cl_float) * 4,
																					NULL);

	weights = renderer->clctx.CreateBuffer (CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
																					weights_data.size () * sizeof (float),
																					&weights_data[0]);

	cl_int num_weights = weights_data.size ();

	return Blur (&memory, storage, weights, num_weights, width, height, this);
}

void Filters::ApplyBlur (const cl::Memory *memory, const cl::Memory &storage,
												 const cl::Memory &weights, GLuint num_weights,
												 GLuint width, GLuint height)
{
	hblur.SetArg (0, *memory);
	hblur.SetArg (1, storage);
	hblur.SetArg (2, weights);
	hblur.SetArg (3, sizeof (num_weights), &num_weights);

	vblur.SetArg (0, storage);
	vblur.SetArg (1, *memory);
	vblur.SetArg (2, weights);
	vblur.SetArg (3, sizeof (num_weights), &num_weights);

	const size_t work_dim[] = { width, height};

	renderer->queue.EnqueueAcquireGLObjects ({ *memory }, 0, NULL, NULL);
	renderer->queue.EnqueueNDRangeKernel (hblur, 2, NULL, work_dim,
																				NULL, 0, NULL, NULL);
	renderer->queue.EnqueueNDRangeKernel (vblur, 2, NULL, work_dim,
																				NULL, 0, NULL, NULL);
	renderer->queue.EnqueueReleaseGLObjects ({ *memory }, 0, NULL, NULL);
}

Blur::Blur (void) : parent (NULL)
{
}

Blur::Blur (const cl::Memory *mem, cl::Memory s, cl::Memory w,
						GLuint num, GLuint x, GLuint y, Filters *p)
	: parent (p), memory (mem), storage (s), weights (w), num_weights (num),
		width (x), height (y)
{
}

Blur::Blur (Blur &&blur)
	: num_weights (blur.num_weights), memory (blur.memory),
		weights (std::move (blur.weights)), parent (blur.parent),
		storage (std::move (blur.storage)), width (blur.width),
		height (blur.height)
{
	blur.num_weights = 0;
	blur.width = 0;
	blur.height = 0;
}

Blur::~Blur (void)
{
}

Blur &Blur::operator= (Blur &&blur)
{
	width = blur.width;
	height = blur.height;
	num_weights = blur.num_weights;
	weights = std::move (blur.weights);
	storage = std::move (blur.storage);
	memory = blur.memory;
	parent = blur.parent;
	blur.num_weights = 0;
	blur.width = 0;
	blur.height = 0;
}

void Blur::Apply (void)
{
	parent->ApplyBlur (memory, storage, weights, num_weights, width, height);
}
