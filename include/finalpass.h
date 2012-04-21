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
#ifndef FINALPASS_H
#define FINALPASS_H

#include <common.h>

class Renderer;

class FinalPass
{
public:
	 FinalPass (Renderer *parent);
	 ~FinalPass (void);
	 bool Init (void);
	 void Render (void);

	 void SetRenderMode (GLuint mode);
	 GLuint GetRenderMode (void);
	 void SetImageKey (float key);
	 float GetImageKey (void);
	 void SetWhiteThreshold (float threshold);
	 float GetWhiteThreshold (void);
	 void SetTonemappingSigma (float sigma);
	 float GetTonemappingSigma (void);
	 void SetTonemappingExponent (float n);
	 float GetTonemappingExponent (void);
	 void SetRGBWorkingSpace (GLuint ws);
	 GLuint GetRGBWorkingSpace (void);
	 void SetTonemappingMode (GLuint mode);
	 GLuint GetTonemappingMode (void);
private:
	 std::vector<gl::Program> fprograms;
	 std::vector<gl::ProgramPipeline> pipelines;
	 gl::Sampler sampler;
	 GLuint rendermode;
	 struct
	 {
			float image_key;
			float white_threshold;
			float sigma;
			float n;
			GLuint rgb_working_space;
			GLuint mode;
	 } tonemapping;
	 Renderer *renderer;
};

#endif /* !defined FINALPASS_H */
