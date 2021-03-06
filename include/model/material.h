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
#ifndef MATERIAL_H
#define MATERIAL_H

#include <common.h>
#include <oglp/oglp.h>

class Material
{
public:
	 Material (void);
	 Material (Material &&material);
	 Material (const Material&) = delete;
	 ~Material (void);
	 Material &operator= (Material &&material);
	 Material &operator= (const Material&) = delete;
	 void Use (const gl::Program &program) const;
	 bool IsTransparent (void) const;
	 bool IsDoubleSided (void) const;
private:
	 bool Load (const std::string &name);
	 gl::Texture diffuse;
	 bool diffuse_enabled;
	 gl::Texture normalmap;
	 bool normalmap_enabled;
	 gl::Texture specularmap;
	 bool specularmap_enabled;
	 gl::Texture parametermap;
	 bool parametermap_enabled;
	 gl::Texture heightmap;
	 bool heightmap_enabled;
	 gl::Texture displacementmap;
	 bool displacementmap_enabled;
	 bool transparent;
	 bool doublesided;
	 friend class Scene;
	 friend class Geometry;
};

#endif /* !defined MATERIAL_H */
