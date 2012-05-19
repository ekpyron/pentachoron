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
#ifndef SHADOWPASS_H
#define SHADOWPASS_H

#include <common.h>
#include "shadowmap.h"
#include "shadow.h"
#include "geometry.h"

class Renderer;

class ShadowPass
{
public:
	 ShadowPass (Renderer *parent);
	 ~ShadowPass (void);
	 bool Init (void);
	 void Render (GLuint shadowid, Geometry &geometry,
								const Shadow &shadow);

	 const gl::Texture &GetShadowMask (void);
	 const cl::Memory &GetShadowMaskMem (void);
	 void SetSoftShadows (bool softshadow);
	 bool GetSoftShadows (void);
private:
	 gl::Sampler sampler;
	 gl::Texture shadowmask;
	 cl::Memory shadowmaskmem;
	 gl::Framebuffer framebuffer;
	 gl::Program fprogram;
	 gl::ProgramPipeline pipeline;
	 ShadowMap shadowmap;
	 Renderer *renderer;
};

#endif /* !defined SHADOWPASS_H */
