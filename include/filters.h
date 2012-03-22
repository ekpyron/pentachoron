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
#ifndef FILTERS_H
#define FILTERS_H

#include <common.h>

class Renderer;
class Filters;

class Blur
{
public:
	 Blur (void);
	 Blur (Blur &&blur);
	 Blur (const Blur&) = delete;
	 ~Blur (void);
	 void Apply (void);
	 Blur &operator= (const Blur&) = delete;
	 Blur &operator= (Blur &&blur);
private:
	 const cl::Memory *memory;
	 cl::Memory weights;
	 int num_weights;
	 Filters *parent;
	 Blur (const cl::Memory *memory, cl::Memory weights,
				 int num_weights, Filters *p);
	 friend class Filters;
};

class Filters
{
public:
	 Filters (Renderer *parent);
	 Filters (const Filters&) = delete;
	 ~Filters (void);
	 Filters &operator= (const Filters&) = delete;
	 bool Init (void);

	 Blur CreateBlur (cl::Memory &memory, float sigma);
private:
	 void ApplyBlur (const cl::Memory *memory, const cl::Memory &weights,
									 int num_weights);
	 cl::CommandQueue queue;
	 cl::Program clblur;
	 cl::Kernel hblur, vblur;
	 cl::Memory storage;
	 Renderer *renderer;
	 friend class Blur;
};

#endif /* !defined FILTERS_H */
