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

long double Coeff (unsigned long n, unsigned long k)
{
	long double ret;
	ret = 1.0 / powl (2.0, n);
	for (unsigned long i = 0; i < k; i++)
	{
		ret *= (long double) (n - i);
		ret /= (long double) (i+1);
	}
	return ret;
}

Blur Filters::CreateBlur (cl::Memory &memory, GLuint width,
													GLuint height, GLuint size)
{
	cl::Memory weights;
	cl::Memory offsets;
	cl::Memory storage;
	std::vector<float> weights_data;
	std::vector<float> offsets_data;
	std::vector<float> linear_weights_data;
	std::vector<float> linear_offsets_data;

	if (size == 0)
		 return Blur ();

	// round up
	size = (size + 3) & ~3;

	for (int i = 0; i <= (size>>1); i++)
	{
		weights_data.push_back (Coeff (size, (size>>1) - i));
	}

	for (int i = 0; i <= (size>>1); i++)
	{
		offsets_data.push_back (float (i));
	}

	linear_weights_data.push_back (weights_data[0]);
	linear_offsets_data.push_back (offsets_data[0]);

	for (int i = 1; i <= (size>>2); i++)
	{
		linear_weights_data.push_back (weights_data[i * 2 - 1] +
																	 weights_data[i * 2]);
		linear_offsets_data.push_back ((offsets_data[i * 2 - 1]
																		* weights_data[i * 2 - 1]
																		+ offsets_data[i * 2]
																		* weights_data[i * 2])
																	 / linear_weights_data[i]);
	}

	storage = renderer->clctx.CreateImage2D (CL_MEM_READ_WRITE,
																					 CL_RGBA,
																					 CL_FLOAT,
																					 width, height, 0,
																					 NULL);
																					 

	weights = renderer->clctx.CreateBuffer (CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
																					linear_weights_data.size ()
																					* sizeof (float),
																					&linear_weights_data[0]);
	offsets = renderer->clctx.CreateBuffer (CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
																					linear_offsets_data.size ()
																					* sizeof (float),
																					&linear_offsets_data[0]);

	cl_int num_weights = linear_weights_data.size ();

	return Blur (&memory, storage, weights, offsets, num_weights, size,
							 width, height, this);
}

void Filters::ApplyBlur (const cl::Memory *memory, const cl::Memory &storage,
												 const cl::Memory &weights, const cl::Memory &offsets,
												 GLuint num_weights, GLuint width, GLuint height)
{
	hblur.SetArg (0, *memory);
	hblur.SetArg (1, storage);
	hblur.SetArg (2, weights);
	hblur.SetArg (3, offsets);
	hblur.SetArg (4, sizeof (num_weights), &num_weights);

	vblur.SetArg (0, storage);
	vblur.SetArg (1, *memory);
	vblur.SetArg (2, weights);
	vblur.SetArg (3, offsets);
	vblur.SetArg (4, sizeof (num_weights), &num_weights);

	const size_t work_dim[] = { width, height };

	renderer->queue.EnqueueAcquireGLObjects ({ *memory }, 0, NULL, NULL);
	renderer->queue.EnqueueNDRangeKernel (hblur, 2, NULL, work_dim,
																				NULL, 0, NULL, NULL);
	renderer->queue.EnqueueNDRangeKernel (vblur, 2, NULL, work_dim,
																				NULL, 0, NULL, NULL);
	renderer->queue.EnqueueReleaseGLObjects ({ *memory }, 0, NULL, NULL);
}

Blur::Blur (void) : parent (NULL), num_weights (0), width (0), height (0),
										size (0)
{
}

Blur::Blur (const cl::Memory *mem, cl::Memory s, cl::Memory w,
						cl::Memory o, GLuint num, GLuint sz,
						GLuint x, GLuint y, Filters *p)
	: parent (p), memory (mem), storage (s), weights (w), 
		offsets (o), num_weights (num), width (x), height (y), size (sz)
{
}

Blur::Blur (Blur &&blur)
	: num_weights (blur.num_weights), memory (blur.memory),
		weights (std::move (blur.weights)), parent (blur.parent),
		storage (std::move (blur.storage)), size (blur.size),
		width (blur.width), height (blur.height),
		offsets (std::move (blur.offsets))
{
	blur.size = 0;
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
	size = blur.size;
	weights = std::move (blur.weights);
	offsets = std::move (blur.offsets);
	storage = std::move (blur.storage);
	memory = blur.memory;
	parent = blur.parent;
	blur.num_weights = 0;
	blur.width = 0;
	blur.height = 0;
	blur.size = 0;
}

GLuint Blur::GetSize (void)
{
	return size;
}

void Blur::Apply (void)
{
	if (size == 0)
		 return;
	parent->ApplyBlur (memory, storage, weights, offsets,
										 num_weights, width, height);
}
