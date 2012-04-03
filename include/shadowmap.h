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
#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include <common.h>
#include "scene/scene.h"
#include "geometry.h"
#include "shadow.h"
#include "filters.h"

class Renderer;

class ShadowMap
{
public:
	ShadowMap (Renderer *parent);
	 ~ShadowMap (void);
	 bool Init (void);
	 void Render (GLuint shadowid, Geometry &geometry, const Shadow &shadow);
	 GLuint GetWidth (void) const;
	 GLuint GetHeight (void) const;

	 bool GetSoftShadows (void) const;
	 void SetSoftShadows (bool s);

	 gl::Texture shadowmap;
	 cl::Memory shadowmapmem;
	 glm::mat4 vmat;
	 glm::mat4 projmat;
private:
	 GLuint width, height;
	 gl::Program program;
	 gl::Renderbuffer depthbuffer;
	 gl::Framebuffer framebuffer;

	 bool soft_shadows;

	 Blur blur;

	 Renderer *renderer;
	 friend class ShadowPass;
};


#endif /* !defined SHADOWMAP_H */
