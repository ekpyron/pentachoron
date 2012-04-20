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

	 /** Screen texture.
		* A texture storing the combined, lit pixel value to be
		* displayed on screen.
		*/
	 gl::Texture screen;
	 /** Glow map.
		* A texture storing the parts of the screen that is supposed to glow.
		*/
	 gl::Texture glow;
	 /** Luminance threshold.
		* Pixels with a luminance higher than this threshold will be written
		* to the glow map.
		*/
	 GLfloat luminance_threshold;
	 /** Shadow alpha.
		* This value specifies the degree of transparency of the shadows.
		*/
	 GLfloat shadow_alpha;
private:
	 /** Glow blur.
		* A Blur object for blurring the downsampled glow map.
		*/
	 Blur blur;
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
	 /** Glow map (full resolution; OpenCL memory object).
		* OpenCL memory object referring to the glow map in
		* full resolution (mipmap level 0).
		*/
	 cl::Memory glowmem_full;
	 /** Glow map (downsampled; OpenCL memory object).
		* OpenCL memory object referring to the downsampled glow
		* map (a specific mipmap level > 0).
		*/
	 cl::Memory glowmem_downsampled;
	 /** Parent renderer.
		* The Renderer this class belongs to.
		*/
	 Renderer *renderer;
	 /** Interface is a friend.
		*/
	 friend class Interface;
};

#endif /* !defined COMPOSITION_H */
