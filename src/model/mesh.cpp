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
#include "model/mesh.h"
#include <iostream>
#include <fstream>
#include "model/model.h"
#include "geometry.h"
#include "renderer.h"
#include <pchm.h>

Mesh::Mesh (Model &model) : trianglecount (0), quadcount (0),
														patches (false), vertexcount (0),
														parent (model), material (NULL),
														bsphere ({ glm::vec3 (0, 0, 0), 0.0f }),
														shadows (true)
{
}

Mesh::Mesh (Mesh &&mesh)
	: vertexarray (std::move (mesh.vertexarray)),
		depthonlyarray (std::move (mesh.depthonlyarray)),
		quadcount (mesh.quadcount),
		trianglecount (mesh.trianglecount),
		patches (mesh.patches),
		vertexcount (mesh.vertexcount),
		buffers (std::move (mesh.buffers)),
		triangleindices (std::move (mesh.triangleindices)),
		quadindices (std::move (mesh.quadindices)),
		material (mesh.material),
		parent (mesh.parent),
		bsphere ({ mesh.bsphere.center, mesh.bsphere.radius }),
		shadows (mesh.shadows)
{
	mesh.trianglecount = mesh.quadcount = mesh.vertexcount = 0;
	mesh.patches = false;
	mesh.bsphere.center = glm::vec3 (0, 0, 0);
	mesh.bsphere.radius = 0.0f;
	mesh.material = NULL;
	mesh.shadows = true;
}

Mesh::~Mesh (void)
{
}

Mesh &Mesh::operator= (Mesh &&mesh)
{
	vertexarray = std::move (mesh.vertexarray);
	depthonlyarray = std::move (mesh.depthonlyarray);
	trianglecount = mesh.trianglecount;
	quadcount = mesh.quadcount;
	patches = mesh.patches;
	vertexcount = mesh.vertexcount;
	buffers = std::move (mesh.buffers);
	triangleindices = std::move (triangleindices);
	quadindices = std::move (quadindices);
	material = mesh.material;
	bsphere.center = mesh.bsphere.center;
	bsphere.radius = mesh.bsphere.radius;
	shadows = mesh.shadows;
	parent = std::move (mesh.parent);
	mesh.trianglecount = mesh.quadcount = mesh.vertexcount = 0;
	mesh.patches = false;
	mesh.material = NULL;
	mesh.bsphere.center = glm::vec3 (0, 0, 0);
	mesh.bsphere.radius = 0.0f;
	mesh.shadows = true;
}

bool Mesh::CastsShadow (void) const
{
	return shadows;
}

bool Mesh::IsTransparent (void) const
{
	return material->IsTransparent ();
}

bool Mesh::IsTessellated (void) const
{
	return patches;
}

bool Mesh::Load (const std::string &filename, const Material *mat,
								 glm::vec3 &min, glm::vec3 &max,
								 bool s)
{
	shadows = s;
	material = mat;

	pchm::model model;
	if (!model.Load (filename))
	{
		(*logstream) << "Cannot load " << filename << "." << std::endl;
		return false;
	}


	patches = model.Patches ();

	vertexcount = model.GetNumVertices ();
	trianglecount = model.GetNumTriangles ();
	quadcount = model.GetNumQuads ();

	const glm::vec3 *vertices = model.GetPositions ();

	// calculate the center of the bounding sphere
	// and calculate the bounding box
	{
		float factor = 1.0f / float (vertexcount);
		bsphere.center = glm::vec3 (0, 0, 0);
		for (auto i = 0; i < vertexcount; i++)
		{
			glm::vec3 vertex = vertices[i];
			bsphere.center += factor * vertex;
			if (vertex.x < min.x)
				 min.x = vertex.x;
			if (vertex.y < min.y)
				 min.y = vertex.y;
			if (vertex.z < min.z)
				 min.z = vertex.z;
			if (vertex.x > max.x)
				 max.x = vertex.x;
			if (vertex.y > max.y)
				 max.y = vertex.y;
			if (vertex.z > max.z)
				 max.z = vertex.z;
		}
	}

	// calculate the radius of the bounding sphere
	{
		bsphere.radius = 0;
		for (auto i = 0; i < vertexcount; i++)
		{
			glm::vec3 vertex = vertices[i];
			float distance = glm::distance (bsphere.center, vertex);
			if (distance > bsphere.radius)
				 bsphere.radius = distance;
		}
	}

	if (model.GetNumTexcoords () != 1)
	{
		(*logstream) << "Invalid number of texture coordinates in "
								 << filename << std::endl;
		return false;
	}

	if (patches)
	{
		buffers.emplace_back ();
		buffers.back ().Data (vertexcount * sizeof (glm::vec3),
													vertices, GL_STATIC_DRAW);
		
		depthonlyarray.VertexAttribOffset(buffers[0], 0, 3, GL_FLOAT,
																			GL_FALSE, 0, 0);
		depthonlyarray.EnableVertexAttrib (0);
		
		vertexarray.VertexAttribOffset (buffers[0], 0, 3, GL_FLOAT,
																		GL_FALSE, 0, 0);
		vertexarray.EnableVertexAttrib (0);
		
		buffers.emplace_back ();
		buffers.back ().Data (vertexcount * sizeof (glm::vec2),
													model.GetTexcoords (0), GL_STATIC_DRAW);
		vertexarray.VertexAttribOffset (buffers.back (), 1, 2, GL_FLOAT,
																		GL_FALSE, 0, 0);
		vertexarray.EnableVertexAttrib (1);

		if (trianglecount)
			 triangleindices.Data (trianglecount * sizeof (GLuint) * 15,
														 model.GetTriangleIndices (), GL_STATIC_DRAW);
		if (quadcount)
			 quadindices.Data (quadcount * sizeof (GLuint) * 20,
												 model.GetQuadIndices (), GL_STATIC_DRAW);
	}
	else
	{
		if (trianglecount)
		{
			buffers.emplace_back ();
			buffers.back ().Data (vertexcount * sizeof (glm::vec3),
														vertices, GL_STATIC_DRAW);

			buffers.emplace_back ();
			buffers.back ().Data (vertexcount * sizeof (glm::vec3),
														model.GetNormals (), GL_STATIC_DRAW);
			buffers.emplace_back ();
			buffers.back ().Data (vertexcount * sizeof (glm::vec3),
														model.GetTangents (), GL_STATIC_DRAW);

			depthonlyarray.VertexAttribOffset(buffers[0], 0, 3, GL_FLOAT,
																				GL_FALSE, 0, 0);
			depthonlyarray.EnableVertexAttrib (0);
	
			for (auto i = 0; i < 3; i++)
			{
				vertexarray.VertexAttribOffset (buffers[i], i, 3, GL_FLOAT,
																				GL_FALSE, 0, 0);
				vertexarray.EnableVertexAttrib (i);
			}

			buffers.emplace_back ();
			buffers.back ().Data (vertexcount * sizeof (glm::vec2),
														model.GetTexcoords (0), GL_STATIC_DRAW);
			vertexarray.VertexAttribOffset (buffers.back (), 3, 2, GL_FLOAT,
																			GL_FALSE, 0, 0);
			vertexarray.EnableVertexAttrib (3);

			triangleindices.Data (trianglecount * sizeof (GLuint) * 3,
														model.GetTriangleIndices (), GL_STATIC_DRAW);
		}
		if (quadcount)
		{
			(*logstream) << filename << " contains quads." << std::endl;
			return false;
		}
	}
	return true;
}

bool Mesh::LoadTriangles (std::ifstream &file, unsigned int num_texcoords,
													glm::vec3 &min, glm::vec3 &max)
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec2> texcoords;
	std::vector<GLuint> indexarray;

	if (num_texcoords > 1)
	{
		(*logstream) << "Multiple texture coordinates unsupported." << std::endl;
		return false;
	}
	if (num_texcoords < 1)
	{
		(*logstream) << "No texture coordinates." << std::endl;
		return false;
	}

	vertices.resize (vertexcount);
	normals.resize (vertexcount);
	tangents.resize (vertexcount);
	texcoords.resize (vertexcount);
	indexarray.resize (trianglecount * 3);

	file.read (reinterpret_cast<char*> (&vertices[0]),
						 vertexcount * sizeof (glm::vec3));
	if (file.gcount () != vertexcount * sizeof (glm::vec3))
	{
		(*logstream) << "Failed to load vertices." << std::endl;
		return false;
	}
	file.read (reinterpret_cast<char*> (&normals[0]),
						 vertexcount * sizeof (glm::vec3));
	if (file.gcount () != vertexcount * sizeof (glm::vec3))
	{
		(*logstream) << "Failed to load normals." << std::endl;
		return false;
	}
	file.read (reinterpret_cast<char*> (&tangents[0]),
						 vertexcount * sizeof (glm::vec3));
	if (file.gcount () != vertexcount * sizeof (glm::vec3))
	{
		(*logstream) << "Failed to load tangents." << std::endl;
		return false;
	}
	file.read (reinterpret_cast<char*> (&texcoords[0]),
						 vertexcount * sizeof (glm::vec2));
	if (file.gcount () != vertexcount * sizeof (glm::vec2))
	{
		(*logstream) << "Failed to load texture coordinates." << std::endl;
		return false;
	}
	file.read (reinterpret_cast<char*> (&indexarray[0]),
						 trianglecount * 3 * sizeof (GLuint));
	if (file.gcount () != trianglecount * 3 * sizeof (GLuint))
	{
		(*logstream) << "Failed to load indices." << std::endl;
		return false;
	}

	// calculate the center of the bounding sphere
	// and calculate the bounding box
	{
		float factor = 1.0f / float (vertexcount);
		bsphere.center = glm::vec3 (0, 0, 0);
		for (auto i = 0; i < vertexcount; i++)
		{
			glm::vec3 vertex = vertices[i];
			bsphere.center += factor * vertex;
			if (vertex.x < min.x)
				 min.x = vertex.x;
			if (vertex.y < min.y)
				 min.y = vertex.y;
			if (vertex.z < min.z)
				 min.z = vertex.z;
			if (vertex.x > max.x)
				 max.x = vertex.x;
			if (vertex.y > max.y)
				 max.y = vertex.y;
			if (vertex.z > max.z)
				 max.z = vertex.z;
		}
	}

	// calculate the radius of the bounding sphere
	{
		bsphere.radius = 0;
		for (auto i = 0; i < vertexcount; i++)
		{
			glm::vec3 vertex = vertices[i];
			float distance = glm::distance (bsphere.center, vertex);
			if (distance > bsphere.radius)
				 bsphere.radius = distance;
		}
	}

	buffers.emplace_back ();
	buffers.back ().Data (vertexcount * sizeof (glm::vec3),
												&vertices[0], GL_STATIC_DRAW);

	buffers.emplace_back ();
	buffers.back ().Data (vertexcount * sizeof (glm::vec3),
												&normals[0], GL_STATIC_DRAW);
	buffers.emplace_back ();
	buffers.back ().Data (vertexcount * sizeof (glm::vec3),
												&tangents[0], GL_STATIC_DRAW);

	depthonlyarray.VertexAttribOffset(buffers[0], 0, 3, GL_FLOAT,
																		GL_FALSE, 0, 0);
	depthonlyarray.EnableVertexAttrib (0);

	for (auto i = 0; i < 3; i++)
	{
		vertexarray.VertexAttribOffset (buffers[i], i, 3, GL_FLOAT,
																		GL_FALSE, 0, 0);
		vertexarray.EnableVertexAttrib (i);
	}

	buffers.emplace_back ();
	buffers.back ().Data (vertexcount * sizeof (glm::vec2),
												&texcoords[0], GL_STATIC_DRAW);
	vertexarray.VertexAttribOffset (buffers.back (), 3, 2, GL_FLOAT,
																	GL_FALSE, 0, 0);
	vertexarray.EnableVertexAttrib (3);

	triangleindices.Data (trianglecount * sizeof (GLuint) * 3,
												&indexarray[0], GL_STATIC_DRAW);

	GL_CHECK_ERROR;
	return true;
}

void Mesh::Render (const gl::Program &program, bool depthonly,
									 bool quads) const
{
	if (!r->culling.IsVisible
			(bsphere.center, bsphere.radius))
		return;

	material->Use (program);
	if (depthonly)
		 depthonlyarray.Bind ();
	else
	 vertexarray.Bind ();

	if (material->IsDoubleSided ())
		 gl::Disable (GL_CULL_FACE);

	if (patches)
	{
		if (quads)
		{
			quadindices.Bind (GL_ELEMENT_ARRAY_BUFFER);
			gl::PatchParameteri (GL_PATCH_VERTICES, 20);
			gl::DrawElements (GL_PATCHES, quadcount * 20,
												GL_UNSIGNED_INT, NULL);
		}
		else
		{
			triangleindices.Bind (GL_ELEMENT_ARRAY_BUFFER);
			gl::PatchParameteri (GL_PATCH_VERTICES, 15);
			gl::DrawElements (GL_PATCHES, trianglecount * 15,
												GL_UNSIGNED_INT, NULL);
		}
	}
	else
	{
		triangleindices.Bind (GL_ELEMENT_ARRAY_BUFFER);
		gl::DrawElements (GL_TRIANGLES, trianglecount * 3,
											GL_UNSIGNED_INT, NULL);
	}

	if (material->IsDoubleSided ())
		 gl::Enable (GL_CULL_FACE);

	GL_CHECK_ERROR;
}
