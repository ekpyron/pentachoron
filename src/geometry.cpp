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
	: renderer (parent), kitty (this), box (this), grid (this)
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
	sampler.Parameter (GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	sampler.Parameter (GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	sampler.Parameter (GL_TEXTURE_WRAP_S, GL_REPEAT);
	sampler.Parameter (GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (!grid.Load ("grid.yaml"))
			return false;
	if (!kitty.Load ("kitty.yaml"))
		 return false;
	if (!box.Load ("box.yaml"))
		 return false;

	return true;
}

void Geometry::Render (const gl::Program &program, const glm::mat4 &viewmat,
											 bool shadowpass) const
{
	if (!shadowpass)
	{
		sampler.Bind (0);
		sampler.Bind (1);
	}

	for (int z = -3; z <= 3; z++)
	{
		for (int x = -3; x <= 3; x++)
		{
			program["mvmat"] = glm::translate (viewmat,
																				 glm::vec3 (5 * x, 0, 8 * z));
			if ((x&1) + (z&1) == 0)
			{
				kitty.Render (program, shadowpass);
			}
			else
			{
				box.Render (program, shadowpass);
			}
		}
	}

	program["mvmat"] = viewmat;

	grid.Render (program, shadowpass);
}

void Geometry::RenderOpaque (const gl::Program &program,
														 const glm::mat4 &viewmat) const
{
	sampler.Bind (0);
	sampler.Bind (1);

	for (int z = -3; z <= 3; z++)
	{
		for (int x = -3; x <= 3; x++)
		{
			program["mvmat"] = glm::translate (viewmat,
																				 glm::vec3 (5 * x, 0, 8 * z));
			if ((x&1) + (z&1) == 0)
			{
				kitty.RenderOpaque (program);
			}
			else
			{
				box.RenderOpaque (program);
			}
		}
	}

	program["mvmat"] = viewmat;

	grid.RenderOpaque (program);
}
