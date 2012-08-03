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
#include "mesh.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>

Mesh::Mesh (void)
{
}

Mesh::~Mesh (void)
{
}

const std::set<unsigned int> &Mesh::GetFacesFromVertex
(const glm::vec3 &v) const
{
	auto it = vertex2faces.find (v);
	if (it == vertex2faces.end ())
		 throw std::runtime_error ("invalid vertex");
	return it->second;
}

const std::set<Edge> &Mesh::GetEdgesFromVertex (const glm::vec3 &v) const
{
	auto it = vertex2edges.find (v);
	if (it == vertex2edges.end ())
		 throw std::runtime_error ("invalid vertex");
	return it->second;
}

const std::set<Edge> &Mesh::GetEdgesFromFace (unsigned int faceid) const
{
	if (faceid >= faces.size ())
		 throw std::runtime_error ("face out of range");
	return face2edges[faceid];
}

const std::set<unsigned int> &Mesh::GetFacesFromEdge (const Edge &e) const
{
	auto it = edge2faces.find (e);
	if (it == edge2faces.end ())
		 throw std::runtime_error ("invalid edge");
	return it->second;
}

unsigned int Mesh::GetNumFaces (void) const
{
	return faces.size ();
}

void Mesh::GeneratePatches (void)
{
	for (auto i = 0; i < faces.size (); i++)
	{
		switch (faces[i].GetNumVertices ())
		{
		case 3:
			trianglepatches.push_back (Triangle2Patch (i));
			break;
		case 4:
			quadpatches.push_back (Quad2Patch (i));
			break;
		default:
			throw std::runtime_error ("invalid primitive type");
		}
	}
}

TrianglePatch Mesh::Triangle2Patch (unsigned int faceid) const
{
	throw std::runtime_error ("triangle patches are not yet supported");
}

bool Mesh::IsBorder (const glm::vec3 &vertex) const
{
	for (const Edge &edge : GetEdgesFromVertex (vertex))
	{
		if (IsBorder (edge))
			 return true;
	}
	return false;
}

bool Mesh::IsBorder (const Edge &edge) const
{
	return GetFacesFromEdge (edge).size () < 2;
}

QuadPatch Mesh::Quad2Patch (unsigned int faceid) const
{
	QuadPatch patch;

	const Face &face = faces[faceid];

	for (auto c = 0; c < 4; c++)
	{
		const glm::vec3 &v = face.GetVertex (c);

		if (IsBorder (v))
		{
			if (GetFacesFromVertex (v).size () == 1)
			{
				patch.p[c] = v;
			}
			else
			{
				// find border edges
				std::vector<Edge> borders;
				for (const Edge &edge : GetEdgesFromVertex (v))
				{
					if (IsBorder (edge))
						 borders.push_back (edge);
				}
				if (borders.size () < 2)
					 throw std::runtime_error ("too few adjacent border edges");
				if (borders.size () > 2)
					 throw std::runtime_error ("too many adjacent border edges");
				patch.p[c] = borders[0].GetOther (v);
				patch.p[c] += borders[1].GetOther (v);
				patch.p[c] += 4.0f * v;
				patch.p[c] /= 6.0f;

			}
		}
		else
		{
			const std::set<Edge> &edges = GetEdgesFromVertex (v);
			for (const Edge &edge : edges)
			{
				patch.p[c] += edge.GetMidpoint ();
			}
			for (const unsigned int &f : GetFacesFromVertex (v))
			{
				patch.p[c] += faces[f].GetCentroid ();
			}
			patch.p[c] *= 4.0f / ((float (edges.size ()) + 5.0f)
														* float (edges.size ()));
			patch.p[c] += ((float (edges.size ()) - 3.0f)
										 / (float (edges.size ()) + 5.0f)) * v;
		}
	}

	for (auto i = 0; i < 4; i++)
	{
		patch.eplus[i] = GetEdgePoint (face.GetVertex (i),
																	 Edge (face.GetVertex (i),
																				 face.GetVertex ((i + 1) % 4)),
																	 faceid, patch.p[i],
																	 patch.p[(i + 1) % 4]);
		
		patch.eminus[i] = GetEdgePoint (face.GetVertex (i),
																		Edge (face.GetVertex (i),
																					face.GetVertex ((i + 3) % 4)),
																		faceid, patch.p[i],
																		patch.p[(i + 3) % 4]);
	}
	for (auto i = 0; i < 4; i++)
	{
		patch.fplus[i] = GetFacePoint (face.GetVertex (i),
																	 Edge (face.GetVertex (i),
																				 face.GetVertex ((i + 1) % 4)),
																	 faceid, patch.p[i],
																	 patch.eplus[i],
																	 patch.eminus[(i + 1) % 4]);
		patch.fminus[i] = GetFacePoint (face.GetVertex (i),
																		Edge (face.GetVertex (i),
																					face.GetVertex ((i + 3) % 4)),
																		faceid, patch.p[i],
																		patch.eminus[i],
																		patch.eplus[(i + 3) % 4]);
	}


	for (auto t = 0; t < face.GetNumTexCoords (); t++)
	{
		patch.texcoords.push_back (Patch<4>::texcoord_t ());
		for (auto i = 0; i < 4; i++)
			 patch.texcoords.back ().p[i] = glm::vec2 (face.GetTexCoord (i, t));
	}

	return patch;
}

glm::vec3 Mesh::GetEdgePoint (const glm::vec3 &v, const Edge &edge,
															unsigned int faceid, const glm::vec3 &p,
															const glm::vec3 &p2) const
{
	const Face &face = faces[faceid];
	if (IsBorder (v))
	{
		if (GetFacesFromEdge (edge).size () == 1)
		{
			return (2.0f * v + edge.GetOther (v)) / 3.0f;
		}
		else
		{
			glm::vec3 e;

			Edge b1 = GetSecondEdgeOnFace (v, faceid, edge);
			unsigned int faceid2 = GetSecondFaceOnEdge (edge, faceid);
			Edge b2 = GetSecondEdgeOnFace (v, faceid2, edge);

			float gamma;
			gamma = (3.0f / 8.0f) - (1.0f / 4.0f) * (PCH_PI / float (GetValence (v)));
			e = (3.0f / 4.0f - gamma) * v
				 + gamma * edge.GetOther (v);

			switch (face.GetNumVertices ())
			{
			case 4:
				e += (1.0f / 16.0f) * b1.GetOther (v);
				e += (1.0f / 16.0f) * face.GetFourth (v, b1.GetOther (v),
																							edge.GetOther (v));
				break;
			case 3:
				e += (1.0f / 8.0f) * b1.GetOther (v);
				break;
			}

			const Face &face2 = faces[faceid2];
			switch (face2.GetNumVertices ())
			{
			case 4:
				e += (1.0f / 16.0f) * b2.GetOther (v);
				e += (1.0f / 16.0f) * face2.GetFourth (v, b2.GetOther (v),
																							 edge.GetOther (v));
				break;
			case 3:
				e += (1.0f / 8.0f) * b2.GetOther (v);
				break;
			}

			return e;
/*			// TODO
			std::vector<Edge> m;
			std::vector<unsigned int> c;
			unsigned int current_face = faceid;
			m.push_back (edge);
			c.push_back (current_face);

			while (true)
			{
				m.push_back (GetSecondEdgeOnFace (v, current_face, m.back ()));
				if (IsBorder (m.back ()))
					 break;
				c.push_back (GetSecondFaceOnEdge (m.back (), current_face));
				current_face = c.back ();
			}

			std::vector<Edge> m2;
			std::vector<unsigned int> c2;

			current_face = faceid;
			c2.push_back (current_face);
			m2.push_back (edge);
			do
			{
				c2.push_back (GetSecondFaceOnEdge (m2.back (), current_face));
				current_face = c2.back ();
				m2.push_back (GetSecondEdgeOnFace (v, current_face, m2.back ()));
			} while (!IsBorder (m2.back ()));

			m.insert (m.end (), m2.rbegin (), m2.rend () - 1);
			c.insert (c.end (), c2.rbegin (), c2.rend () - 1);

			glm::vec3 q;
			float cos_pi_over_n = cosf (PCH_PI / float (m.size ()));
			float cos_two_pi_over_n = cosf (2.0f * PCH_PI / float (m.size ()));
			float sigma = 1.0f / sqrtf (4.0f + cos_pi_over_n * cos_pi_over_n);
			for (auto i = 0; i < m.size (); i++)
			{
				q += (1 - sigma * cos_pi_over_n)
					 * cosf (2.0f * PCH_PI * float (i) / float (m.size ()))
					 * m[i].GetMidpoint ();
			}
			q *= 2.0f / float (m.size ());

			glm::vec3 q2;
			cos_pi_over_n = cosf (PCH_PI / float (c.size ()));
			cos_two_pi_over_n = cosf (2.0f * PCH_PI / float (c.size ()));
			sigma = 1.0f / sqrtf (4.0f + cos_pi_over_n * cos_pi_over_n);
			for (auto i = 0; i < c.size (); i++)
			{
				q2 += cosf ((2.0f * PCH_PI * float (i) + PCH_PI) / float (c.size ()))
					 * 2.0f * sigma * faces[c[i]].GetCentroid ();
			}
			q2 *= 2.0f / float (c.size ());

			cos_pi_over_n = cosf (PCH_PI / float (c.size () + m.size ()));
			cos_two_pi_over_n = cosf (2.0f * PCH_PI / float (c.size () + m.size ()));
			float lambda = (cos_pi_over_n * sqrtf (18.0f + 2.0f * cos_two_pi_over_n)
											+ cos_two_pi_over_n + 5.0f) / 16.0f;

			return p + (2.0f / 3.0f) * lambda * q;*/
		}
	}

	std::vector<Edge> m;
	std::vector<unsigned int> c;
	unsigned int current_face = faceid;
	m.push_back (edge);
	c.push_back (current_face);

	do
	{
		m.push_back (GetSecondEdgeOnFace (v, current_face, m.back ()));
		c.push_back (GetSecondFaceOnEdge (m.back (), current_face));
		current_face = c.back ();
	} while (current_face != faceid);
	c.pop_back ();
	m.pop_back ();

	glm::vec3 q;
	float cos_pi_over_n = cosf (PCH_PI / float (m.size ()));
	float cos_two_pi_over_n = cosf (2.0f * PCH_PI / float (m.size ()));
	float sigma = 1.0f / sqrtf (4.0f + cos_pi_over_n * cos_pi_over_n);
	float lambda = (cos_pi_over_n * sqrtf (18.0f + 2.0f * cos_two_pi_over_n)
									+ cos_two_pi_over_n + 5.0f) / 16.0f;
	for (auto i = 0; i < m.size (); i++)
	{
		q += (1 - sigma * cos_pi_over_n)
			 * cosf (2.0f * PCH_PI * float (i) / float (m.size ()))
			 * m[i].GetMidpoint ();
		q += cosf ((2.0f * PCH_PI * float (i) + PCH_PI) / float (m.size ()))
			 * 2.0f * sigma * faces[c[i]].GetCentroid ();
	}
	q *= 2.0f / float (m.size ());

	return p + (2.0f / 3.0f) * lambda * q;
}

unsigned int Mesh::GetSecondFaceOnEdge (const Edge &edge,
																			 unsigned int faceid) const
{
	std::set<unsigned int> faces = GetFacesFromEdge (edge);
	if (faces.erase (faceid) != 1)
		 throw std::runtime_error ("invalid face");
	if (faces.size () != 1)
		 throw std::runtime_error ("invalid number of faces on edge");
	return *faces.begin ();
	
}

Edge Mesh::GetSecondEdgeOnFace (const glm::vec3 &v, unsigned int faceid,
																const Edge &edge) const
{
	const std::set<Edge> &e1 = GetEdgesFromVertex (v);
	const std::set<Edge> &e2 = GetEdgesFromFace (faceid);
	std::set<Edge> e;
	std::set_intersection (e1.begin (), e1.end (), e2.begin (), e2.end (),
												 std::inserter (e, e.begin ()));

	if (e.size () != 2)
		 throw std::runtime_error ("unexpected set intersection");
	if (e.erase (edge) != 1)
		 throw std::runtime_error ("invalid edge");
	return *e.begin ();
}

glm::vec3 Mesh::GetFacePoint (const glm::vec3 &v, const Edge &edge,
															unsigned int faceid, const glm::vec3 &p,
															const glm::vec3 &e1, const glm::vec3 &e2) const
{
	glm::vec3 r;
	unsigned int n0 = GetValence (v);
	unsigned int n1 = GetValence (edge.GetOther (v));
	float d = faces[faceid].GetNumVertices ();

	if (IsBorder (v))
	{
		Edge m = GetSecondEdgeOnFace (v, faceid, edge);
		r = (2.0f / 3.0f) * (m.GetMidpoint () - p)
			 + (4.0f / 3.0f) * (faces[faceid].GetCentroid () - edge.GetMidpoint ());
		n0++;
	}
	else
	{
		std::vector<Edge> m;
		std::vector<unsigned int> c;
		unsigned int current_face = faceid;
		m.push_back (edge);
		c.push_back (current_face);

		do
		{
			m.push_back (GetSecondEdgeOnFace (v, current_face, m.back ()));
			c.push_back (GetSecondFaceOnEdge (m.back (), current_face));
			current_face = c.back ();
		} while (current_face != faceid);
		c.pop_back ();
		m.pop_back ();

		r = (1.0f / 3.0f) * (m[1].GetOther (v) - m.back ().GetOther (v))
			 + (2.0f / 3.0f) * (faces[c[0]].GetCentroid ()
													- faces[c.back ()].GetCentroid ());
	}

	float c0 = cosf (2.0f * PCH_PI / float (n0));
	float c1 = cosf (2.0f * PCH_PI / float (n1));
	return (1.0f / d) * (c1 * p + (d - 2.0f * c0 - c1) * e1
											 + 2.0f * c0 * e2 + r);
}

unsigned int Mesh::GetValence (const glm::vec3 &v) const
{
	return GetEdgesFromVertex (v).size ();
}

const Face &Mesh::GetFace (unsigned int i) const
{
	if (i >= faces.size ())
		 throw std::runtime_error ("face out of range");
	return faces[i];
}

const std::set<Edge> &Mesh::GetEdges (void) const
{
	return edges;
}

unsigned int Mesh::GetNumQuadPatches (void) const
{
	return quadpatches.size ();
}

const QuadPatch &Mesh::GetQuadPatch (unsigned int q) const
{
	return quadpatches[q];
}
