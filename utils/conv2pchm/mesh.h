/*  
 * This file is part of conv2pchm.
 *
 * conv2pchm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * conv2pchm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with conv2pchm.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MESH_H
#define MESH_H

#include "common.h"

class LogStream : public Assimp::LogStream
{
public:
	 LogStream (void);
	 ~LogStream (void);
	 void write (const char *msg);
	 const std::string &get (void);
	 void clear (void);
private:
	 std::string str;
};

class Mesh
{
public:
	 Mesh (void);
	 ~Mesh (void);
	 bool Load (aiMesh *mesh, GLuint e);
	 void Sanitize (void);
	 std::vector<GLuint> indices;
	 std::vector<GLfloat> vertices;
	 std::vector<GLfloat> normals;
	 std::vector<GLfloat> texcoords;
	 std::vector<GLfloat> tangents;
	 GLuint edges;
	 glm::vec3 min;
	 glm::vec3 max;
};

bool LoadMeshes (const char *filename, GLuint edges);

#endif /* !defined MESH_H */
