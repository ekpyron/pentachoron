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
#include <glow.h>

/** Composition class.
 * This class handles the composition of the gbuffer data into
 * a final color value and creates a glow map.
 */
class Composition
{
public:
	/** Constructor.
	 */
	 Composition (void);
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
	 float GetScreenLimit (void);
	 void SetScreenLimit (float limit);
	 /** Set mode.
		* Sets the composition mode.
		* \param mode Composition mode to use.
		*/
	 void SetMode (GLuint mode);
	 /** Get mode.
		* Obtains the composition mode.
		* \returns the composition mode
		*/
	 GLuint GetMode (void);

	 /** Get screen texture.
		* Returns a reference to the screen texture.
		* \returns a referene to the screen texture
		*/
	 const gl::Texture &GetScreen (void);

	 void GeneratePerezCoefficients (void);
	 float GetPerezY (int idx);
	 void SetPerezY (int idx, float val);
	 float GetPerezx (int idx);
	 void SetPerezx (int idx, float val);
	 float GetPerezy (int idx);
	 void SetPerezy (int idx, float val);
	 float GetTurbidity (void);
	 void SetTurbidity (float t);
	 float GetLatitude (void);
	 void SetLatitude (float l);
	 int GetDate (void);
	 void SetDate (int m);
	 float GetTimeOfDay (void);
	 void SetTimeOfDay (float t);

	 void SetupSkyZenithYxy (void);
	 void SetupSunPosition (void);

	 bool GetTileBased (void);
	 void SetTileBased (bool tb);

	 /** Get glow.
		* Returns a reference to the internal glow effect class.
		* \returns a referene to the Glow class
		*/
	 Glow &GetGlow (void);

	 gl::Texture dummy;

	 struct {
			gl::SmartUniform<GLfloat> theta;
			gl::SmartUniform<GLfloat> cos_theta;
			gl::SmartUniform<glm::vec3> direction;
	 } sun;

private:
	 /** Screen texture.
		* A texture storing the combined, lit pixel value to be
		* displayed on screen.
		*/
	 gl::Texture screen;
	 /** Glow effect.
		* This class handles the glow effect.
		*/
	 Glow glow;
	 /** Glow map.
		* A texture storing the parts of the screen that is supposed to glow.
		*/
	 gl::Texture glowmap;
	 /** Shadow alpha.
		* This value specifies the degree of transparency of the shadows.
		*/
	 gl::SmartUniform<GLfloat> shadow_alpha;
	 /** Luminance threshold.
		* Pixels with a luminance greater than this threshold will be
		* written to the glow map.
		*/
	 gl::SmartUniform<bool> tile_based;
	 gl::SmartUniform<GLfloat> luminance_threshold;
	 gl::SmartUniform<GLfloat> screenlimit;
	 gl::SmartUniform<glm::mat4> shadowmat;
	 gl::SmartUniform<glm::vec3> eye;
	 struct {
			float turbidity;
			float latitude;
			int date;
			float time;
			
			gl::SmartUniform<std::array<GLfloat, 5>> perezY;
			gl::SmartUniform<std::array<GLfloat, 5>> perezx;
			gl::SmartUniform<std::array<GLfloat, 5>> perezy;
			gl::SmartUniform<glm::vec3> zenithYxy;
	 } sky;

	 gl::Framebuffer framebuffer;
	 gl::ProgramPipeline pipeline;
	 gl::Program fprogram;

	 gl::Framebuffer clearfb;
	 gl::Framebuffer lightcullfb;
	 gl::ProgramPipeline lightcullpipeline;
	 gl::Program lightcullprog;
	 gl::Texture lighttex;
	 gl::Buffer numlights;

	 gl::Texture lightbuffertex;

	 gl::Framebuffer minmaxdepthfb;
	 gl::ProgramPipeline minmaxdepthpipeline;
	 gl::Program minmaxdepthprog;
	 gl::Texture mindepthtex;
	 gl::Texture maxdepthtex;
};

#endif /* !defined COMPOSITION_H */
