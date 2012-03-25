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
#ifndef RENDERER_H
#define RENDERER_H

#include <common.h>
#include "geometry.h"
#include "gbuffer.h"
#include "finalpass.h"
#include "windowgrid.h"
#include "camera.h"
#include "shadowpass.h"
#include "opacitypass.h"
#include "filters.h"
#include "interface.h"
#include "composition.h"
#include "light.h"

class Renderer
{
public:
	 Renderer (void);
	 Renderer (const Renderer&) = delete;
	 ~Renderer (void);
	 Renderer &operator= (const Renderer&) = delete;
	 bool Init (void);
	 void Resize (int w, int h);
	 void Frame (void);
	 void OnKeyDown (int key);
	 void OnKeyUp (int key);
private:
	 Camera camera;

	 Geometry geometry;
	 GBuffer gbuffer;
	 ShadowPass shadowpass;
	 OpacityPass opacitypass;
	 FinalPass finalpass;
	 WindowGrid windowgrid;
	 Filters filters;
	 Composition composition;
	 gl::Program opacityprogram;

	 std::vector<Shadow> shadows;

	 std::vector<Light> lights;
	 cl::Memory lightmem;

	 cl::Context clctx;

	 Interface interface;

	 friend class Geometry;
	 friend class GBuffer;
	 friend class ShadowMap;
	 friend class ShadowPass;
	 friend class OpacityPass;
	 friend class FinalPass;
	 friend class Filters;
	 friend class Interface;
	 friend class Composition;
};

#endif /* !defined RENDERER_H */
