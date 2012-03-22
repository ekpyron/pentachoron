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
	 void Render (const Geometry &geometry);

	 gl::Texture colorbuffer;
	 gl::Texture normalbuffer;
	 gl::Texture specularbuffer;
	 gl::Texture depthbuffer;
	 gl::Renderbuffer depthbuffer_internal;

	 GLuint width, height;

private:
	 gl::Program program;
	 gl::Framebuffer framebuffer;

	 Renderer *renderer;
	 friend class Renderer;
};


#endif /* !defined GBUFFER_H */
