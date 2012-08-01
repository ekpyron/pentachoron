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
	 /** Get screen limit.
		* Obtains the screen limit, i.e. the maximum value that's
		* written to the screen texture.
		* \returns the screen limit
		*/
	 float GetScreenLimit (void);
	 /** Set screen limit.
		* Sets the screen limit, i.e. the maximum value that's
		* written to the screen texture.
		* \param limit the screen limit
		*/
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

	 /** Generate perez coefficients.
		* Generates the 15 perez coefficients from the turbidity value.
		*/
	 void GeneratePerezCoefficients (void);
	 /** Obtain perez coefficient.
		* Obtains the specified perez coefficient for luminacity.
		* \param idx Which perez coefficient to return (range 1..5).
		* \returns the perez coefficient.
		*/
	 float GetPerezY (int idx);
	 /** Set perez coefficient.
		* Sets the specified perez coefficient for luminacity.
		* \param idx Which perez coefficient to specify (range 1..5).
		* \param val the perez coefficient
		*/
	 void SetPerezY (int idx, float val);
	 /** Obtain perez coefficient.
		* Obtains the specified perez coefficient x chromacity.
		* \param idx Which perez coefficient to return (range 1..5).
		* \returns the perez coefficient.
		*/
	 float GetPerezx (int idx);
	 /** Set perez coefficient.
		* Sets the specified perez coefficient for x chromacity.
		* \param idx Which perez coefficient to specify (range 1..5).
		* \param val the perez coefficient
		*/
	 void SetPerezx (int idx, float val);
	 /** Obtain perez coefficient.
		* Obtains the specified perez coefficient y chromacity.
		* \param idx Which perez coefficient to return (range 1..5).
		* \returns the perez coefficient.
		*/
	 float GetPerezy (int idx);
	 /** Set perez coefficient.
		* Sets the specified perez coefficient for y chromacity.
		* \param idx Which perez coefficient to specify (range 1..5).
		* \param val the perez coefficient
		*/
	 void SetPerezy (int idx, float val);
	 /** Obtain turbidity.
		* Obtain the turbidity value.
		* \returns the turbidity value.
		*/
	 float GetTurbidity (void);
	 /** Set turbidity.
		* Sets the turbidity value.
		* \param t turbidity value
		*/
	 void SetTurbidity (float t);
	 /** Obtain latitude.
		* Obtain the latitude, used for calculating the position of the sun.
		* \returns the current latitude
		*/
	 float GetLatitude (void);
	 /** Set latitude.
		* Sets the latitude used for calculating the position of the sun.
		* \param l latitude
		*/
	 void SetLatitude (float l);
   /** Obtain date.
		* Obtains the current date (in days since the first of January).
		* Used to calculate the position of the sun.
		* \return current date
		*/
	 int GetDate (void);
   /** Set date.
		* Sets the current date (in days since the first of January).
		* Used to calculate the position of the sun.
		* \param d date
		*/
	 void SetDate (int d);
	 /** Obtain time of day.
		* Obtains the time of day as floating point value in hours.
		* Used to calculate the position of the sun.
		* \returns the time of day.
		*/
	 float GetTimeOfDay (void);
	 /** Set time of day.
		* Sets the time of day.
		* Used to calculate the position of the sun.
		* \param t the time of day as floating point value in hours.
		*/
	 void SetTimeOfDay (float t);
	 /** Calculate and setup zenith Yxy.
		* Calculates the zenith Yxy color from the current position
		* of the sun and sets the shader uniforms accordingly.
		*/
	 void SetupSkyZenithYxy (void);
	 /** Calculate and setup the position of the sun.
		* Calculates the current position of the sun
		* and sets the shader uniforms accordingly.
		*/
	 void SetupSunPosition (void);
	 void SetSkyLuminosity (GLfloat l);
	 GLfloat GetSkyLuminosity (void);
	 /** Check, whether rendering is tile-based.
		* Returns true, if tile-based light culling is currently used,
		* false, if not.
		*/
	 bool GetTileBased (void);
	 /** Enable or disable tile-based light culling.
		* Enables or disables tile-based light culling. Tile-based
		* light culling results in a major speedup in most situations
		* with around 16 lights or more.
		* \param tb state of tile-based light culling
		*/
	 void SetTileBased (bool tb);
	 /** Get glow.
		* Returns a reference to the internal glow effect class.
		* \returns a referene to the Glow class
		*/
	 Glow &GetGlow (void);
	 /** dummy texture.
		* Dummy texture. Necessary, because a framebuffer has to have
		* at least one attachment.
		*/
	 gl::Texture dummy;

	 /** Sun parameters.
		* Uniform parameters of the sun.
		*/
	 struct {
			/** Sun inclination.
			 * Inclination of the sun (i.e. the angle between sun and zenith)
			 * in radians.
			 */
			gl::SmartUniform<GLfloat> theta;
			/** Cosine of sun inclination.
			 * Cosine of the inclination of the sun (i.e. the angle between
			 * sun and zenith).
			 */
			gl::SmartUniform<GLfloat> cos_theta;
			/** Direction of the sun.
			 * Normalized direction of the sun.
			 */
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
			gl::SmartUniform<GLfloat> luminosity;
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
