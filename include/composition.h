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
#ifndef COMPOSITION_H
#define COMPOSITION_H

#include <common.h>
#include <gbuffer.h>
#include <filters.h>

class Renderer;

/** Composition class.
 * This class handles the composition of the gbuffer data into
 * a final color value and creates a glow map.
 */
class Composition
{
public:
	/** Constructor.
	 * \param parent Specifies the parent Renderer class.
	 */
	 Composition (Renderer *parent);
	 /** Destructor.
		*/
	 ~Composition (void);
	 /** Initialization.
		* Initializes the composition class.
		* \returns Whether the initialization was successful.
		*/
	 bool Init (void);
	 /** Per-frame subroutine.
		* Handles all per-frame computations required for the composition.
		* \param timefactor The fraction of seconds since the last frame.
		*/
	 void Frame (float timefactor);
	 /** Get glow size.
		* Obtains the size of the current glow effect.
		* \return size of current glow.
		*/
	 GLuint GetGlowSize (void);
	 /** Set glow size.
		* Sets the size of the current glow effect.
		*/
	 void SetGlowSize (GLuint size);
	 /** Get antialiasing.
		* Obtains the current level of antialiasing.
		* \return current antialiasing level.
		*/
	 GLuint GetAntialiasing (void);
	 /** Set antialiasing.
		* Sets the current level of antialiasing.
		*/
	 void SetAntialiasing (GLuint size);
	 /** Get shadow alpha.
		* Obtains the transparency factor of the shadows.
		* \returns shadow transparency factor.
		*/
	 float GetShadowAlpha (void);
	 /** Set shadow alpha.
		* Sets the transparency factor for the shadows.
		* \param alpha shadow alpha
		*/
	 void SetShadowAlpha (float alpha);
	 /** Get luminance threshold.
		* Obtains the luminance threshold for writing to the glow map.
		* \returns the luminance threshold
		*/
	 float GetLuminanceThreshold (void);
	 /** Set luminance threshold.
		* Sets the luminance threshold for writing to the glow map.
		* \param threshold the luminance threshold
		*/
	 void SetLuminanceThreshold (float threshold);
	 /** Get screen texture.
		* Returns a reference to the screen texture.
		* \returns a referene to the screen texture
		*/
	 const gl::Texture &GetScreen (void);
	 /** Get glow map.
		* Returns a reference to the (downsampled and blurred) glow map.
		* \returns a referene to the glow map
		*/
	 const gl::Texture &GetGlowMap (void);
	 /** Get edge map.
		* Returns a reference to the edge map.
		* \returns a referene to the edge map
		*/
	 const gl::Texture &GetEdgeMap (void);
private:
	 /** Screen texture.
		* A texture storing the combined, lit pixel value to be
		* displayed on screen.
		*/
	 gl::Texture screen;
	 struct
	 {
			/** Glow map.
			 * A texture storing the parts of the screen that is supposed to glow.
			 */
			gl::Texture map;
			/** Glow map (downsampled).
			 * A texture storing the parts of the screen that is supposed to glow
			 * downsampled to a lower resolution.
			 */
			gl::Texture downsampled;
			/** Glow map (full resolution; OpenCL memory object).
			 * OpenCL memory object referring to the glow map.
			 */
			cl::Memory mem;
			/** Glow map (downsampled; OpenCL memory object).
			 * OpenCL memory object referring to the downsampled glow
			 * map.
			 */
			cl::Memory mem_downsampled;
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
	 } glow;
	 /** Edge map.
		* A texture storing a map which highlights the edges in the
		* screen texture intended for anti-aliasing.
		*/
	 gl::Texture edgemap;
	 /** Shadow alpha.
		* This value specifies the degree of transparency of the shadows.
		*/
	 GLfloat shadow_alpha;
	 /** Luminance threshold.
		* Pixels with a luminance greater than this threshold will be
		* written to the glow map.
		*/
	 GLfloat luminance_threshold;
	 /** Antialiasing.
		* Stores the currently used level of antialiasing.
		*/
	 GLuint antialiasing;
	 /** Frei Chen filter.
		* A Frei Chen filter object used for edge detection.
		*/
	 FreiChen freichen;
	 /** OpenCL program.
		* The OpenCL program containing the composition kernel.
		*/
	 cl::Program program;
	 /** OpenCL kernel
		* The OpenCL kernel that does the actual computations.
		*/
	 cl::Kernel composition;
	 /** Screen texture (OpenCL memory object).
		* OpenCL memory object referring to the screen texture.
		*/
	 cl::Memory screenmem;
	 /** Edge map (OpenCL memory object).
		* OpenCL memory object referring to the edge map.
		*/
	 cl::Memory edgemem;
	 /** Parent renderer.
		* The Renderer this class belongs to.
		*/
	 Renderer *renderer;
};

#endif /* !defined COMPOSITION_H */
