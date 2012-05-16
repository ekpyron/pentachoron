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
#include <assimp.hpp>
#include <aiScene.h>
#include <aiPostProcess.h>
#include <DefaultLogger.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "renderer.h"

Model::Model (Geometry *p) : parent (p)
{
}

Model::Model (Model &&model) : meshes (std::move (model.meshes)),
															 materials (std::move (model.materials)),
															 parent (model.parent)
{
}

Model::~Model (void)
{
}

Model &Model::operator= (Model &&model)
{
	meshes = std::move (model.meshes);
	materials = std::move (model.materials);
	parent = model.parent;
}

/** @cond */
class LogStream : public Assimp::LogStream
{
public:
	 LogStream (void);
	 ~LogStream (void);
	 void write (const char *msg);
};

LogStream::LogStream (void)
{
}

LogStream::~LogStream (void)
{
}

void LogStream::write (const char *msg)
{
	(*logstream) << msg;
}
/** @endcond */

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

	Assimp::DefaultLogger::create ("", Assimp::Logger::VERBOSE);
	Assimp::DefaultLogger::get ()->attachStream (new LogStream,
																							 //Assimp::Logger::DEBUGGING|
																							 //Assimp::Logger::INFO|
																							 Assimp::Logger::ERR|
																							 Assimp::Logger::WARN);
	Assimp::Importer importer;

	const aiScene *scene;
	std::string modelfile (MakePath ("models", desc["model"].as<std::string> ()));
	scene = importer.ReadFile (modelfile,aiProcess_CalcTangentSpace
														 | aiProcess_Triangulate
														 | aiProcess_JoinIdenticalVertices
														 | aiProcess_SortByPType
														 | aiProcess_GenSmoothNormals
														 | aiProcess_GenUVCoords
														 | aiProcess_TransformUVCoords
														 | aiProcess_OptimizeMeshes
														 | aiProcess_OptimizeGraph);
	if (!scene)
	{
		(*logstream) << "Cannot load " << modelfile << ":" << std::endl
								 << importer.GetErrorString () << std::endl;
		return false;
	}

	scene = importer.ApplyPostProcessing (aiProcess_CalcTangentSpace);
	if (!scene)
	{
		(*logstream) << "Cannot load " << modelfile << ":" << std::endl
								 << importer.GetErrorString () << std::endl;
		return false;
	}

	if (scene->HasAnimations ())
	{
		(*logstream) << "Warning: " << modelfile << " contains animations."
								 << std::endl;
	}
	if (scene->HasCameras ())
	{
		(*logstream) << "Warning: " << modelfile << " contains camera information."
								 << std::endl;
	}
	if (scene->HasLights ())
	{
		(*logstream) << "Warning: " << modelfile << " contains lights."
								 << std::endl;
	}
	if (scene->HasTextures ())
	{
		(*logstream) << "Warning: " << modelfile << " contains textures."
								 << std::endl;
	}
	if (scene->HasMaterials ())
	{
		(*logstream) << "Warning: " << modelfile << " contains materials."
								 << std::endl;
	}

	if (!desc["meshes"].IsSequence ())
	{
		(*logstream) << filename << " has an invalid format." << std::endl;
		(*logstream) << desc["meshes"].Type () << std::endl;
		return false;
	}

	bbox.min = glm::vec3 (FLT_MAX, FLT_MAX, FLT_MAX);
	bbox.max = glm::vec3 (-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (const YAML::Node &node : desc["meshes"])
	{
		unsigned int mesh = node["number"].as<unsigned int> ();
		const Material &material = parent->GetMaterial
			 (node["material"].as<std::string> ());
		if (mesh >= scene->mNumMeshes)
		{
			(*logstream) << "Requested mesh number " << (mesh+1)
									 << ", but only " << scene->mNumMeshes
									 << " were found in " << modelfile << "."
									 << std::endl;
			return false;
		}
		if (scene->mMeshes[mesh]->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
		{
			(*logstream) << "Mesh " << (mesh+1) << " in " << modelfile
									 << " has an invalid primitive type." << std::endl;
			return false;
		}
		meshes.emplace_back (*this);
		if (!meshes.back ().Load (static_cast<void*> (scene->mMeshes[mesh]),
															&material, bbox.min, bbox.max,
															node["shadows"].as<bool> (true)))
		{
			(*logstream) << "Mesh " << mesh << " in " << modelfile
									 << " could not be loaded." << std::endl;
			return false;
		}
	}

	if (meshes.size () < 1)
	{
		(*logstream) << filename << " contains no triangle meshes." << std::endl;
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

	Assimp::DefaultLogger::kill ();
	return true;
}

void Model::Render (GLuint pass, const gl::Program &program, bool shadowpass,
										bool transparent)
{
	GLuint result = GL_TRUE;

	if (!parent->renderer->culling.IsVisible
			(bsphere.center, bsphere.radius))
		return;

	if (!shadowpass && !transparent)
	{
		auto query = queries.find (pass);
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
	}

	if (result == GL_TRUE)
	{
		if (transparent)
			 gl::Disable (GL_CULL_FACE);
		for (Mesh &mesh : meshes)
		{
			if (transparent == mesh.IsTransparent ())
				 mesh.Render (program, shadowpass);
		}
		if (transparent)
			 gl::Enable (GL_CULL_FACE);
	}
	else
	{
		gl::ColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		gl::DepthMask (GL_FALSE);
		gl::Disable (GL_CULL_FACE);
		parent->bboxprogram.Use ();
		bbox.array.Bind ();
		bbox.indices.Bind (GL_ELEMENT_ARRAY_BUFFER);
		gl::DrawElements (GL_TRIANGLES, 36,
											GL_UNSIGNED_BYTE, NULL);
		program.Use ();
		gl::Enable (GL_CULL_FACE);
		gl::DepthMask (GL_TRUE);
		gl::ColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		culled++;
	}
	if (!shadowpass && !transparent)
		 gl::Query::End (GL_ANY_SAMPLES_PASSED);
}

GLuint Model::culled = 0;
