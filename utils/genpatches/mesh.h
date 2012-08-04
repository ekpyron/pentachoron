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
#ifndef MESH_H
#define MESH_H

#include "common.h"
#include "face.h"
#include "edge.h"
#include "patch.h"

class Mesh
{
public:
	 Mesh (void);
	 ~Mesh (void);
	 void Import (const std::string &filename, unsigned int meshid);
	 void Export (const std::string &filename);
	 unsigned int GetNumFaces (void) const;
	 void GeneratePatches (void);

	 unsigned int GetNumQuadPatches (void) const;
	 const QuadPatch &GetQuadPatch (unsigned int q) const;

	 unsigned int GetNumTrianglePatches (void) const;
	 const TrianglePatch &GetTrianglePatch (unsigned int q) const;

	 const Face &GetFace (unsigned int i) const;

	 const std::set<Edge> &GetEdges (void) const;
	 
	 bool IsBorder (const glm::vec3 &v) const;
	 bool IsBorder (const Edge &edge) const;

private:
	 TrianglePatch Triangle2Patch (unsigned int faceid) const;
	 QuadPatch Quad2Patch (unsigned int faceid) const;

	 glm::vec3 GetEdgePoint (const glm::vec3 &v, const Edge &edge,
													 unsigned int faceid,
													 const glm::vec3 &p,
													 const glm::vec3 &p2) const;
	 glm::vec3 GetFacePoint (const glm::vec3 &v, const Edge &edge,
													 unsigned int faceid, const glm::vec3 &p,
													 const glm::vec3 &e1, const glm::vec3 &e2) const;

	 unsigned int GetSecondFaceOnEdge (const Edge &edge,
																		unsigned int faceid) const;
	 Edge GetSecondEdgeOnFace (const glm::vec3 &v,
														 unsigned int faceid,
														 const Edge &edge) const;

	 void AddFaceToVertex (const glm::vec3 &v, unsigned int face);
	 void AddEdgeToVertex (const glm::vec3 &v, const Edge &edge);
	 void AddEdgeToFace (unsigned int face, const Edge &edge);
	 void AddFaceToEdge (const Edge &edge, unsigned int face);

	 const std::set<unsigned int> &GetFacesFromVertex (const glm::vec3 &v) const;
	 const std::set<Edge> &GetEdgesFromVertex (const glm::vec3 &v) const;
	 const std::set<unsigned int> &GetFacesFromEdge (const Edge &e) const;
	 const std::set<Edge> &GetEdgesFromFace (unsigned int faceid) const;
	 unsigned int GetValence (const glm::vec3 &v) const;

	 std::vector<TrianglePatch> trianglepatches;
	 std::vector<QuadPatch> quadpatches;

	 std::vector<Face> faces;
	 std::set<Edge> edges;
	 std::map<glm::vec3, std::set<unsigned int>> vertex2faces;
	 std::map<glm::vec3, std::set<Edge>> vertex2edges;
	 std::vector<std::set<Edge>> face2edges;
	 std::map<Edge, std::set<unsigned int>> edge2faces;
};

#endif /* !defined MESH_H */
