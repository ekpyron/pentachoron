/*
 * This file is part of DRE.
 *
 * DRE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * DRE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DRE.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "scene/mesh.h"
#include <assimp.hpp>
#include <aiScene.h>
#include <iostream>
#include "scene/scene.h"
#include "geometry.h"

Mesh::Mesh (Scene &scene) : trianglecount (0), vertexcount (0),
														parent (scene), material (NULL)
{
}

Mesh::Mesh (Mesh &&mesh)
	: vertexarray (std::move (mesh.vertexarray)),
		shadowpassarray (std::move (mesh.shadowpassarray)),
		trianglecount (mesh.trianglecount),
		vertexcount (mesh.vertexcount),
		buffers (std::move (mesh.buffers)),
		indices (std::move (mesh.indices)),
		material (mesh.material),
		parent (mesh.parent)
{
	mesh.trianglecount = mesh.vertexcount = 0;
	mesh.material = NULL;
}

Mesh::~Mesh (void)
{
}

Mesh &Mesh::operator= (Mesh &&mesh)
{
	vertexarray = std::move (mesh.vertexarray);
	shadowpassarray = std::move (mesh.shadowpassarray);
	trianglecount = mesh.trianglecount;
	vertexcount = mesh.vertexcount;
	buffers = std::move (mesh.buffers);
	indices = std::move (indices);
	material = mesh.material;
	parent = std::move (mesh.parent);
	mesh.trianglecount = mesh.vertexcount = 0;
	mesh.material = NULL;
}

bool Mesh::Load (void *m, const Material *mat)
{
	material = mat;
	aiMesh *mesh = static_cast<aiMesh*> (m);

	if (mesh->HasBones ())
	{
		(*logstream) << "Warning: A mesh contains bones." << std::endl;
	}
	if (!mesh->HasFaces ())
	{
		(*logstream) << "No faces in the mesh." << std::endl;
		return false;
	}
	if (!mesh->HasNormals ())
	{
		(*logstream) << "No normals in the mesh." << std::endl;
		return false;
	}
	if (mesh->GetNumColorChannels () > 0)
	{
		(*logstream) << "Warning: A mesh contains color channels." << std::endl;
	}
	if (mesh->GetNumUVChannels () < 1)
	{
		(*logstream) << "Warning: No texture coords in the mesh." << std::endl;
	}
	if (mesh->GetNumUVChannels () > 1)
	{
		(*logstream) << "Warning: Several texture coords in the mesh." << std::endl;
	}
	if (!mesh->HasTangentsAndBitangents ())
	{
		(*logstream) << "Warning: No tangents and bitangents in the mesh."
								 << std::endl;
	}

	trianglecount = mesh->mNumFaces;
	vertexcount = mesh->mNumVertices;

	buffers.emplace_back ();
	buffers.back ().Data (vertexcount * sizeof (aiVector3D),
												mesh->mVertices, GL_STATIC_DRAW);
	buffers.emplace_back ();
	buffers.back ().Data (vertexcount * sizeof (aiVector3D),
												mesh->mNormals, GL_STATIC_DRAW);
	buffers.emplace_back ();
	buffers.back ().Data (vertexcount * sizeof (aiVector3D),
												mesh->mTangents, GL_STATIC_DRAW);

	shadowpassarray.VertexAttribOffset(buffers[0], 0, 3, GL_FLOAT,
																		 GL_FALSE, 0, 0);
	shadowpassarray.EnableVertexAttrib (0);

	for (auto i = 0; i < 3; i++)
	{
		vertexarray.VertexAttribOffset (buffers[i], i, 3, GL_FLOAT,
																		GL_FALSE, 0, 0);
		vertexarray.EnableVertexAttrib (i);
	}

	for (auto i = 0; i < mesh->GetNumUVChannels (); i++)
	{
		buffers.emplace_back ();
		buffers.back ().Data (vertexcount * sizeof (GLfloat) * 2, NULL,
													GL_STATIC_DRAW);
		GLfloat *ptr;
		ptr = static_cast<GLfloat*> (buffers.back ().Map (GL_WRITE_ONLY));
		for (auto j = 0; j < vertexcount; j++)
		{
			GLfloat t1, t2;
			ptr[j * 2 + 0] = mesh->mTextureCoords[i][j].x;
			ptr[j * 2 + 1] = mesh->mTextureCoords[i][j].y;
		}
		buffers.back ().Unmap ();
		vertexarray.VertexAttribOffset (buffers.back (), 3 + i, 2, GL_FLOAT,
																		GL_FALSE, 0, 0);
		vertexarray.EnableVertexAttrib (3 + i);
	}


	indices.Data (trianglecount * sizeof (GLuint) * 3,
								NULL, GL_STATIC_DRAW);
	GLuint *indexptr = static_cast<GLuint*> (indices.Map (GL_WRITE_ONLY));
	for (auto i = 0; i < trianglecount; i++)
	{
		for (auto j = 0; j < 3; j++)
		{
			 indexptr[i*3 + j] = mesh->mFaces[i].mIndices[j];
		}
	}
	indices.Unmap ();

	GL_CHECK_ERROR;
	return true;
}

void Mesh::Render (const gl::Program &program, bool shadowpass) const
{
	if (!shadowpass)
	{
		material->Use (program);
		vertexarray.Bind ();
	}
	else
	{
		shadowpassarray.Bind ();
	}
	indices.Bind (GL_ELEMENT_ARRAY_BUFFER);
	gl::DrawElements (GL_TRIANGLES, trianglecount * 3, GL_UNSIGNED_INT, NULL);
	GL_CHECK_ERROR;
}
