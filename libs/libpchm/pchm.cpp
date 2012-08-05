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
#include "pchm.h"
#include <fstream>
#include <cstring>
#include <stdexcept>

namespace pchm {

typedef struct header
{
	 char magic[4];
	 uint16_t version;
	 uint16_t flags;
	 uint16_t num_texcoords;
	 uint32_t vertexcount;
	 uint32_t trianglecount;
	 uint32_t quadcount;
} header_t;

#define PCHM_FLAGS_GREGORY_PATCHES       0x0001

#define PCHM_VERSION 0x0000

model::model (void) : patches (false)
{
}

model::model (const model &m)
	: patches (m.patches)
{
	positions.assign (m.positions.begin (), m.positions.end ());
	normals.assign (m.normals.begin (), m.normals.end ());
	tangents.assign (m.tangents.begin (), m.tangents.end ());
	texcoords.resize (m.texcoords.size ());
	for (size_t i = 0; i < m.texcoords.size (); i++)
	{
		texcoords[i].assign (m.texcoords[i].begin (), m.texcoords[i].end ());
	}
	triangleindices.assign (m.triangleindices.begin (),
													m.triangleindices.end ());
	quadindices.assign (m.quadindices.begin (),
											m.quadindices.end ());
}

model::model (model &&m)
	: positions (std::move (m.positions)), normals (std::move (m.normals)),
		tangents (std::move (m.tangents)), texcoords (std::move (m.texcoords)),
		triangleindices (std::move (m.triangleindices)),
		quadindices (std::move (m.quadindices)),
		patches (m.patches)
{																						
	m.patches = false;
}

model::~model (void)
{
}

model &model::operator= (const model &m)
{
	positions.assign (m.positions.begin (), m.positions.end ());
	normals.assign (m.normals.begin (), m.normals.end ());
	tangents.assign (m.tangents.begin (), m.tangents.end ());
	texcoords.resize (m.texcoords.size ());
	for (size_t i = 0; i < m.texcoords.size (); i++)
	{
		texcoords[i].assign (m.texcoords[i].begin (), m.texcoords[i].end ());
	}
	triangleindices.assign (m.triangleindices.begin (),
													m.triangleindices.end ());
	quadindices.assign (m.quadindices.begin (),
											m.quadindices.end ());
	patches = m.patches;
}

model &model::operator= (model &&m)
{
	positions = std::move (m.positions);
	normals = std::move (m.normals);
	tangents = std::move (m.tangents);
	texcoords = std::move (m.texcoords);
	triangleindices = std::move (m.triangleindices);
	quadindices = std::move (m.quadindices);
	patches = m.patches;
	m.patches = false;
}

bool model::Patches (void) const
{
	return patches;
}

bool model::Load (const std::string &filename)
{
	std::ifstream file (filename, std::ios_base::in|std::ios_base::binary);
	if (!file.is_open ())
		 return false;
	return Load (file);
}

bool model::Load (std::istream &in)
{
	header_t header;

	in.read (reinterpret_cast<char*> (&header), sizeof (header_t));
	if (in.gcount () != sizeof (header_t))
		 return false;

	const char magic[4] = { 'P', 'C', 'H', 'M' };
	if (memcmp (header.magic, magic, 4))
		 return false;

	if (header.version != PCHM_VERSION)
		 return false;

	patches = header.flags & PCHM_FLAGS_GREGORY_PATCHES;

	positions.resize (header.vertexcount);
	in.read (reinterpret_cast<char*> (positions.data ()),
					 header.vertexcount * sizeof (glm::vec3));
	if (in.gcount () != header.vertexcount * sizeof (glm::vec3))
		 return false;

	if (patches)
	{
		texcoords.clear ();
		for (size_t i = 0; i < header.num_texcoords; i++)
		{
			texcoords.push_back (std::vector<glm::vec2> ());
			texcoords.back ().resize (header.vertexcount);
			in.read (reinterpret_cast<char*> (texcoords[i].data ()),
							 header.vertexcount * sizeof (glm::vec2));
			if (in.gcount () != header.vertexcount * sizeof (glm::vec2))
				 return false;
		}

		triangleindices.resize (header.trianglecount * 15);
		if (header.trianglecount)
		{
			in.read (reinterpret_cast<char*> (triangleindices.data ()),
							 triangleindices.size () * sizeof (unsigned int));
			if (in.gcount () != triangleindices.size () * sizeof (unsigned int))
				 return false;
		}
		quadindices.resize (header.quadcount * 20);
		if (header.quadcount)
		{
			in.read (reinterpret_cast<char*> (quadindices.data ()),
							 quadindices.size () * sizeof (unsigned int));
			if (in.gcount () != quadindices.size () * sizeof (unsigned int))
				 return false;
		}
	}
	else
	{
		normals.resize (header.vertexcount);
		in.read (reinterpret_cast<char*> (normals.data ()),
						 header.vertexcount * sizeof (glm::vec3));
		if (in.gcount () != header.vertexcount * sizeof (glm::vec3))
			 return false;
		
		tangents.resize (header.vertexcount);
		in.read (reinterpret_cast<char*> (tangents.data ()),
						 header.vertexcount * sizeof (glm::vec3));
		if (in.gcount () != header.vertexcount * sizeof (glm::vec3))
			 return false;

		texcoords.clear ();
		for (size_t i = 0; i < header.num_texcoords; i++)
		{
			texcoords.push_back (std::vector<glm::vec2> ());
			texcoords.back ().resize (header.vertexcount);
			in.read (reinterpret_cast<char*> (texcoords[i].data ()),
							 header.vertexcount * sizeof (glm::vec2));
			if (in.gcount () != header.vertexcount * sizeof (glm::vec2))
				 return false;
		}

		triangleindices.resize (header.trianglecount * 3);
		if (header.trianglecount)
		{
			in.read (reinterpret_cast<char*> (triangleindices.data ()),
							 triangleindices.size () * sizeof (unsigned int));
			if (in.gcount () != triangleindices.size () * sizeof (unsigned int))
				 return false;
		}
		quadindices.resize (header.quadcount * 4);
		if (header.quadcount)
		{
			in.read (reinterpret_cast<char*> (quadindices.size ()),
							 quadindices.size () * sizeof (unsigned int));
			if (in.gcount () != quadindices.size () * sizeof (unsigned int))
				 return false;
		}
	}

	return true;
}

bool model::Save (const std::string &filename) const
{
	std::ofstream file (filename, std::ios_base::out|std::ios_base::binary
											|std::ios_base::trunc);
	return Save (file);
}

bool model::Save (std::ostream &file) const
{
	return false;
}

void model::Define (unsigned int vertices, unsigned int triangles,
										unsigned int quads)
{
	normals.clear ();
	tangents.clear ();
	texcoords.clear ();
	positions.resize (vertices);
	triangleindices.resize (triangles * 3);
	quadindices.resize (quads * 4);
	patches = false;
}

void model::SetTriangles (const unsigned int *i)
{
	triangleindices.assign (i, i + triangleindices.size ());
}

void model::SetQuads (const unsigned int *i)
{
	quadindices.assign (i, i + quadindices.size ());
}

void model::SetPositions (const glm::vec3 *p)
{
	positions.assign (p, p + positions.size ());
}

void model::SetNormals (const glm::vec3 *n)
{
	normals.assign (n, n + positions.size ());
}

void model::SetTangents (const glm::vec3 *t)
{
	tangents.assign (t, t + positions.size ());
}

void model::SetTexcoords (unsigned int id, const glm::vec2 *t)
{
	if (id >= texcoords.size ())
		 throw std::runtime_error ("invalid texture coordinate id");
	texcoords[id].assign (t, t + positions.size ());
}

void model::AddTexcoords (const glm::vec2 *t)
{
	texcoords.push_back (std::vector<glm::vec2> ());
	texcoords.back ().assign (t, t + positions.size ());
}

unsigned int model::GetNumVertices (void) const
{
	return positions.size ();
}

const glm::vec3 *model::GetPositions (void) const
{
	return positions.data ();
}

const glm::vec3 *model::GetNormals (void) const
{
	if (normals.empty ())
		 return NULL;
	return normals.data ();
}

const glm::vec3 *model::GetTangents (void) const
{
	if (tangents.empty ())
		 return NULL;
	return tangents.data ();
}

const glm::vec2 *model::GetTexcoords (unsigned int id) const
{
	if (id >= texcoords.size ())
		 return NULL;
	if (texcoords[id].empty ())
		 return NULL;
	return texcoords[id].data ();
}

unsigned int model::GetNumTexcoords (void) const
{
	return texcoords.size ();
}

unsigned int model::GetNumTriangles (void) const
{
	if (patches)
		 return triangleindices.size () / 15;
	else
		 return triangleindices.size () / 3;
}

unsigned int model::GetNumQuads (void) const
{
	if (patches)
		 return quadindices.size () / 20;
	else
		 return quadindices.size () / 4;
}

const unsigned int *model::GetTriangleIndices (void) const
{
	return triangleindices.data ();
}

const unsigned int *model::GetQuadIndices (void) const
{
	return quadindices.data ();
}

} /* namespace pchm */
