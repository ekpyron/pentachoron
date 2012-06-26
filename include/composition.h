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

	 void SetLightMem (cl::Memory &m);

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
	 /** Glow map (OpenCL memory object).
		* OpenCL memory object referring to the glow map.
		*/
	 cl::Memory glowmem;

	 SkyParams sky;

	 class Info
	 {
	 public:
			Info (void);
			~Info (void);
			bool Init (void);

			void SetProjInfo (const glm::vec4 &i);
			const glm::vec4 &GetProjInfo (void);
			void SetInverseViewMatrix (const glm::mat4 &m);
			const glm::mat4 &GetInverseViewMatrix (void);
			void SetShadowMatrix (const glm::mat4 &m);
			const glm::mat4 &GetShadowMatrix (void);
			void SetEye (const glm::vec4 &e);
			const glm::vec4 &GetEye (void);
			void SetCenter (const glm::vec4 &c);
			const glm::vec4 &GetCenter (void);
			void SetGlowSize (GLuint s);
			GLuint GetGlowSize (void);
			void SetGlowExponent (GLfloat e);
			GLfloat GetGlowExponent (void);
			void SetGlowThreshold (GLfloat t);
			GLfloat GetGlowThreshold (void);
			void SetGlowLimit (GLfloat l);
			GLfloat GetGlowLimit (void);
			void SetSunDirection (const glm::vec4 &d);
			const glm::vec4 &GetSunDirection (void);
			void SetSunTheta (GLfloat theta);
			GLfloat GetSunTheta (void);
			GLfloat GetCosSunTheta (void);
			void SetSkyTurbidity (GLfloat T);
			GLfloat GetSkyTurbidity (void);
			void SetSkyPerezY (const GLfloat *p);
			const GLfloat *GetSkyPerezY (void);
			void SetSkyPerezx (const GLfloat *p);
			const GLfloat *GetSkyPerezx (void);
			void SetSkyPerezy (const GLfloat *p);
			const GLfloat *GetSkyPerezy (void);
			void SetSkyZenithYxy (const glm::vec4 &Yxy);
			const glm::vec4 &GetSkyZenithYxy (void);
			void SetShadowAlpha (GLfloat a);
			GLfloat GetShadowAlpha (void);
			void SetMode (GLuint m);
			GLuint GetMode (void);
			void SetNumLights (GLuint n);
			GLuint GetNumLights (void);
			void SetScreenLimit (GLfloat f);
			GLfloat GetScreenLimit (void);

			const cl::Memory &GetMem (void);
	 private:
			cl::Memory mem;
			typedef struct InfoData {
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
						glm::vec4 direction;
						float theta;
						float cos_theta;
						float padding[2];
				 } sun;
				 struct
				 {
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
			} InfoData;
			InfoData data;
	 };

	 Info info;
};

#endif /* !defined COMPOSITION_H */
