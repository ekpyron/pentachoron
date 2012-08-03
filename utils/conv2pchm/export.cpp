/*  
 * This file is part of conv2pchm.
 *
 * conv2pchm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * conv2pchm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with conv2pchm.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "export.h"
#include "mesh.h"
#include <fstream>
#include <cerrno>
#include <cstring>

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

bool export_mesh (const char *filename, Mesh *mesh)
{
	mesh->Sanitize ();

	{
		std::ifstream testfile (filename, std::ios_base::in);
		if (testfile.is_open ())
		{
			fl_message_title ("File already exists");
			if (fl_choice ("The file \"%s\" does already exist.\nDo you really "
										 "want to overwrite it?", "Override", "Cancel", NULL,
										 filename))
				 return false;
		}
	}

	std::ofstream file;

	file.clear ();
	file.open (filename, std::ios_base::out|std::ios_base::binary
						 |std::ios_base::trunc);
	if (!file.is_open ())
	{
		fl_message_title ("Cannot open output file.");
		fl_alert ("The file \"%s\" can't be opened for writing.", filename);
		return false;
	}

	const char magic[4] = {
		'P', 'C', 'H', 'M'
	};

	header_t header;

	header.version = PCHM_VERSION;
	memcpy (header.magic, magic, 4);
	if (mesh->edges == 3)
		 header.flags = PCHM_FLAGS_TRIANGLES;
	else if (mesh->edges == 4)
		 header.flags = PCHM_FLAGS_QUADS;
	else
	{
		fl_message_title ("Invalid primitive type.");
		fl_alert ("Only quad and triangle meshes can be exported.");
		return false;
	}
	header.vertexcount = mesh->vertices.size () / 3;
	header.facecount = mesh->indices.size () / mesh->edges;
	header.num_texcoords = 1;

	file.write ((char*) &header, sizeof (header_t));

	
	file.write ((char*) &mesh->vertices[0],
							mesh->vertices.size () * sizeof (GLfloat));
	file.write ((char*) &mesh->normals[0],
							mesh->normals.size () * sizeof (GLfloat));
	file.write ((char*) &mesh->tangents[0],
							mesh->tangents.size () * sizeof (GLfloat));
	file.write ((char*) &mesh->texcoords[0],
							mesh->texcoords.size () * sizeof (GLfloat));
	file.write ((char*) &mesh->indices[0],
							mesh->indices.size () * sizeof (GLuint));
	if (file.bad ())
	{
		fl_message_title ("Cannot export the mesh");
		fl_alert ("Could not write to \"%s\": %s.", filename, strerror (errno));
		return false;
	}

	return true;
}
