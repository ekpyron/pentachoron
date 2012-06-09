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
#ifndef LIGHT_H
#define LIGHT_H

#include <common.h>

typedef struct _Light
{
	 glm::vec4 position;
	 glm::vec4 color;
	 glm::vec4 direction;
	 struct
	 {
			float cosine;
			float exponent;
			float angle;
			float tangent;
			float penumbra_angle;
			float penumbra_cosine;
			float padding[2];
	 } spot;
	 struct
	 {
			glm::vec3 color;
			float padding;
	 } specular;
	 struct
	 {
			glm::vec4 l;
			glm::vec4 r;
			glm::vec4 b;
			glm::vec4 t;
			glm::vec4 n;
			glm::vec4 f;
	 } frustum;
	 /*
		* attenuation.x: constant attenuation
		* attenuation.y: linear attenuation
		* attenuation.z: quadratic attenuation
		* attenuation.w: max distance
		*/
	 glm::vec4 attenuation;

	 void CalculateFrustum (void);
} Light;


#endif /* !defined LIGHT_H */
