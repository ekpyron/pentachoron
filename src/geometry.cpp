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
#include "geometry.h"
#include "renderer.h"
#include <fstream>

Geometry::Geometry (void)
{
}

Geometry::~Geometry (void)
{
	for (auto it = materials.begin (); it != materials.end (); it++)
	{
		delete (*it).second;
	}
	materials.clear ();
}

const glm::vec3 &Geometry::GetBoxMin (void)
{
	return boxmin;
}

const glm::vec3 &Geometry::GetBoxMax (void)
{
	return boxmax;
}

const Material &Geometry::GetMaterial (const std::string &name)
{
	Material *material;
	auto it = materials.find (name);
	if (it != materials.end ())
	{
		return *(it->second);
	}
	material = new Material;
	if (!material->Load (name))
	{
		delete material;
		throw std::runtime_error (std::string ("Cannot load the material ")
															+ name + ".");
	}
	auto ret = materials.insert (std::make_pair(name, material));
	if (!ret.second)
	{
		delete material;
		throw std::runtime_error (std::string ("Cannot add ") + name
															+ " to the material list.");
	}
	return *ret.first->second;
}


bool Geometry::Init (void)
{
	{
		gl::Shader vshader (GL_VERTEX_SHADER),
			 fshader (GL_FRAGMENT_SHADER);
		std::string src;
		if (!ReadFile (MakePath ("shaders", "bbox", "vshader.txt"), src))
			 return false;
		vshader.Source (src);
		if (!vshader.Compile ())
		{
			(*logstream) << "Cannot compile "
									 << MakePath ("shaders", "bbox", "vshader.txt")
									 << ":" << std::endl << vshader.GetInfoLog () << std::endl;
			return false;
		}

		if (!ReadFile (MakePath ("shaders", "bbox", "fshader.txt"), src))
			 return false;
		fshader.Source (src);
		if (!fshader.Compile ())
		{
			(*logstream) << "Cannot compile "
									 << MakePath ("shaders", "bbox", "fshader.txt")
									 << ":" << std::endl << fshader.GetInfoLog () << std::endl;
			return false;
		}

		bboxprogram.Attach (vshader);
		bboxprogram.Attach (fshader);
		if (!bboxprogram.Link ())
		{
			(*logstream) << "Cannot link the bbox shader:" << std::endl
									 << bboxprogram.GetInfoLog () << std::endl;
			return false;
		}		
	}

	sampler.Parameter (GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	sampler.Parameter (GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	sampler.Parameter (GL_TEXTURE_WRAP_S, GL_REPEAT);
	sampler.Parameter (GL_TEXTURE_WRAP_T, GL_REPEAT);

	std::ifstream file (MakePath ("scene.yaml"), std::ifstream::in);
	std::map<std::string, GLuint> names;
	if (!file.is_open ())
	{
		(*logstream) << "Cannot open " << MakePath ("scene.yaml")
								 << "." << std::endl;
		return false;
	}

	std::vector<YAML::Node> streams;
	streams = YAML::LoadAll (file);
	if (streams.size () != 2)
	{
		(*logstream) << MakePath ("scene.yaml")
								 << " has an invalid format." << std::endl;
		return false;
	}

	YAML::Node scene = streams[0];
	if (!scene.IsMap ())
	{
		(*logstream) << MakePath ("scene.yaml")
								 << " has an invalid format." << std::endl;
		return false;
	}
	{
		YAML::Node modeldesc;
		modeldesc = scene["models"];
		GLuint id = 0;
		for (YAML::const_iterator it = modeldesc.begin ();
				 it != modeldesc.end (); it++)
		{
			names[it->first.as<std::string> ()] = id++;
			models.emplace_back ();
			if (!models.back ().Load (it->second.as<std::string> ()))
				 return false;
		}
	}
	try {
	boxmin = scene["boxmin"].as<glm::vec3> ();
	boxmax = scene["boxmax"].as<glm::vec3> ();
	} catch (std::exception &e) {
		(*logstream) << "Can't parse bounding box of the scene: "
								 << e.what () << std::endl;
		return false;
	}

	root.Load (names, streams[1]);

	tessLevel = 1;

	return true;
}

void Geometry::SetTessLevel (GLuint l)
{
	tessLevel = l;
}

GLuint Geometry::GetTessLevel (void)
{
	return tessLevel;
}

Geometry::Node::Node (void)
{
}

Geometry::Node::~Node (void)
{
}

void Geometry::Node::Load (std::map<std::string, GLuint> &names,
													 const YAML::Node &desc)
{
	if (desc["nodes"])
	{
		for (YAML::const_iterator it = desc["nodes"].begin ();
				 it != desc["nodes"].end (); it++)
		{
			children.emplace_back ();
			children.back ().Load (names, *it);
		}
	}
	if (desc["models"])
	{
		for (YAML::const_iterator it = desc["models"].begin ();
				 it != desc["models"].end (); it++)
		{
			models.push_back (names [it->as<std::string> ()]);
		}
	}

	translation = desc["translation"].as<glm::vec3>
		 (glm::vec3 (0.0f, 0.0f, 0.0f));

	if (desc["rotations"])
	{
		for (YAML::const_iterator it = desc["rotations"].begin ();
				 it != desc["rotations"].end (); it++)
		{
			glm::vec3 v;
			GLfloat angle;
			angle = (*it)[0].as<GLfloat> (0.0f);
			v.x = (*it)[1].as<GLfloat> (0.0f);
			v.y = (*it)[2].as<GLfloat> (0.0f);
			v.z = (*it)[3].as<GLfloat> (0.0f);
			if (angle != 0)
			{
				orientation = glm::rotate (orientation, angle, v);
				orientation = glm::normalize (orientation);
			}
		}
	}
}

void Geometry::Node::Render (Geometry *geometry,
														 glm::mat4 parentmvmat,
														 glm::mat3 rotation)
{
	glm::mat4 mvmat = glm::translate (parentmvmat,
																		translation)
		 * glm::mat4 (glm::mat3_cast (orientation));

	rotation *= glm::mat3_cast (orientation);

	for (Node &node : children)
	{
		node.Render (geometry, mvmat, rotation);
	}

	for (GLuint &model : models)
	{
		geometry->Render (model, mvmat, rotation);
	}

}

void Geometry::SetProjMatrix (const glm::mat4 &projmat)
{
			bboxprogram["projmat"] = projmat;
}

void Geometry::Render (GLuint p,
											 const gl::Program &prog,
											 const glm::mat4 &viewmat)
{
	program = &prog;
	switch (p & Pass::Mask)
	{
	case Pass::GBufferTess:
		prog["tessLevel"] = tessLevel;
		sampler.Bind (4);
	case Pass::GBuffer:
	case Pass::GBufferTransparency:
		sampler.Bind (0);
		sampler.Bind (1);
		sampler.Bind (2);
		sampler.Bind (3);
		break;
	}

	pass = p;
	root.Render (this, viewmat, glm::mat3 (1));
}

void Geometry::Render (GLuint model, glm::mat4 &mvmat,
											 glm::mat3 &rotation)
{
	(*program)["mvmat"] = mvmat;
	(*program)["normalmat"] = rotation;
	bboxprogram["mvmat"] = mvmat;
	r->culling.SetModelViewMatrix (mvmat);
	models[model].Render (pass, *program);
	pass++;
}
