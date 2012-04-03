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
	 void Render (GLuint pass, const gl::Program &program, bool shadowpass);
	 bool IsTransparent (void) const;
	 static GLuint culled;
private:
	 bool Load (void*, const Material *mat);
	 Scene &parent;
	 friend class Scene;
	 const Material *material;

	 struct
	 {
			glm::vec3 center;
			float radius;
	 } bsphere;

	 gl::VertexArray vertexarray, shadowpassarray;
	 GLuint trianglecount;
	 GLuint vertexcount;
	 std::vector<gl::Buffer> buffers;
	 gl::Buffer indices;
	 struct
	 {
			glm::vec3 min, max;
			gl::Buffer buffer;
			gl::Buffer indices;
			gl::VertexArray array;
	 } bbox;
	 std::map<GLuint, gl::Query> queries;
};

#endif /* !defined MESH_H */
