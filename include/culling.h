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
#ifndef CULLING_H
#define CULLING_H

#include <common.h>

class Culling
{
public:
	 Culling (void);
	 ~Culling (void);
	 bool IsVisible (const glm::vec3 &center, float radius) const;
	 void SetProjMatrix (const glm::mat4 &mat);
	 void SetModelViewMatrix (const glm::mat4 &mat);
	 void Frame (void);
private:
	 glm::mat4 projmat, mvmat;
};

#endif /* !defined CULLING_H */
