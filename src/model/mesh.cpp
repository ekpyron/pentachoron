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
#include <openctmpp.h>
#include <iostream>
#include "model/model.h"
#include "geometry.h"
#include "renderer.h"

Mesh::Mesh (Model &model) : trianglecount (0), vertexcount (0),
														parent (model), material (NULL),
														bsphere ({ glm::vec3 (0, 0, 0), 0.0f }),
														shadows (true)
{
}

Mesh::Mesh (Mesh &&mesh)
	: vertexarray (std::move (mesh.vertexarray)),
		depthonlyarray (std::move (mesh.depthonlyarray)),
		trianglecount (mesh.trianglecount),
		vertexcount (mesh.vertexcount),
		buffers (std::move (mesh.buffers)),
		indices (std::move (mesh.indices)),
		material (mesh.material),
		parent (mesh.parent),
		bsphere ({ mesh.bsphere.center, mesh.bsphere.radius }),
		shadows (mesh.shadows)
{
	mesh.trianglecount = mesh.vertexcount = 0;
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
	vertexcount = mesh.vertexcount;
	buffers = std::move (mesh.buffers);
	indices = std::move (indices);
	material = mesh.material;
	bsphere.center = mesh.bsphere.center;
	bsphere.radius = mesh.bsphere.radius;
	shadows = mesh.shadows;
	parent = std::move (mesh.parent);
	mesh.trianglecount = mesh.vertexcount = 0;
	mesh.material = NULL;
	mesh.bsphere.center = glm::vec3 (0, 0, 0);
	mesh.bsphere.radius = 0.0f;
	mesh.shadows = true;
}

bool Mesh::Load (const std::string &filename, const Material *mat,
								 glm::vec3 &min, glm::vec3 &max,
								 bool s)
{
	shadows = s;
	material = mat;

	try {
		CTMimporter importer;
		const glm::vec3 *vertices;
		const glm::vec3 *normals;
		const glm::vec4 *tangents;
		const glm::vec4 *bitangents;
		const GLuint *indexarray;

		importer.Load (filename.c_str ());

		if (!importer.GetInteger (CTM_HAS_NORMALS))
			 return false;

		if (importer.GetInteger (CTM_UV_MAP_COUNT) < 1)
			 return false;

		trianglecount = importer.GetInteger (CTM_TRIANGLE_COUNT);
		vertexcount = importer.GetInteger (CTM_VERTEX_COUNT);
		indexarray = importer.GetIntegerArray (CTM_INDICES);

		vertices = reinterpret_cast<const glm::vec3*>
			 (importer.GetFloatArray (CTM_VERTICES));
		normals = reinterpret_cast<const glm::vec3*>
			 (importer.GetFloatArray (CTM_NORMALS));

		CTMenum attrib = importer.GetNamedAttribMap ("TANGENTS");
		if (attrib == CTM_NONE)
			 return false;
		tangents = reinterpret_cast<const glm::vec4*>
			 (importer.GetFloatArray (attrib));

		attrib = importer.GetNamedAttribMap ("BITANGENTS");
		if (attrib == CTM_NONE)
			 return false;
		bitangents = reinterpret_cast<const glm::vec4*>
			 (importer.GetFloatArray (attrib));

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
												vertices, GL_STATIC_DRAW);
	buffers.emplace_back ();
	buffers.back ().Data (vertexcount * sizeof (glm::vec3),
												normals, GL_STATIC_DRAW);
	buffers.emplace_back ();

	{
		buffers.back ().Data (vertexcount * sizeof (glm::vec3),
													NULL, GL_STATIC_DRAW);
		glm::vec3 *dst = reinterpret_cast<glm::vec3*>
			 (buffers.back ().Map (GL_WRITE_ONLY));
		for (auto i = 0; i < vertexcount; i++)
		{
			dst[i] = glm::vec3 (tangents[i]);
		}
		buffers.back ().Unmap ();
	}

	depthonlyarray.VertexAttribOffset(buffers[0], 0, 3, GL_FLOAT,
																		GL_FALSE, 0, 0);
	depthonlyarray.EnableVertexAttrib (0);

	for (auto i = 0; i < 3; i++)
	{
		vertexarray.VertexAttribOffset (buffers[i], i, 3, GL_FLOAT,
																		GL_FALSE, 0, 0);
		vertexarray.EnableVertexAttrib (i);
	}

	for (auto i = 0; i < importer.GetInteger (CTM_UV_MAP_COUNT); i++)
	{
		const glm::vec2 *texcoords;
		texcoords = reinterpret_cast<const glm::vec2*>
			 (importer.GetFloatArray (CTM_UV_MAP_1));
		buffers.emplace_back ();
		buffers.back ().Data (vertexcount * sizeof (glm::vec2),
													texcoords,
													GL_STATIC_DRAW);
		vertexarray.VertexAttribOffset (buffers.back (), 3 + i, 2, GL_FLOAT,
																		GL_FALSE, 0, 0);
		vertexarray.EnableVertexAttrib (3 + i);
	}


	indices.Data (trianglecount * sizeof (GLuint) * 3,
								indexarray, GL_STATIC_DRAW);

	GL_CHECK_ERROR;
	return true;

	} catch (ctm_error &err) {
		(*logstream) << "Error loading " << filename << ": " << err.what ()
								 << std::endl;
		return false;
	}
}

bool Mesh::IsTransparent (void) const
{
	return material->IsTransparent ();
}

void Mesh::Render (const gl::Program &program, GLuint passtype)
{
	if (passtype == Geometry::Pass::ShadowMap && !shadows)
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
	
	gl::DrawElements (GL_TRIANGLES, trianglecount * 3,
										GL_UNSIGNED_INT, NULL);

	if (material->IsDoubleSided ())
		 gl::Enable (GL_CULL_FACE);

	GL_CHECK_ERROR;
}
