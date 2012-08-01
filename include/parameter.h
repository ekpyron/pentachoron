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
#ifndef PARAMETER_H
#define PARAMETER_H

#include <common.h>

typedef struct _Parameter
{
	 struct
	 {
			unsigned int model;
			union
			{
				 float smoothness;
				 float shininess;
				 float param1;
			};
			union
			{
				 float gaussfactor;
				 float param2;
			};
			struct
			{
				 float n;
				 float k;
			} fresnel;
	 } specular;
	 struct
	 {
			float factor;
			struct
			{
				 float n;
				 float k;
			} fresnel;
	 } reflection;
} Parameter;

static_assert (sizeof (Parameter) % sizeof (glm::vec4) == 0,
							 "Parameter is not aligned correctly.");


#endif /* !defined PARAMETER_H */
