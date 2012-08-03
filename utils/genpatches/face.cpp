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
#include "face.h"
#include <stdexcept>

Face::Face (void)
{
}

Face::Face (const std::vector<glm::vec3> &v)
{
	Define (v);
}

Face::Face (const Face &f)
{
	vertices.assign (f.vertices.begin (), f.vertices.end ());
	for (const std::vector<glm::vec3> &t : f.texcoords)
	{
		texcoords.push_back (std::vector<glm::vec3> (t.begin (), t.end ()));
	}
}

Face::Face (Face &&f)
{
	vertices = std::move (f.vertices);
	texcoords = std::move (f.texcoords);
}

Face::~Face (void)
{
}

Face &Face::operator= (const Face &f)
{
	vertices.assign (f.vertices.begin (), f.vertices.end ());
	for (const std::vector<glm::vec3> &t : f.texcoords)
	{
		texcoords.push_back (std::vector<glm::vec3> (t.begin (), t.end ()));
	}
}

Face &Face::operator= (Face &&f)
{
	vertices = std::move (f.vertices);
	texcoords = std::move (f.texcoords);
}

void Face::Define (const std::vector<glm::vec3> &v)
{
	vertices.assign (v.begin (), v.end ());
}

void Face::AddTexCoords (const std::vector<glm::vec3> &t)
{
	if (t.size () != vertices.size ())
		 throw std::runtime_error ("invalid number of texture coordinates");
	texcoords.push_back (t);
}

unsigned int Face::GetNumVertices (void) const
{
	return vertices.size ();
}

const glm::vec3 &Face::GetVertex (unsigned int i) const
{
	if (i >= vertices.size ())
		 throw std::runtime_error ("vertex out of range");
	return vertices[i];
}

unsigned int Face::GetNumTexCoords (void) const
{
	return texcoords.size ();
}

const glm::vec3 &Face::GetTexCoord (unsigned int i, unsigned int t) const
{
	if (t >= texcoords.size ())
		 throw std::runtime_error ("invalid texture coordinate set");
	if (i >= texcoords[t].size ())
		 throw std::runtime_error ("vertex out of range");
	return texcoords[t][i];
}

glm::vec3 Face::GetCentroid (void) const
{
	glm::vec3 centroid;
	for (const glm::vec3 &v : vertices)
	{
		centroid += v;
	}
	centroid /= float (vertices.size ());
	return centroid;
}

bool Face::Contains (const glm::vec3 &vertex) const
{
	for (const glm::vec3 &v : vertices)
	{
		if (v == vertex)
			 return true;
	}
	return false;
}

const glm::vec3 &Face::GetFourth (const glm::vec3 &v1,
																	const glm::vec3 &v2,
																	const glm::vec3 &v3) const
{
	if (!Contains (v1) || !Contains (v2) || !Contains (v3))
		 throw std::runtime_error ("invalid vertices");
	for (const glm::vec3 &v : vertices)
	{
		if (v != v1 && v != v2 && v != v3)
			return v;
	}
}
