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
#ifndef GLOW_H
#define GLOW_H

#include <common.h>
#include <gbuffer.h>
#include <filters.h>

/** Glow class.
 * This class handles the glow effect.
 */
class Glow
{
public:
	/** Constructor.
	 * \param parent Specifies the parent Renderer class.
	 */
	Glow (Renderer *parent);
	 /** Destructor.
		*/
	 ~Glow (void);
	 /** Initialization.
		* Initializes the glow class.
		* \param screenmap Screenmap to blend the glow map into.
		* \param glowmap Glowmap to use.
		* \param mipmap_level Specifies which mipmap level of the glow map to use.
		* \returns Whether the initialization was successful.
		*/
	 bool Init (gl::Texture &screenmap, gl::Texture &glowmap,
							GLuint mipmap_level);
	 /** Apply glow effect.
		* Applies the glow effect.
		*/
	 void Apply (void);
	 /** Get glow size.
		* Obtains the size of the glow effect.
		* \return size of glow.
		*/
	 GLuint GetSize (void);
	 /** Set glow size.
		* Sets the size of the glow effect.
		*/
	 void SetSize (GLuint size);
	 /** Get glow map.
		* Returns a reference to the (downsampled and blurred) glow map.
		* \returns a referene to the glow map
		*/
	 const gl::Texture &GetMap (void);
private:
	 /** Glow map width.
		* Width of the glow map.
		*/
	 GLuint width;
	 /** Glow map height.
		* Height of the glow map.
		*/
	 GLuint height;
	 /** Glow map (downsampled).
		* A texture storing the parts of the screen that is supposed to glow
		* downsampled to a lower resolution.
		*/
	 gl::Texture map;
	 /** Glow map (OpenCL memory object).
		* OpenCL memory object referring to the downsampled glow
		* map.
		*/
	 cl::Memory mem;
	 /** Glow blur.
		* A Blur object for blurring the downsampled glow map.
		*/
	 Blur blur;
	 /** Source framebuffer.
		* The source framebuffer object for copying and downsampling
		* the glow map.
		*/
	 gl::Framebuffer source;
	 /** Destination framebuffer.
		* The destination framebuffer object for copying and downsampling
		* the glow map.
		*/
	 gl::Framebuffer destination;
	 /** Blend framebuffer.
		* The framebuffer object for blending the glow map into the screen
		* texture.
		*/
	 gl::Framebuffer framebuffer;
	 gl::Program fprogram;
	 gl::ProgramPipeline pipeline;
	 gl::Sampler sampler;

	 Renderer *renderer;
};

#endif /* !defined GLOW_H */
