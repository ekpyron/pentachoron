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

class Model;
class Material;

class Mesh
{
public:
	 Mesh (Model &scene);
	 Mesh (Mesh &&mesh);
	 Mesh (const Mesh&) = delete;
	 ~Mesh (void);
	 Mesh &operator= (Mesh &&mesh);
	 Mesh &operator= (const Mesh&) = delete;
	 void Render (const gl::Program &program, GLuint passtype);
	 bool IsTransparent (void) const;
	 bool IsTessellated (void) const;
	 bool IsShadowTessellated (void) const;
	 bool IsDoubleSided (void) const;
	 static GLuint culled;
private:
	 bool shadows;
	 bool tessellated;
	 bool shadowtessellated;
	 bool Load (const std::string &filename, const Material *mat,
							glm::vec3 &min, glm::vec3 &max,
							bool shadows, bool tessellated=false,
							bool shadowtessellated=false);
	 Model &parent;
	 friend class Model;
	 const Material *material;

	 struct
	 {
			glm::vec3 center;
			float radius;
	 } bsphere;

	 gl::VertexArray vertexarray, depthonlyarray;
	 GLuint facecount;
	 GLuint edgesperface;
	 GLuint vertexcount;
	 std::vector<gl::Buffer> buffers;
	 gl::Buffer indices;
};

#endif /* !defined MESH_H */
