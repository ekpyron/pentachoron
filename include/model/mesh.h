/*
 * This file is part of Pentachoron.
 *
 * Pentachoron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pentachoron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pentachoron.  If not, see <http://www.gnu.org/licenses/>.
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
	 Mesh (Model &model);
	 Mesh (Mesh &&mesh);
	 Mesh (const Mesh&) = delete;
	 ~Mesh (void);
	 Mesh &operator= (Mesh &&mesh);
	 Mesh &operator= (const Mesh&) = delete;
	 void Render (const gl::Program &program,
								bool depthonly = false,
								bool quads = true) const;
	 bool Load (const std::string &filename,
							const Material *mat,
							glm::vec3 &min,
							glm::vec3 &max,
							bool cast_shadows);
	 bool CastsShadow (void) const;
	 bool IsTransparent (void) const;
	 bool IsTessellated (void) const;
	 static GLuint culled;
private:

	 bool LoadTriangles (std::ifstream &file,
											 unsigned int num_texcoords,
											 glm::vec3 &min,
											 glm::vec3 &max);
	 bool LoadTrianglePatches (std::ifstream &file,
														 unsigned int num_texcoords,
														 glm::vec3 &min,
														 glm::vec3 &max);
	 bool LoadQuadPatches (std::ifstream &file,
												 unsigned int num_texcoords,
												 glm::vec3 &min,
												 glm::vec3 &max);

	 const Material *material;
	 bool shadows;

	 struct
	 {
			glm::vec3 center;
			float radius;
	 } bsphere;

	 bool patches;

	 Model &parent;

	 gl::VertexArray vertexarray, depthonlyarray;
	 GLuint trianglecount;
	 GLuint quadcount;
	 GLuint vertexcount;
	 std::vector<gl::Buffer> buffers;
	 gl::Buffer triangleindices;
	 gl::Buffer quadindices;
};

#endif /* !defined MESH_H */
