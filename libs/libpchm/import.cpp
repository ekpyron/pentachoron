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
#include "common.h"
#include "mesh.h"
#include <cstring>
#include <stdexcept>

void Mesh::Import (const pchm::model &m)
{
	const glm::vec3 *positions = m.GetPositions ();

	for (unsigned int i = 0; i < m.GetNumTriangles (); i++)
	{
		std::vector<glm::vec3> v;
		const unsigned int *idx = &m.GetTriangleIndices ()[i * 3];
		for (auto c = 0; c < 3; c++)
			 v.push_back (positions[idx[c]]);
		faces.push_back (Face (v));

		for (auto tid = 0; tid < m.GetNumTexcoords (); tid++)
		{
			const glm::vec2 *texcoords = m.GetTexcoords (tid);
			std::vector<glm::vec3> t;
			for (auto c = 0; c < 3; c++)
				 t.push_back (glm::vec3 (texcoords[idx[c]].x,
																 texcoords[idx[c]].y, 0.0f));
			faces.back ().AddTexCoords (t);
		}
	}
	for (unsigned int i = 0; i < m.GetNumQuads (); i++)
	{
		std::vector<glm::vec3> v;
		const unsigned int *idx = &m.GetQuadIndices ()[i * 4];
		for (auto c = 0; c < 4; c++)
			 v.push_back (positions[idx[c]]);
		faces.push_back (Face (v));

		for (auto tid = 0; tid < m.GetNumTexcoords (); tid++)
		{
			const glm::vec2 *texcoords = m.GetTexcoords (tid);
			std::vector<glm::vec3> t;
			for (auto c = 0; c < 4; c++)
				 t.push_back (glm::vec3 (texcoords[idx[c]].x,
																 texcoords[idx[c]].y, 0.0f));
			faces.back ().AddTexCoords (t);
		}
	}

	face2edges.resize (faces.size ());
	for (auto i = 0; i < faces.size (); i++)
	{
		for (auto c = 0; c < faces[i].GetNumVertices (); c++)
		{
			const glm::vec3 &v = faces[i].GetVertex (c);
			Edge edge (v, faces[i].GetVertex ((c + 1) % faces[i].GetNumVertices ()));
			AddFaceToVertex (v, i);
			AddEdgeToVertex (edge.GetFirst (), edge);
			AddEdgeToVertex (edge.GetSecond (), edge);
			AddEdgeToFace (i, edge);
			AddFaceToEdge (edge, i);
			edges.insert (edge);
		}
	}
}

void Mesh::AddFaceToVertex (const glm::vec3 &v, unsigned int face)
{
	auto it = vertex2faces.find (v);
	if (it != vertex2faces.end ())
		it->second.insert (face);
	else
		vertex2faces.insert (std::make_pair (v, std::set<unsigned int> ({face})));
}

void Mesh::AddEdgeToVertex (const glm::vec3 &v, const Edge &edge)
{
	auto it = vertex2edges.find (v);
	if (it != vertex2edges.end ())
		 it->second.insert (edge);
	else
		 vertex2edges.insert (std::make_pair(v, std::set<Edge> ({edge})));
}

void Mesh::AddEdgeToFace (unsigned int face, const Edge &edge)
{
	face2edges[face].insert (edge);
}

void Mesh::AddFaceToEdge (const Edge &edge, unsigned int face)
{
	auto it = edge2faces.find (edge);
	if (it != edge2faces.end ())
		 it->second.insert (face);
	else
		 edge2faces.insert (std::make_pair (edge,
																				std::set<unsigned int> ({face})));
}
