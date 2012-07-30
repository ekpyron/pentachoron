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
#include "model/model.h"
#include "geometry.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include "renderer.h"

Model::Model (void)
{
}

Model::Model (Model &&model) : meshes (std::move (model.meshes)),
															 materials (std::move (model.materials)),
															 queries (std::move (model.queries))
{
	bbox.min = model.bbox.min;
	bbox.max = model.bbox.max;
	bbox.buffer = std::move (model.bbox.buffer);
	bbox.indices = std::move (model.bbox.indices);
	bbox.array = std::move (model.bbox.array);
	bsphere.center = model.bsphere.center;
	bsphere.radius = model.bsphere.radius;
}

Model::~Model (void)
{
}

Model &Model::operator= (Model &&model)
{
	meshes = std::move (model.meshes);
	materials = std::move (model.materials);
	bbox.min = model.bbox.min;
	bbox.max = model.bbox.max;
	bbox.buffer = std::move (model.bbox.buffer);
	bbox.indices = std::move (model.bbox.indices);
	bbox.array = std::move (model.bbox.array);
	bsphere.center = model.bsphere.center;
	bsphere.radius = model.bsphere.radius;
}

bool Model::Load (const std::string &filename)
{
#ifdef DEBUG
	debug.filename = filename;
#endif /* DEBUG */

	YAML::Node desc;
	std::ifstream file (MakePath ("models", filename), std::ifstream::in);
	if (!file.is_open ())
	{
		(*logstream) << "Cannot open " << filename << std::endl;
		return false;
	}

	desc = YAML::Load (file);
	if (!desc.IsMap ())
	{
		(*logstream) << "The model file " << filename
								 << " has an invalid format." << std::endl;
		return false;
	}

	if (!desc["meshes"].IsSequence ())
	{
		(*logstream) << filename << " has an invalid format." << std::endl;
		(*logstream) << desc["meshes"].Type () << std::endl;
		return false;
	}

	bbox.min = glm::vec3 (FLT_MAX, FLT_MAX, FLT_MAX);
	bbox.max = glm::vec3 (-FLT_MAX, -FLT_MAX, -FLT_MAX);

	GLuint num_meshes = 0;
	for (const YAML::Node &node : desc["meshes"])
	{
		std::string filename = MakePath ("models",
																		 node["filename"].as<std::string> ());
		const Material &material = r->geometry.GetMaterial
			 (node["material"].as<std::string> ());

		Mesh mesh (*this);
		mesh.Load (filename, &material, bbox.min, bbox.max,
							 node["shadows"].as<bool> (true));

		switch (mesh.GetPatchType ())
		{
		case 0:
			meshes.emplace_back (std::move (mesh));
			num_meshes++;
			break;
		default:
			(*logstream) << "Mesh " << filename
									 << " has an invalid type." << std::endl;
			break;
		}
	}

	if (num_meshes < 1)
	{
		(*logstream) << filename << " contains no meshes." << std::endl;
		return false;
	}

	{
		glm::vec3 bboxvertex[8];
		bboxvertex[0] = glm::vec3 (bbox.min.x, bbox.min.y, bbox.min.z);
		bboxvertex[1] = glm::vec3 (bbox.max.x, bbox.min.y, bbox.min.z);
		bboxvertex[2] = glm::vec3 (bbox.max.x, bbox.max.y, bbox.min.z);
		bboxvertex[3] = glm::vec3 (bbox.min.x, bbox.max.y, bbox.min.z);
		bboxvertex[4] = glm::vec3 (bbox.min.x, bbox.min.y, bbox.max.z);
		bboxvertex[5] = glm::vec3 (bbox.max.x, bbox.min.y, bbox.max.z);
		bboxvertex[6] = glm::vec3 (bbox.max.x, bbox.max.y, bbox.max.z);
		bboxvertex[7] = glm::vec3 (bbox.min.x, bbox.max.y, bbox.max.z);
		bbox.buffer.Data (8 * sizeof (glm::vec3), &bboxvertex[0], GL_STATIC_DRAW);

		glm::detail::tvec3<GLubyte> bboxindices[12];

		bboxindices[0] = glm::detail::tvec3<GLubyte> (0, 1, 2);
		bboxindices[1] = glm::detail::tvec3<GLubyte> (2, 3, 0);
		bboxindices[2] = glm::detail::tvec3<GLubyte> (1, 5, 6);
		bboxindices[3] = glm::detail::tvec3<GLubyte> (1, 6, 2);
		bboxindices[4] = glm::detail::tvec3<GLubyte> (5, 4, 6);
		bboxindices[5] = glm::detail::tvec3<GLubyte> (4, 7, 6);
		bboxindices[6] = glm::detail::tvec3<GLubyte> (4, 0, 3);
		bboxindices[7] = glm::detail::tvec3<GLubyte> (4, 3, 7);
		bboxindices[8] = glm::detail::tvec3<GLubyte> (3, 2, 6);
		bboxindices[9] = glm::detail::tvec3<GLubyte> (3, 6, 7);
		bboxindices[10] = glm::detail::tvec3<GLubyte> (0, 4, 1);
		bboxindices[11] = glm::detail::tvec3<GLubyte> (4, 5, 1);

		bbox.indices.Data (12 * sizeof (glm::detail::tvec3<GLubyte>),
											 &bboxindices[0], GL_STATIC_DRAW);
	}

	bbox.array.VertexAttribOffset (bbox.buffer, 0, 3, GL_FLOAT,
																 GL_FALSE, 0, 0);
	bbox.array.EnableVertexAttrib (0);

// TODO: Calculate a decent bounding sphere.
//       This is a very rough approximation.
	{
		bsphere.center = 0.5f * (bbox.min + bbox.max);
		bsphere.radius = glm::distance (bbox.min, bsphere.center);
	}

	return true;
}

void Model::Render (GLuint pass, const gl::Program &program)
{
	GLuint result = GL_TRUE;
	GLuint passtype;

	if (!r->culling.IsVisible
			(bsphere.center, bsphere.radius))
		return;

	passtype = pass & Geometry::Pass::Mask;

	bool shadowpass = (passtype == Geometry::Pass::ShadowMap)
		 || (passtype == Geometry::Pass::ShadowMapTess);

	std::map<GLuint, gl::Query>::iterator query;
	switch (passtype)
	{
	case Geometry::Pass::GBuffer:
	case Geometry::Pass::GBufferTransparency:
		query = queries.find (pass);
		if (query == queries.end ())
		{
			auto ret = queries.insert (std::pair<GLuint, gl::Query>
																 (pass, gl::Query ()));
			if (ret.second == false)
				 throw std::runtime_error ("Cannot insert element to map.");
			query = ret.first;
		}

		if (query->second.IsValid ())
		{
			do
			{
				query->second.Get (GL_QUERY_RESULT_AVAILABLE, &result);
			} while (result == GL_FALSE);
			if (result == GL_TRUE)
			{
				query->second.Get (GL_QUERY_RESULT, &result);
			}
			else
			{
				(*logstream) << "Query result not yet available" << std::endl;
			}
		}

		query->second.Begin (GL_ANY_SAMPLES_PASSED);
		break;
	case Geometry::Pass::GBufferSRAA:
		GLuint p;
		p = (pass & (~Geometry::Pass::Mask)) | Geometry::Pass::GBuffer;
		query = queries.find (pass);
		if (query != queries.end ())
		{
			if (query->second.IsValid ())
			{
				query->second.Get (GL_QUERY_RESULT_AVAILABLE, &result);
				if (result == GL_TRUE)
				{
					query->second.Get (GL_QUERY_RESULT, &result);
				}
				else result = GL_TRUE;
			}
		}
		break;
	}

	if (result == GL_TRUE)
	{
		switch (passtype)
		{
		case Geometry::Pass::GBufferTess:
			for (Mesh &mesh : tessellated)
			{
				if (!shadowpass || mesh.CastsShadow ())
					 mesh.Render (program, false);
			}
			break;
		case Geometry::Pass::ShadowMapTess:
			for (Mesh &mesh : tessellated)
			{
				if (!shadowpass || mesh.CastsShadow ())
					 mesh.Render (program, true);
			}
			break;
		case Geometry::Pass::GBufferTransparency:
			for (Mesh &mesh : transparent)
			{
				if (!shadowpass || mesh.CastsShadow ())
					 mesh.Render (program, false);
			}
			break;
		case Geometry::Pass::ShadowMap:
			for (Mesh &mesh : transparent)
			{
				if (!shadowpass || mesh.CastsShadow ())
					 mesh.Render (program, true);
			}
		case Geometry::Pass::GBuffer:
			for (Mesh &mesh : meshes)
			{
				if (!shadowpass || mesh.CastsShadow ())
					 mesh.Render (program, false);
			}
			break;
		case Geometry::Pass::GBufferSRAA:
			for (Mesh &mesh : meshes)
			{
				if (!shadowpass || mesh.CastsShadow ())
					 mesh.Render (program, true);
			}
			break;
		}
	}
	else if (passtype != Geometry::Pass::GBufferSRAA)
	{
		if (passtype != Geometry::Pass::GBufferTransparency)
		{
			gl::ColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			gl::DepthMask (GL_FALSE);
		}
		r->geometry.bboxprogram.Use ();
		bbox.array.Bind ();
		bbox.indices.Bind (GL_ELEMENT_ARRAY_BUFFER);
		gl::DrawElements (GL_TRIANGLES, 36,
											GL_UNSIGNED_BYTE, NULL);
		program.Use ();
		if (passtype != Geometry::Pass::GBufferTransparency)
		{
			gl::DepthMask (GL_TRUE);
			gl::ColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}
		culled++;
	}
	switch (passtype)
	{
	case Geometry::Pass::GBuffer:
	case Geometry::Pass::GBufferTransparency:
		 gl::Query::End (GL_ANY_SAMPLES_PASSED);
		 break;
	}
}

GLuint Model::culled = 0;
