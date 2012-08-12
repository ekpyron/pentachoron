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
#ifndef RENDERER_H
#define RENDERER_H

#include <common.h>
#include "geometry.h"
#include "gbuffer.h"
#include "postprocess.h"
#include "windowgrid.h"
#include "camera.h"
#include "shadowmap.h"
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
	 gl::Texture &GetParameterTexture (void);
	 const std::string &GetParameterName (GLuint param);

#ifdef DEBUG
	 unsigned long memory;
#endif

/* TODO: make as much as possible private */
	 Geometry geometry;
	 GBuffer gbuffer;
	 ShadowMap shadowmap;
	 PostProcess postprocess;
	 WindowGrid windowgrid;
	 Composition composition;
	 Culling culling;
	 Camera camera;

	 std::vector<Shadow> shadows;

private:
	 GLuint antialiasing;

	 gl::Program opacityprogram;

	 std::vector<Parameter> parameters;
	 std::vector<std::string> parameter_names;
	 gl::Buffer parameterbuffer;
	 gl::Texture parametertexture;

	 std::vector<Light> lights;
	 gl::Buffer lightbuffer;
};

extern std::unique_ptr<Renderer> r;

#endif /* !defined RENDERER_H */
