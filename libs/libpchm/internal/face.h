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
#ifndef FACE_H
#define FACE_H

#include "common.h"

class Face
{
public:
	 Face (void);
	 Face (const std::vector<glm::vec3> &v);
	 Face (const Face &f);
	 Face (Face &&f);
	 ~Face (void);
	 Face &operator= (const Face &f);
	 Face &operator= (Face &&f);
	 void Define (const std::vector<glm::vec3> &v);
	 void AddTexCoords (const std::vector<glm::vec3> &t);
	 unsigned int GetNumVertices (void) const;
	 const glm::vec3 &GetVertex (unsigned int i) const;
	 unsigned int GetNumTexCoords (void) const;
	 const glm::vec3 &GetTexCoord (unsigned int i, unsigned int t) const;
	 glm::vec3 GetCentroid (void) const;
	 const glm::vec3 &GetFourth (const glm::vec3 &v1, const glm::vec3 &v2,
															 const glm::vec3 &v3) const;
	 bool Contains (const glm::vec3 &v) const;
private:
	 std::vector<glm::vec3> vertices;
	 std::vector<std::vector<glm::vec3>> texcoords;
};

#endif /* !defined FACE_H */
