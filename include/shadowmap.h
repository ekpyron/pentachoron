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
#include "model/model.h"
#include "geometry.h"
#include "shadow.h"
#include "filters.h"

class Renderer;

/** Shadow map class.
 * This class handles the creation of shadow maps.
 */
class ShadowMap
{
public:
	/** Constructor.
	 * \param parent Specifies the parent Renderer class.
	 */
	ShadowMap (Renderer *parent);
	/** Destructor.
	 */
	 ~ShadowMap (void);
	 /** Initialization.
		* Initializes the shadow map class.
		* \returns Whether the initialization was successful.
		*/
	 bool Init (void);
	 /** Render the shadow map.
		* Renders a shadow map for the given geometry.
		* \param shadowid ID of the shadow that is rendered.
		* \param geometry Geometry to be rendered to fill the shadow map.
		* \param shadow Shadow parameters.
		*/
	 void Render (GLuint shadowid, Geometry &geometry, const Shadow &shadow);
	 /** Get the shadow map's height.
		* Obtains the height of the contained shadow map.
		*/
	 GLuint GetWidth (void) const;
   /** Get the shadow map's width.
		* Obtains the width of the contained shadow map.
		*/
	 GLuint GetHeight (void) const;
	 /** Query soft shadows.
		* Queries whether the shadow map should be blurred to create
		* soft shadows.
		* \return Whether soft shadows are active.
		*/
	 bool GetSoftShadows (void) const;
	 /** Set soft shadows.
		* Determines whether to blur the shadow map to create soft shadows.
		* \param s Whether to activate soft shadows.
		*/
	 void SetSoftShadows (bool s);
	 /** Shadow map.
		* A texture containing the actual shadow map.
		*/
	 gl::Texture shadowmap;
	 /** Temporary storage.
		* A texture used as temporary storage for blurring the shadow map.
		*/
	 gl::Texture tmpstore;
	 /** View matrix.
		* The view matrix to render the scene from the perspective
		* of the shadow caster.
		*/
	 glm::mat4 vmat;
	 /** Projection matrix (smart uniform).
		* The projection matrix to render the scene from the perspective
		* of the shadow caster. Stored in a smart uniform wrapper.
		*/
	 gl::SmartUniform<glm::mat4> projmat;
private:
	 /** Shadow map width.
		*/
	 GLuint width;
	 /** Shadow map height.
		*/
	 GLuint height;
	 /** OpenGL shader.
		* OpenGL shader program used to fill the shadow map.
		*/
	 gl::Program program;
	 gl::Program vblurprog;
	 gl::Program hblurprog;
	 gl::Sampler sampler;
	 gl::ProgramPipeline vblurpipeline;
	 gl::ProgramPipeline hblurpipeline;
	 /** Depthbuffer.
		* Renderbuffer used to store the depth buffer while
		* rendering the geometry.
		*/
	 gl::Renderbuffer depthbuffer;
	 /** Framebuffer.
		* Framebuffer object used for rendering to the shadow
		* map texture.
		*/
	 gl::Framebuffer framebuffer;

	 gl::Framebuffer vblurfb;
	 gl::Framebuffer hblurfb;
	 /** Soft shadows.
		* If true, the shadow map is blurred, if false, it is not.
		*/
	 bool soft_shadows;
	 /** Parent renderer.
		* The Renderer this class belongs to.
		*/
	 Renderer *renderer;
};


#endif /* !defined SHADOWMAP_H */
