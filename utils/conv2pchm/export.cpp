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
#include <pchm.h>

bool export_mesh (const char *filename, Mesh *mesh)
{
	mesh->Sanitize ();

	if (mesh->edges != 3 && mesh->edges != 4)
	{
		fl_message_title ("Invalid primitive type.");
		fl_alert ("Only quad and triangle meshes can be exported.");
		return false;
	}

	pchm::model model;

	if (mesh->edges == 3)
	{
		 model.Define (mesh->vertices.size () / 3,
									 mesh->indices.size () / 3, 0);
		 model.SetTriangles (mesh->indices.data ());
	}
	else
	{
		 model.Define (mesh->vertices.size () / 3,
									 0, mesh->indices.size () / 4);
		 model.SetQuads (mesh->indices.data ());
	}


	model.SetPositions (reinterpret_cast<glm::vec3*> (mesh->vertices.data ()));
	model.SetNormals (reinterpret_cast<glm::vec3*> (mesh->normals.data ()));
	model.SetTangents (reinterpret_cast<glm::vec3*> (mesh->tangents.data ()));
	model.AddTexcoords (reinterpret_cast<glm::vec2*> (mesh->texcoords.data ()));

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

	if (!model.Save (file))
	{
		std::cerr << "FALSE" << std::endl;
		fl_message_title ("Cannot export the mesh");
		fl_alert ("Could not write to \"%s\": %s.", filename, strerror (errno));
		return false;
	}

	return true;
}
