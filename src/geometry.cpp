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

Geometry::Geometry (Renderer *parent)
	: renderer (parent), kitty (this), headglass (this),
		box (this), grid (this)
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

	if (!grid.Load ("grid.yaml"))
			return false;
	if (!kitty.Load ("kitty.yaml"))
		 return false;
	if (!headglass.Load ("headglass.yaml"))
		 return false;
	if (!box.Load ("box.yaml"))
		 return false;

	return true;
}

void Geometry::Render (GLuint p,
											 const gl::Program &program,
											 const glm::mat4 &viewmat,
											 bool shadowpass, bool transparent)
{
	if (!shadowpass)
	{
		sampler.Bind (0);
		sampler.Bind (1);
		sampler.Bind (2);
	}

	GLuint pass = p;

	int x = 0, z = 0;
	for (int z = -3; z <= 3; z++)
	{
		for (int x = -3; x <= 3; x++)
		{
			glm::mat4 mvmat = glm::translate (viewmat,
																				glm::vec3 (5 * x, -2.99, 8 * z));
			program["mvmat"] = mvmat;
			bboxprogram["mvmat"] = mvmat;
			renderer->culling.SetModelViewMatrix (mvmat);
			bboxprogram["projmat"] = renderer->culling.GetProjMatrix ();
			if ((x&1) + (z&1) == 0)
			{
				kitty.Render (pass, program, shadowpass, transparent);
				headglass.Render (pass, program, shadowpass, transparent);
			}
			else
			{
				box.Render (pass, program, shadowpass, transparent);
			}
			pass++;
		}
	}

	program["mvmat"] = viewmat;
	bboxprogram["mvmat"] = viewmat;
	renderer->culling.SetModelViewMatrix (viewmat);

	grid.Render (pass, program, shadowpass, transparent);
}
