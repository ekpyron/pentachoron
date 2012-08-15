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
#include <cstring>
#include <iostream>
#include <sstream>
#include <Importer.hpp>
#include <scene.h>
#include <postprocess.h>
#include <stdexcept>

pchm::model model;

int main (int argc, char *argv[])
{
	unsigned int meshid;

	if (argc != 4)
	{
		std::cerr << "Usage: " << argv[0] << " [input] [mesh] [output]"
							<< std::endl;
		return -1;
	}

	{
		std::stringstream stream (argv[2]);
		if ((stream >> meshid).bad ())
		{
			std::cerr << "Invalid mesh." << std::endl;
			std::cerr << "Usage: " << argv[0] << " [input] [mesh] [output]"
								<< std::endl;
			return -1;
		}
	}

	Assimp::Importer importer;

	const aiScene *scene;

	scene = importer.ReadFile (argv[1],
														 aiProcess_JoinIdenticalVertices
														 | aiProcess_GenUVCoords
														 | aiProcess_SortByPType
														 | aiProcess_TransformUVCoords
														 | aiProcess_OptimizeMeshes
														 | aiProcess_OptimizeGraph);
	if (!scene)
		 throw std::runtime_error ("cannot load the model file");

	if (meshid >= scene->mNumMeshes)
		 throw std::runtime_error ("invalid mesh id");

	aiMesh *mesh = scene->mMeshes[meshid];

	std::vector<unsigned int> triangleindices;
	std::vector<unsigned int> quadindices;

	for (auto i = 0; i < mesh->mNumFaces; i++)
	{
		if (mesh->mFaces[i].mNumIndices == 3)
		{
			for (auto c = 0; c < 3; c++)
				 triangleindices.push_back (mesh->mFaces[i].mIndices[c]);
		}
		else if (mesh->mFaces[i].mNumIndices == 4)
		{
			for (auto c = 0; c < 4; c++)
				 quadindices.push_back (mesh->mFaces[i].mIndices[c]);
		}
		else
		{
			throw std::runtime_error ("invalid primitive type");
		}
	}

	model.Define (mesh->mNumVertices, triangleindices.size () / 3,
								quadindices.size () / 4);

	model.SetTriangles (triangleindices.data ());
	model.SetQuads (quadindices.data ());

	model.SetPositions (reinterpret_cast<glm::vec3*> (mesh->mVertices));
	for (auto i = 0; i < mesh->GetNumUVChannels (); i++)
	{
		std::vector<glm::vec2> texcoords;
		for (auto c = 0; c < mesh->mNumVertices; c++)
		{
			texcoords.push_back (glm::vec2 (mesh->mTextureCoords[i][c].x,
																			mesh->mTextureCoords[i][c].y));
		}
		model.AddTexcoords (texcoords.data ());
	}

	model.GeneratePatches ();

	if (!model.Save (argv[3]))
	{
		std::cerr << "Could not save to " << argv[3] << "." << std::endl;
		return -1;
	}

	return 0;
}
