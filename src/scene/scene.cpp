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
#include "scene/scene.h"
#include "geometry.h"
#include <assimp.hpp>
#include <aiScene.h>
#include <aiPostProcess.h>
#include <DefaultLogger.h>
#include <iostream>
#include <fstream>

Scene::Scene (Geometry *p) : parent (p)
{
}

Scene::Scene (Scene &&scene) : meshes (std::move (scene.meshes)),
															 materials (std::move (scene.materials)),
															 parent (scene.parent)
{
}

Scene::~Scene (void)
{
}

Scene &Scene::operator= (Scene &&scene)
{
	meshes = std::move (scene.meshes);
	materials = std::move (scene.materials);
	parent = scene.parent;
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

bool Scene::Load (const std::string &filename)
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
		(*logstream) << "The scene file " << filename
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
			(*logstream) << "Mesh " << mesh << " in " << modelfile
									 << " has an invalid primitive type." << std::endl;
			return false;
		}
		meshes.emplace_back (*this);
		if (!meshes.back ().Load (static_cast<void*> (scene->mMeshes[mesh]),
															&material))
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

	Assimp::DefaultLogger::kill ();
	return true;
}

void Scene::Render (GLuint pass, const gl::Program &program, bool shadowpass)
{
	for (Mesh &mesh : meshes)
	{
		mesh.Render (pass, program, shadowpass);
	}
}
