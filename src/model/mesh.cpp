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
#include "model/mesh.h"
#include <iostream>
#include <fstream>
#include "model/model.h"
#include "geometry.h"
#include "renderer.h"

Mesh::Mesh (Model &model) : facecount (0), edgesperface (0), vertexcount (0),
														parent (model), material (NULL),
														bsphere ({ glm::vec3 (0, 0, 0), 0.0f }),
														shadows (true), tessellated (false),
														shadowtessellated (false)
{
}

Mesh::Mesh (Mesh &&mesh)
	: vertexarray (std::move (mesh.vertexarray)),
		depthonlyarray (std::move (mesh.depthonlyarray)),
		facecount (mesh.facecount),
		edgesperface (mesh.edgesperface),
		vertexcount (mesh.vertexcount),
		buffers (std::move (mesh.buffers)),
		indices (std::move (mesh.indices)),
		material (mesh.material),
		parent (mesh.parent),
		bsphere ({ mesh.bsphere.center, mesh.bsphere.radius }),
		shadows (mesh.shadows),
		tessellated (mesh.tessellated),
		shadowtessellated (mesh.shadowtessellated)
{
	mesh.facecount = mesh.edgesperface = mesh.vertexcount = 0;
	mesh.bsphere.center = glm::vec3 (0, 0, 0);
	mesh.bsphere.radius = 0.0f;
	mesh.material = NULL;
	mesh.shadows = true;
	mesh.tessellated = false;
	mesh.shadowtessellated = false;
}

Mesh::~Mesh (void)
{
}

Mesh &Mesh::operator= (Mesh &&mesh)
{
	vertexarray = std::move (mesh.vertexarray);
	depthonlyarray = std::move (mesh.depthonlyarray);
	facecount = mesh.facecount;
	edgesperface = mesh.edgesperface,
	vertexcount = mesh.vertexcount;
	buffers = std::move (mesh.buffers);
	indices = std::move (indices);
	material = mesh.material;
	bsphere.center = mesh.bsphere.center;
	bsphere.radius = mesh.bsphere.radius;
	shadows = mesh.shadows;
	tessellated = mesh.tessellated;
	shadowtessellated = mesh.shadowtessellated;
	parent = std::move (mesh.parent);
	mesh.facecount = mesh.edgesperface = mesh.vertexcount = 0;
	mesh.material = NULL;
	mesh.bsphere.center = glm::vec3 (0, 0, 0);
	mesh.bsphere.radius = 0.0f;
	mesh.shadows = true;
	mesh.tessellated = false;
	mesh.shadowtessellated = false;
}

bool Mesh::Load (const std::string &filename, const Material *mat,
								 glm::vec3 &min, glm::vec3 &max,
								 bool s, bool tess, bool shadowtess)
{
	shadows = s;
	tessellated = tess;
	shadowtessellated = shadowtess;
	material = mat;

	typedef struct header
	{
		 char magic[4];
		 GLuint edgesperface;
		 GLuint vertexcount;
		 GLuint facecount;
	} header_t;

	header_t header;

	std::ifstream file (filename, std::ios_base::in|std::ios_base::binary);

	if (!file.is_open ())
	{
		(*logstream) << "Cannot open " << filename << std::endl;
		return false;
	}

	file.read (reinterpret_cast<char*> (&header), sizeof (header_t));
	if (file.gcount () != sizeof (header_t))
	{
		(*logstream) << "Cannot read the dmf header of "
								 << filename << std::endl;
		return false;
	}

	const char magic[4] = { 'D', 'M', 'F', 0x00 };
	if (memcmp (header.magic, magic, 4))
	{
		(*logstream) << filename << " is no dmf model." << std::endl;
		return false;
	}

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec2> texcoords;
	std::vector<GLuint> indexarray;

	vertexcount = header.vertexcount;
	facecount = header.facecount;
	edgesperface = header.edgesperface;
	
	vertices.resize (vertexcount);
	normals.resize (vertexcount);
	tangents.resize (vertexcount);
	texcoords.resize (vertexcount);
	indexarray.resize (facecount * edgesperface);

	file.read (reinterpret_cast<char*> (&vertices[0]),
						 vertexcount * sizeof (glm::vec3));
	if (file.gcount () != vertexcount * sizeof (glm::vec3))
	{
		(*logstream) << "Failed to load the vertices from "
								 << filename << "." << std::endl;
		return false;
	}
	file.read (reinterpret_cast<char*> (&normals[0]),
						 vertexcount * sizeof (glm::vec3));
	if (file.gcount () != vertexcount * sizeof (glm::vec3))
	{
		(*logstream) << "Failed to load the normals from "
								 << filename << "." << std::endl;
		return false;
	}
	file.read (reinterpret_cast<char*> (&tangents[0]),
						 vertexcount * sizeof (glm::vec3));
	if (file.gcount () != vertexcount * sizeof (glm::vec3))
	{
		(*logstream) << "Failed to load the tangents from "
								 << filename << "." << std::endl;
		return false;
	}
	file.read (reinterpret_cast<char*> (&texcoords[0]),
						 vertexcount * sizeof (glm::vec2));
	if (file.gcount () != vertexcount * sizeof (glm::vec2))
	{
		(*logstream) << "Failed to load the texture coordinates from "
								 << filename << "." << std::endl;
		return false;
	}
	file.read (reinterpret_cast<char*> (&indexarray[0]),
						 facecount * edgesperface * sizeof (GLuint));
	if (file.gcount () != facecount * edgesperface * sizeof (GLuint))
	{
		(*logstream) << "Failed to load the indices from "
								 << filename << "." << std::endl;
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

	indices.Data (facecount * sizeof (GLuint) * edgesperface,
								&indexarray[0], GL_STATIC_DRAW);

	GL_CHECK_ERROR;
	return true;
}

bool Mesh::IsTessellated (void) const
{
	return tessellated;
}

bool Mesh::IsShadowTessellated (void) const
{
	return shadowtessellated;
}

bool Mesh::IsTransparent (void) const
{
	return material->IsTransparent ();
}

void Mesh::Render (const gl::Program &program, GLuint passtype)
{
	if (((passtype == Geometry::Pass::ShadowMap)
			 || (passtype == Geometry::Pass::ShadowMapTess))
			&& !shadows)
		 return;
	if (!IsTessellated () && (passtype == Geometry::Pass::GBufferTess))
			 return;
	if ((!IsTessellated () || !IsShadowTessellated ())
			&& (passtype == Geometry::Pass::ShadowMapTess))
		 return;
			
	if (!r->culling.IsVisible
			(bsphere.center, bsphere.radius))
		return;

	switch (passtype)
	{
	case Geometry::Pass::ShadowMap:
	case Geometry::Pass::GBufferSRAA:
		depthonlyarray.Bind ();
	break;
	default:
		material->Use (program);
		vertexarray.Bind ();
		break;
	}
	indices.Bind (GL_ELEMENT_ARRAY_BUFFER);

	if (material->IsDoubleSided ())
		 gl::Disable (GL_CULL_FACE);

	if (IsTessellated ())
	{
		if ((passtype == Geometry::Pass::ShadowMap) && !IsShadowTessellated ())
		{
			switch (edgesperface)
			{
			case 3:
				gl::DrawElements (GL_TRIANGLES, facecount * 3,
													GL_UNSIGNED_INT, NULL);
				break;
			case 4:
/*				gl::DrawElements (GL_QUADS, facecount * 4,
													GL_UNSIGNED_INT, NULL);*/
				break;
			}
		}
		else
		{
			gl::PatchParameteri (GL_PATCH_VERTICES, edgesperface);
			gl::DrawElements (GL_PATCHES, facecount * edgesperface,
												GL_UNSIGNED_INT, NULL);
		}
	}
	else
	{
		switch (edgesperface)
		{
		case 3:
			gl::DrawElements (GL_TRIANGLES, facecount * 3,
												GL_UNSIGNED_INT, NULL);
			break;
		case 4:
/*			gl::DrawElements (GL_QUADS, facecount * 4,
												GL_UNSIGNED_INT, NULL);*/
			break;
		}
	}

	if (material->IsDoubleSided ())
		 gl::Enable (GL_CULL_FACE);

	GL_CHECK_ERROR;
}
