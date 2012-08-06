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
#include "mesh.h"
#include <stdexcept>

namespace pchm {

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

void model::GeneratePatches (void)
{
	normals.clear ();
	tangents.clear ();

	Mesh mesh;
	mesh.Import (*this);

	mesh.GeneratePatches ();

	patches = true;

	positions.clear ();
	normals.clear ();
	tangents.clear ();
	texcoords.clear ();
	triangleindices.clear ();
	quadindices.clear ();

	std::vector<vertex_t> data;

	for (auto p = 0; p < mesh.GetNumTrianglePatches (); p++)
	{
		const TrianglePatch &patch = mesh.GetTrianglePatch (p);
		for (auto i = 0; i < 3; i++)
		{
			{
				vertex_t v;
				v.position = patch.p[i];
				for (auto c = 0; c < patch.texcoords.size (); c++)
					 v.texcoords.push_back (patch.texcoords[c].p[i]);
				AddToList (v, data, triangleindices);
			}
			{
				vertex_t v;
				v.position = patch.eminus[i];
				for (auto c = 0; c < patch.texcoords.size (); c++)
					 v.texcoords.push_back (patch.texcoords[c].eminus[i]);
				AddToList (v, data, triangleindices);
			}
			{
				vertex_t v;
				v.position = patch.eplus[i];
				for (auto c = 0; c < patch.texcoords.size (); c++)
					 v.texcoords.push_back (patch.texcoords[c].eplus[i]);
				AddToList (v, data, triangleindices);
			}
			{
				vertex_t v;
				v.position = patch.fminus[i];
				for (auto c = 0; c < patch.texcoords.size (); c++)
					 v.texcoords.push_back (patch.texcoords[c].fminus[i]);
				AddToList (v, data, triangleindices);
			}
			{
				vertex_t v;
				v.position = patch.fplus[i];
				for (auto c = 0; c < patch.texcoords.size (); c++)
					 v.texcoords.push_back (patch.texcoords[c].fplus[i]);
				AddToList (v, data, triangleindices);
			}
		}
	}
	for (auto p = 0; p < mesh.GetNumQuadPatches (); p++)
	{
		const QuadPatch &patch = mesh.GetQuadPatch (p);
		{
			vertex_t v;
			v.position = patch.p[0];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].p[0]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.eminus[0];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].eminus[0]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.eplus[3];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].eplus[3]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.p[3];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].p[3]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.eplus[0];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].eplus[0]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.fminus[0];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].fminus[0]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.fplus[0];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].fplus[0]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.fminus[3];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].fminus[3]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.fplus[3];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].fplus[3]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.eminus[3];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].eminus[3]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.eminus[1];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].eminus[1]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.fminus[1];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].fminus[1]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.fplus[1];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].fplus[1]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.fminus[2];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].fminus[2]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.fplus[2];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].fplus[2]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.eplus[2];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].eplus[2]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.p[1];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].p[1]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.eplus[1];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].eplus[1]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.eminus[2];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].eminus[2]);
			AddToList (v, data, quadindices);
		}

		{
			vertex_t v;
			v.position = patch.p[2];
			for (auto i = 0; i < patch.texcoords.size (); i++)
				 v.texcoords.push_back (patch.texcoords[i].p[2]);
			AddToList (v, data, quadindices);
		}
	}

	texcoords.resize (data.begin ()->texcoords.size ());
	for (const vertex_t &v : data)
	{
		positions.push_back (v.position);
		for (auto i = 0; i < data.begin ()->texcoords.size (); i++)
		{
			if (i >= v.texcoords.size ())
				 throw std::runtime_error ("invalid number of texture coordinates");
			texcoords[i].push_back (v.texcoords[i]);
		}
	}
}

} /* namespace pchm */
