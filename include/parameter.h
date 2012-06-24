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
#ifndef PARAMETER_H
#define PARAMETER_H

#include <common.h>

typedef struct _Parameter
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
			float fresnel;
			float param2;
	 };
	 float reflect;
} Parameter;


#endif /* !defined PARAMETER_H */
