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
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <iterator>

typedef struct vertex
{
	 glm::vec3 position;
	 std::vector<glm::vec2> texcoords;
} vertex_t;

void AddToList (const vertex_t &v, std::vector<vertex_t> &data,
								std::vector<unsigned int> &indices)
{
	data.push_back (v);
	indices.push_back (data.size () - 1);
}

void Mesh::Export (const std::string &filename)
{
	std::ofstream file (filename, std::ios_base::out
											|std::ios_base::binary
											|std::ios_base::trunc);
	if (!file.is_open ())
		 throw std::runtime_error ("cannot open output file");

	std::vector<vertex_t> data;
	std::vector<unsigned int> indices;

	for (QuadPatch &patch : quadpatches)
	{
		{
			vertex_t v;
			v.position = patch.p[0];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].p[0]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.eminus[0];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].eminus[0]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.eplus[3];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].eplus[3]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.p[3];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].p[3]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.eplus[0];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].eplus[0]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.fminus[0];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].fminus[0]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.fplus[0];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].fplus[0]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.fminus[3];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].fminus[3]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.fplus[3];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].fplus[3]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.eminus[3];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].eminus[3]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.eminus[1];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].eminus[1]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.fminus[1];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].fminus[1]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.fplus[1];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].fplus[1]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.fminus[2];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].fminus[2]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.fplus[2];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].fplus[2]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.eplus[2];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].eplus[2]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.p[1];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].p[1]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.eplus[1];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].eplus[1]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.eminus[2];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].eminus[2]);
			AddToList (v, data, indices);
		}

		{
			vertex_t v;
			v.position = patch.p[2];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].p[2]);
			AddToList (v, data, indices);
		}
	}

#define PCHM_FLAGS_TRIANGLES     0x0001
#define PCHM_FLAGS_QUADS         0x0002
#define PCHM_FLAGS_PATCHES       0x0004

#define PCHM_VERSION 0x0000

	typedef struct header
	{
		 char magic[4];
		 uint16_t version;
		 uint16_t flags;
		 uint16_t num_texcoords;
		 uint32_t vertexcount;
		 uint32_t facecount;
	} header_t;

	header_t header;

	const char magic[4] = {
		'P', 'C', 'H', 'M'
	};
	header.version = PCHM_VERSION;
	memcpy (header.magic, magic, 4);
	header.flags = PCHM_FLAGS_QUADS|PCHM_FLAGS_PATCHES;
	header.vertexcount = data.size ();
	header.facecount = quadpatches.size ();
	header.num_texcoords = data.begin ()->texcoords.size ();

	file.write ((char*) &header, sizeof (header_t));
	for (const vertex_t &v : data)
		 file.write ((char*) &v.position.x, sizeof (glm::vec3));
	for (auto i = 0; i < header.num_texcoords; i++)
	{
		for (const vertex_t &v : data)
		{
			if (i >= v.texcoords.size ())
				 throw std::runtime_error ("invalid number of texture coordinates");
			file.write ((char*) &v.texcoords[i].x, sizeof (glm::vec2));
		}
	}
	for (unsigned int &idx : indices)
	{
		file.write ((char*) &idx, sizeof (unsigned int));
	}
}
