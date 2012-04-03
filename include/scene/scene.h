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
#ifndef SCENE_H
#define SCENE_H

#include <common.h>
#include "mesh.h"
#include "material.h"
class Geometry;

class Scene
{
public:
	 Scene (Geometry *p);
	 Scene (Scene &&scene);
	 Scene (const Scene&) = delete;
	 ~Scene (void);
	 Scene &operator= (Scene &&scene);
	 Scene &operator= (const Scene&) = delete;
	 bool Load (const std::string &filename);
	 void Render (GLuint pass, const gl::Program &program, bool shadowpass);
private:
	 std::vector<Material> materials;
	 std::vector<Mesh> meshes;
	 friend class Material;
	 friend class Mesh;
	 Geometry *parent;
#ifdef DEBUG
	 struct
	 {
			std::string filename;
	 } debug;
#endif /* DEBUG */
};

#endif /* !defined SCENE_H */
