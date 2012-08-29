/*  
 * This file is part of Pentachoron.
 *
 * Pentachoron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pentachoron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pentachoron.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef GBUFFER_H
#define GBUFFER_H

#include <common.h>
#include "geometry.h"

class GBuffer
{
public:
	 GBuffer (void);
	 ~GBuffer (void);
	 bool Init (void);
	 void Render (Geometry &geometry);

	 gl::Texture colorbuffer;
	 gl::Texture normalbuffer;
	 gl::Texture specularbuffer;
	 gl::Texture depthbuffer;

	 gl::Texture msdepthtexture;
#ifdef DEBUG
	 GLuint numsamples;
#endif

	 GLuint GetWidth (void);
	 GLuint GetHeight (void);

	 bool GetWireframe (void);
	 void SetWireframe (bool w);

	 void SetAntialiasing (GLuint samples);
	 void SetProjMatrix (const glm::mat4 &projmat);
	 gl::Buffer fraglist;
	 gl::Buffer fragidx;

private:
	 GLuint width, height;
	 bool wireframe;

	 gl::Framebuffer framebuffer;
	 gl::Framebuffer multisamplefb;
	 gl::Framebuffer transparencyfb;

	 gl::Program program;
	 gl::Program transparencyprog;
	 gl::Program sraaprog;

	 gl::Program quadtessprog;
	 gl::Program triangletessprog;

	 gl::Buffer counter;
};


#endif /* !defined GBUFFER_H */
