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
#include "shadowmap.h"
#include "interface.h"
#include "composition.h"
#include "light.h"
#include "parameter.h"
#include "culling.h"

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

	 GLuint GetAntialiasing (void);
	 void SetAntialiasing (GLuint value);

	 Light &GetLight (GLuint light);
	 GLuint GetNumLights (void);
	 void RemoveLight (GLuint light);
	 void AddLight (const Light &light);
	 void UpdateLight (GLuint light);
	 gl::Buffer &GetLightBuffer (void);

	 GLuint GetNumParameters (void);
	 Parameter &GetParameters (GLuint params);
	 void UpdateParameters (GLuint params);
	 const cl::Memory &GetParameterMem (void);

#ifdef DEBUG
	 unsigned long memory;
#endif

/* TODO: make as much as possible private */
	 Geometry geometry;
	 GBuffer gbuffer;
	 ShadowMap shadowmap;
	 FinalPass finalpass;
	 WindowGrid windowgrid;
	 Composition composition;
	 Culling culling;
	 Camera camera;

	 std::vector<Shadow> shadows;
	 cl::CommandQueue queue;
	 cl::Context clctx;

private:
	 GLuint antialiasing;

	 gl::Program opacityprogram;

	 std::vector<Parameter> parameters;
	 cl::Memory parametermem;

	 std::vector<Light> lights;
	 gl::Buffer lightbuffer;

	 Interface interface;
};

extern std::unique_ptr<Renderer> r;

#endif /* !defined RENDERER_H */
