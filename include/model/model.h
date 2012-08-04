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
#ifndef MODEL_H
#define MODEL_H

#include <common.h>
#include "mesh.h"
#include "material.h"
class Geometry;

class Model
{
public:
	 Model (void);
	 Model (Model &&model);
	 Model (const Model&) = delete;
	 ~Model (void);
	 Model &operator= (Model &&model);
	 Model &operator= (const Model&) = delete;
	 bool Load (const std::string &filename);
	 void Render (GLuint pass, const gl::Program &program);
	 static GLuint culled;
private:
	 std::vector<Material> materials;
	 std::vector<Mesh> meshes;
	 std::vector<Mesh> patches;
	 std::vector<Mesh> transparent;
	 friend class Material;
	 friend class Mesh;
	 struct
	 {
			glm::vec3 min, max;
			gl::Buffer buffer;
			gl::Buffer indices;
			gl::VertexArray array;
	 } bbox;
	 struct
	 {
			glm::vec3 center;
			GLfloat radius;
	 } bsphere;
	 std::map<GLuint, gl::Query> queries;
#ifdef DEBUG
	 struct
	 {
			std::string filename;
	 } debug;
#endif /* DEBUG */
};

#endif /* !defined MODEL_H */
