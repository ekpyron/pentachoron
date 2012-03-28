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
#include "culling.h"

Culling::Culling (void)
{
}

Culling::~Culling (void)
{
}

void Culling::Frame (void)
{
	projmat = mvmat = glm::mat4 (1.0f);
}

void Culling::SetProjMatrix (const glm::mat4 &mat)
{
	projmat = mat;
}

void Culling::SetModelViewMatrix (const glm::mat4 &mat)
{
	mvmat = mat;
}

bool Culling::IsVisible (const glm::vec3 &center, float radius) const
{
	glm::mat4 mvpmat;
	glm::vec4 left, right, bottom, top, near, far;
	float distance;

	mvpmat = projmat * mvmat;

	left.x = mvpmat[0].w + mvpmat[0].x;
	left.y = mvpmat[1].w + mvpmat[1].x;
	left.z = mvpmat[2].w + mvpmat[2].x;
	left.w = mvpmat[3].w + mvpmat[3].x;
	left /= glm::length (glm::vec3 (left));

	distance = left.x * center.x + left.y * center.y
		 + left.z * center.z + left.w;
	if (distance <= -radius)
		 return false;

	right.x = mvpmat[0].w - mvpmat[0].x;
	right.y = mvpmat[1].w - mvpmat[1].x;
	right.z = mvpmat[2].w - mvpmat[2].x;
	right.w = mvpmat[3].w - mvpmat[3].x;
	right /= glm::length (glm::vec3 (right));

	distance = right.x * center.x + right.y * center.y
		 + right.z * center.z + right.w;
	if (distance <= -radius)
		 return false;

	bottom.x = mvpmat[0].w + mvpmat[0].y;
	bottom.y = mvpmat[1].w + mvpmat[1].y;
	bottom.z = mvpmat[2].w + mvpmat[2].y;
	bottom.w = mvpmat[3].w + mvpmat[3].y;
	bottom /= glm::length (glm::vec3 (bottom));

	distance = bottom.x * center.x + bottom.y * center.y
		 + bottom.z * center.z + bottom.w;
	if (distance <= -radius)
		 return false;

	top.x = mvpmat[0].w - mvpmat[0].y;
	top.y = mvpmat[1].w - mvpmat[1].y;
	top.z = mvpmat[2].w - mvpmat[2].y;
	top.w = mvpmat[3].w - mvpmat[3].y;
	top /= glm::length (glm::vec3 (top));

	distance = top.x * center.x + top.y * center.y
		 + top.z * center.z + top.w;
	if (distance <= -radius)
		 return false;

	near.x = mvpmat[0].w + mvpmat[0].z;
	near.y = mvpmat[1].w + mvpmat[1].z;
	near.z = mvpmat[2].w + mvpmat[2].z;
	near.w = mvpmat[3].w + mvpmat[3].z;
	near /= glm::length (glm::vec3 (near));

	distance = near.x * center.x + near.y * center.y
		 + near.z * center.z + near.w;
	if (distance <= -radius)
		 return false;

	far.x = mvpmat[0].w - mvpmat[0].z;
	far.y = mvpmat[1].w - mvpmat[1].z;
	far.z = mvpmat[2].w - mvpmat[2].z;
	far.w = mvpmat[3].w - mvpmat[3].z;
	far /= glm::length (glm::vec3 (far));
	
	distance = far.x * center.x + far.y * center.y
		 + far.z * center.z + far.w;
	if (distance <= -radius)
		 return false;

	return true;
}
