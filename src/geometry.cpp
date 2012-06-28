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

	YAML::Node scene;
	std::map<std::string, GLuint> names;
	{
		std::ifstream file (MakePath ("scene.yaml"), std::ifstream::in);
		if (!file.is_open ())
		{
			(*logstream) << "Cannot open " << MakePath ("scene.yaml")
									 << "." << std::endl;
			return false;
		}
		scene = YAML::Load (file);
		if (!scene.IsMap ())
		{
			(*logstream) << MakePath ("scene.yaml")
									 << " has an invalid format." << std::endl;
			return false;
		}
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

		root.Load (names, scene["root"]);
	}

/*		YAML::Node nodedesc;
		nodedesc = scene["nodes"];
		for (YAML::const_iterator it = nodedesc.begin ();
				 it != nodedesc.end (); it++)
		{
			Node node;
			// TODO: some more error handling.
			if ((*it)["models"].IsSequence ())
			{
				for (YAML::const_iterator m = (*it)["models"].begin ();
						 it != (*it)["models"].end (); it++)
				{
					node.models.push_back (names [(*it)["model"].as<std::string> ()]);
				}
			}
			node.translation.x = (*it)["translation"][0].as<GLfloat> (0.0f);
			node.translation.y = (*it)["translation"][1].as<GLfloat> (0.0f);
			node.translation.z = (*it)["translation"][2].as<GLfloat> (0.0f);
			glm::vec3 v;
			GLfloat angle;
			angle = (*it)["orientation"][0].as<GLfloat> (0.0f);
			v.x = (*it)["orientation"][1].as<GLfloat> (0.0f);
			v.y = (*it)["orientation"][2].as<GLfloat> (0.0f);
			v.z = (*it)["orientation"][3].as<GLfloat> (0.0f);
			angle *= DRE_PI / 180.0f;
			node.orientation = glm::quat (angle, v);
			glm::normalize (node.orientation);
			nodes.push_back (node);
		}
		}*/
	return true;
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
	if (desc["children"])
	{
		for (YAML::const_iterator it = desc["children"].begin ();
				 it != desc["children"].end (); it++)
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

	translation.x = desc["translation"][0].as<GLfloat> (0.0f);
	translation.y = desc["translation"][1].as<GLfloat> (0.0f);
	translation.z = desc["translation"][2].as<GLfloat> (0.0f);
	glm::vec3 v;
	GLfloat angle;
	angle = desc["orientation"][0].as<GLfloat> (0.0f);
	v.x = desc["orientation"][1].as<GLfloat> (0.0f);
	v.y = desc["orientation"][2].as<GLfloat> (0.0f);
	v.z = desc["orientation"][3].as<GLfloat> (0.0f);
	angle *= DRE_PI / 180.0f;
	orientation = glm::quat (angle, v);
	glm::normalize (orientation);
}

void Geometry::Node::Render (Geometry *geometry,
														 glm::mat4 parentmvmat)
{
	glm::mat4 mvmat = glm::translate (parentmvmat
																		* glm::mat4_cast (orientation),
																		translation);
	for (Node &node : children)
	{
		node.Render (geometry, mvmat);
	}

	for (GLuint &model : models)
	{
		geometry->Render (model, mvmat);
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
	case Pass::GBuffer:
	case Pass::GBufferTransparency:
		sampler.Bind (0);
		sampler.Bind (1);
		sampler.Bind (2);
		break;
	}

	pass = p;
	root.Render (this, viewmat);
}

void Geometry::Render (GLuint model, glm::mat4 mvmat)
{
	(*program)["mvmat"] = mvmat;
	bboxprogram["mvmat"] = mvmat;
	r->culling.SetModelViewMatrix (mvmat);
	models[model].Render (pass, *program);
	pass++;
}
