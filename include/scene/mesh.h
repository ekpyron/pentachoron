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
#ifndef MESH_H
#define MESH_H

#include <common.h>
#include <oglp/oglp.h>

class Scene;
class Material;

class Mesh
{
public:
	 Mesh (Scene &scene);
	 Mesh (Mesh &&mesh);
	 Mesh (const Mesh&) = delete;
	 ~Mesh (void);
	 Mesh &operator= (Mesh &&mesh);
	 Mesh &operator= (const Mesh&) = delete;
	 void Render (const gl::Program &program, bool shadowpass) const;
private:
	 bool Load (void*, const Material *mat);
	 Scene &parent;
	 friend class Scene;
	 const Material *material;

	 gl::VertexArray vertexarray, shadowpassarray;
	 GLuint trianglecount;
	 GLuint vertexcount;
	 std::vector<gl::Buffer> buffers;
	 gl::Buffer indices;
};

#endif /* !defined MESH_H */
