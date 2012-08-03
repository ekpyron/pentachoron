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
#include <assimp.hpp>
#include <aiScene.h>
#include <aiPostProcess.h>
#include <stdexcept>

void Mesh::Import (const std::string &filename, unsigned int meshid)
{
	Assimp::Importer importer;

	const aiScene *scene;

	scene = importer.ReadFile (filename,
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

	for (auto i = 0; i < mesh->mNumFaces; i++)
	{
		if((mesh->mFaces[i].mNumIndices != 3)
			 && (mesh->mFaces[i].mNumIndices != 4))
		{
			throw std::runtime_error ("invalid primitive type");
		}
		std::vector<glm::vec3> vertices;
		for (auto c = 0; c < mesh->mFaces[i].mNumIndices; c++)
		{
			const aiVector3D &v = mesh->mVertices[mesh->mFaces[i].mIndices[c]];
			vertices.push_back (glm::vec3 (v.x, v.y, v.z));
		}
		faces.push_back (Face (vertices));

		for (auto t = 0; t < mesh->GetNumUVChannels (); t++)
		{
			std::vector<glm::vec3> texcoords;
			for (auto c = 0; c < mesh->mFaces[i].mNumIndices; c++)
			{
				const aiVector3D &v
					 = mesh->mTextureCoords[t][mesh->mFaces[i].mIndices[c]];
				texcoords.push_back (glm::vec3 (v.x, v.y, v.z));
			}
			faces.back ().AddTexCoords (texcoords);
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
