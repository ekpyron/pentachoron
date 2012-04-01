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
#include "renderer.h"

Mesh::Mesh (Scene &scene) : trianglecount (0), vertexcount (0),
														parent (scene), material (NULL),
														bsphere ({ glm::vec3 (0, 0, 0), 0.0f })
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
		parent (mesh.parent),
		bsphere ({ mesh.bsphere.center, mesh.bsphere.radius })
{
	mesh.trianglecount = mesh.vertexcount = 0;
	mesh.bsphere.center = glm::vec3 (0, 0, 0);
	mesh.bsphere.radius = 0.0f;
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
	bsphere.center = mesh.bsphere.center;
	bsphere.radius = mesh.bsphere.radius;
	parent = std::move (mesh.parent);
	mesh.trianglecount = mesh.vertexcount = 0;
	mesh.material = NULL;
	mesh.bsphere.center = glm::vec3 (0, 0, 0);
	mesh.bsphere.radius = 0.0f;
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

	bbox.min = glm::vec3 (FLT_MAX, FLT_MAX, FLT_MAX);
	bbox.max = glm::vec3 (-FLT_MAX, -FLT_MAX, -FLT_MAX);

	// calculate the center of the bounding sphere
	// calculate the bounding box
	{
		float factor = 1.0f / float (vertexcount);
		bsphere.center = glm::vec3 (0, 0, 0);
		for (auto i = 0; i < vertexcount; i++)
		{
			glm::vec3 vertex (mesh->mVertices[i].x, mesh->mVertices[i].y,
												mesh->mVertices[i].z);
			bsphere.center += factor * vertex;
			if (vertex.x < bbox.min.x)
				 bbox.min.x = vertex.x;
			if (vertex.y < bbox.min.y)
				 bbox.min.y = vertex.y;
			if (vertex.z < bbox.min.z)
				 bbox.min.z = vertex.z;
			if (vertex.x > bbox.max.x)
				 bbox.max.x = vertex.x;
			if (vertex.y > bbox.max.y)
				 bbox.max.y = vertex.y;
			if (vertex.z > bbox.max.z)
				 bbox.max.z = vertex.z;
		}
	}
	// calculate the radius of the bounding sphere
	{
		bsphere.radius = 0;
		for (auto i = 0; i < vertexcount; i++)
		{
			glm::vec3 vertex (mesh->mVertices[i].x, mesh->mVertices[i].y,
												mesh->mVertices[i].z);
			float distance = glm::distance (bsphere.center, vertex);
			if (distance > bsphere.radius)
				 bsphere.radius = distance;
		}
	}

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

bool Mesh::IsTransparent (void) const
{
	return material->IsTransparent ();
}

void Mesh::OcclusionQuery (void)
{
	query.Begin (GL_SAMPLES_PASSED);

	gl::ColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	gl::DepthMask (GL_FALSE);

	gl::ColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	gl::Query::End (GL_SAMPLES_PASSED);
}

void Mesh::Render (const gl::Program &program, bool shadowpass) const
{
	if (!parent.parent->renderer->culling.IsVisible
			(bsphere.center, bsphere.radius))
		return;
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
