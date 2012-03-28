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
#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <common.h>
#include "scene/scene.h"
#include "scene/material.h"
#include <map>

class Renderer;

class Geometry
{
public:
	 Geometry (Renderer *parent);
	 ~Geometry (void);
	 bool Init (void);
	 void Render (const gl::Program &program,
								const glm::mat4 &viewmat,
								bool shadowpass = false) const;
	 const Material &GetMaterial (const std::string &name);

private:
	 Scene kitty;
	 Scene box;
	 Scene grid;
	 gl::Sampler sampler;
	 std::map<std::string, Material*> materials;

	 friend class Scene;
	 friend class Mesh;
	 Renderer *renderer;
};


#endif /* !defined GEOMETRY_H */
