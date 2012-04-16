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
#ifndef COMPOSITION_H
#define COMPOSITION_H

#include <common.h>
#include <gbuffer.h>
#include <filters.h>

class Renderer;

class Composition
{
public:
	 Composition (Renderer *parent);
	 ~Composition (void);
	 bool Init (void);
	 void Frame (float timefactor);

	 gl::Texture screen;
	 gl::Texture glow;
private:
	 Blur blur;
	 cl::Program program;
	 cl::Kernel composition;
	 cl::Memory screenmem;
	 cl::Memory glowmem_full;
	 cl::Memory glowmem_downsampled;
	 cl::Memory colormem[GBuffer::layers];
	 cl::Memory normalmem[GBuffer::layers];
	 cl::Memory specularmem[GBuffer::layers];
	 Renderer *renderer;
};

#endif /* !defined COMPOSITION_H */
