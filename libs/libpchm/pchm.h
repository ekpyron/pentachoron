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
#ifndef PCHM_H
#define PCHM_H

#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <string>

namespace pchm {

class model
{
public:
	 model (void);
	 model (const model &m);
	 model (model &&m);
	 ~model (void);
	 model &operator= (const model &m);
	 model &operator= (model &&m);

	 bool Patches (void) const;

	 void GeneratePatches (void);

	 bool Load (const std::string &filename);
	 bool Load (std::istream &in);
	 bool Save (std::ostream &out) const;
	 bool Save (const std::string &filename) const;

	 void Define (unsigned int vertices, unsigned int triangles,
								unsigned int quads);

	 void SetTriangles (const unsigned int *i);
	 void SetQuads (const unsigned int *i);

	 void SetPositions (const glm::vec3 *p);
	 void SetNormals (const glm::vec3 *n);
	 void SetTangents (const glm::vec3 *t);
	 void SetTexcoords (unsigned int id, const glm::vec2 *t);
	 void AddTexcoords (const glm::vec2 *t);

	 unsigned int GetNumVertices (void) const;

	 const glm::vec3 *GetPositions (void) const;
	 const glm::vec3 *GetNormals (void) const;
	 const glm::vec3 *GetTangents (void) const;
	 const glm::vec2 *GetTexcoords (unsigned int id) const;
	 unsigned int GetNumTexcoords (void) const;

	 unsigned int GetNumTriangles (void) const;
	 unsigned int GetNumQuads (void) const;
	 const unsigned int *GetTriangleIndices (void) const;
	 const unsigned int *GetQuadIndices (void) const;
	 
private:
	 std::vector<glm::vec3> positions;
	 std::vector<glm::vec3> normals;
	 std::vector<glm::vec3> tangents;
	 std::vector<std::vector<glm::vec2> > texcoords;

	 std::vector<unsigned int> triangleindices;
	 std::vector<unsigned int> quadindices;

	 bool patches;
};

} /* namespace pchm */

#endif /* !defined PCHM_H */
