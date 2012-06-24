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

	 typedef struct SkyParams
	 {
			float turbidity;
			float latitude;
			int date;
			float time;
			float perezY[5];
			float perezx[5];
			float perezy[5];
	 } SkyParams;

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
	 glm::vec3 GetSunDirection (float &theta, float &cos_theta);
	 glm::vec3 GetSunDirection (void);

	 /** Get glow.
		* Returns a reference to the internal glow effect class.
		* \returns a referene to the Glow class
		*/
	 Glow &GetGlow (void);
private:
	 /** Screen texture.
		* A texture storing the combined, lit pixel value to be
		* displayed on screen.
		*/
	 gl::Texture screen;
	 GLfloat screenlimit;
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
	 GLfloat shadow_alpha;
	 /** Luminance threshold.
		* Pixels with a luminance greater than this threshold will be
		* written to the glow map.
		*/
	 GLfloat luminance_threshold;

	 SkyParams sky;

	 typedef struct Info
	 {
			glm::vec4 projinfo;
			glm::mat4 vmatinv;
			glm::mat4 shadowmat;
			glm::vec4 eye;
			glm::vec4 center;
			struct
			{
				 GLuint size;
				 GLfloat exponent;
				 GLfloat threshold;
				 GLfloat glowlimit;
			} glow;
			struct
			{
				 struct
				 {
						glm::vec4 direction;
						float theta;
						float cos_theta;
						float padding[2];
				 } sun;
				 float turbidity;
				 float perezY[5];
				 float perezx[5];
				 float perezy[5];
				 glm::vec4 zenithYxy;
			} sky;
			GLfloat shadow_alpha;
			GLuint mode;
			cl_uint num_lights;
			GLfloat screenlimit;
	 } Info;

	 Info info;

	 gl::Framebuffer framebuffer;
	 gl::ProgramPipeline pipeline;
	 gl::Program fprogram;

	 gl::Framebuffer lightfb;
	 gl::Framebuffer clearfb;
	 gl::ProgramPipeline lightpipeline;
	 gl::Program lightprog;
	 gl::Texture lighttex;
	 gl::Texture numlighttex;
	 gl::Texture mindepthtex;
	 gl::Texture maxdepthtex;
	 gl::Renderbuffer dummy;

	 /** Parent renderer.
		* The Renderer this class belongs to.
		*/
	 Renderer *renderer;
};

#endif /* !defined COMPOSITION_H */
