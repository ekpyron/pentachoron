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

/** Glow class.
 * This class handles the glow effect.
 */
class Glow
{
public:
	/** Constructor.
	 */
	Glow (void);
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
	 /** Get glow limit.
		* Returns the maximum pixel value that's written to the glow map.
		* \returns the glow limit
		*/
	 GLfloat GetLimit (void);
	 /** Set glow limit.
		* Sets the maximum pixel value that's written to the glow map.
		* \param limit the glow limit
		*/
	 void SetLimit (GLfloat limit);
	 /** Get glow exponent.
		* Returns the glow exponent.
		* \returns the glow exponent
		*/
	 GLfloat GetExponent (void);
	 /** Set glow exponent.
		* Sets the glow exponent.
		* \param exp the glow exponent
		*/
	 void SetExponent (GLfloat exp);
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
	 struct
	 {
			gl::Framebuffer fb;
			gl::Program prog;
			gl::ProgramPipeline pipeline;
	 } vblur;
	 struct
	 {
			gl::Framebuffer fb;
			gl::Program prog;
			gl::ProgramPipeline pipeline;
	 } hblur;
	 /** The glow limit.
		* The maximum pixel value to be written to the glow map.
		*/
	 GLfloat limit;
	 /** The glow exponent.
		* The exponent to adjust the pixel value written to the glow map.
		*/
	 GLfloat exponent;
	 /** Blend framebuffer.
		* The framebuffer object for blending the glow map into the screen
		* texture.
		*/
	 gl::Sampler sampler;
	 gl::Sampler sampler2;

	 gl::Buffer buffer;
	 gl::Texture buffertex;

	 gl::Texture map, map2;

	 gl::Framebuffer blendfb;
	 gl::Program blendprog;
	 gl::ProgramPipeline blendpipeline;

	 GLuint size;

	 gl::Texture *glowmap;
};

#endif /* !defined GLOW_H */
