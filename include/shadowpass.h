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
#include "shadow.h"
#include "shadowmap.h"
#include "filters.h"
#include "gbuffer.h"

class Renderer;

class ShadowPass
{
public:
	 ShadowPass (Renderer *parent);
	 ~ShadowPass (void);
	 bool Init (void);
	 void FrameInit (void);
	 void Render (const Shadow &shadow);
	 void FrameFinish (void);

	 void SetSoftShadows (bool value);
	 bool GetSoftShadows (void);

	 gl::Texture shadowmask;
	 ShadowMap shadowmap;
	 cl::Memory shadowmem;
private:
	 cl::CommandQueue queue;
	 cl::Program program;
	 cl::Kernel genshadow;
	 cl::Memory shadowmapmem;

	 bool soft_shadows;
	 Blur blur;
	 Renderer *renderer;
};

#endif /* !defined SHADOWPASS_H */
