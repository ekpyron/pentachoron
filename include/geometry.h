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
#include "model/model.h"
#include "model/material.h"
#include <map>

class Renderer;

class Geometry
{
public:
	 Geometry (Renderer *parent);
	 ~Geometry (void);
	 bool Init (void);
	 void Render (GLuint pass, const gl::Program &program,
								const glm::mat4 &viewmat);
	 const Material &GetMaterial (const std::string &name);

	 class Pass
	 {
		 public:
		 static constexpr GLuint GBuffer = 0x00000000;
		 static constexpr GLuint GBufferTransparency = 0x10000000;
		 static constexpr GLuint GBufferSRAA = 0x20000000;
		 static constexpr GLuint ShadowMap = 0x30000000;
		 static constexpr GLuint Mask = 0xF0000000;
	 };

private:
	 Model kitty;
	 Model headglass;
	 Model box;
	 Model grid;
	 gl::Sampler sampler;
	 std::map<std::string, Material*> materials;

	 gl::Program bboxprogram;

	 friend class Model;
	 friend class Mesh;
	 friend class GBuffer;
	 Renderer *renderer;
};


#endif /* !defined GEOMETRY_H */
