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
#ifndef GBUFFER_H
#define GBUFFER_H

#include <common.h>
#include "geometry.h"

class Renderer;

class GBuffer
{
public:
	 GBuffer (Renderer *parent);
	 ~GBuffer (void);
	 bool Init (void);
	 void Render (Geometry &geometry);

	 gl::Texture colorbuffer;
	 cl::Memory colormem;
	 gl::Texture normalbuffer;
	 cl::Memory normalmem;
	 gl::Texture specularbuffer;
	 cl::Memory specularmem;
	 gl::Texture depthtexture;
	 gl::Renderbuffer depthbuffer;
	 cl::Memory depthmem;
	 GLuint GetWidth (void);
	 GLuint GetHeight (void);
private:
	 GLuint width, height;

	 gl::Framebuffer framebuffer;

	 gl::Program program;
	 gl::Sampler depthsampler;

	 Renderer *renderer;
	 friend class Renderer;
};


#endif /* !defined GBUFFER_H */
