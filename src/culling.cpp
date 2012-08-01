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
#include "culling.h"
#include "renderer.h"

Culling::Culling (void)
{
}

Culling::~Culling (void)
{
}

void Culling::Frame (void)
{
	projmat = mvmat = glm::mat4 (1.0f);
	culled = 0;
}

void Culling::SetProjMatrix (const glm::mat4 &mat)
{
	projmat = mat;
	r->geometry.SetProjMatrix (mat);
}

const glm::mat4 &Culling::GetProjMatrix (void)
{
	return projmat;
}

void Culling::SetModelViewMatrix (const glm::mat4 &mat)
{
	mvmat = mat;
}

const glm::mat4 &Culling::GetModelViewMatrix (void)
{
	return mvmat;
}

bool Culling::IsVisible (const glm::vec3 &center, float radius)
{
	glm::mat4 mvpmat;
	glm::vec4 left_plane, right_plane, bottom_plane,
		 top_plane, near_plane, far_plane;
	float distance;

	mvpmat = projmat * mvmat;

	left_plane.x = mvpmat[0].w + mvpmat[0].x;
	left_plane.y = mvpmat[1].w + mvpmat[1].x;
	left_plane.z = mvpmat[2].w + mvpmat[2].x;
	left_plane.w = mvpmat[3].w + mvpmat[3].x;
	left_plane /= glm::length (glm::vec3 (left_plane));

	distance = left_plane.x * center.x + left_plane.y * center.y
		 + left_plane.z * center.z + left_plane.w;
	if (distance <= -radius)
	{
		culled++;
		return false;
	}

	right_plane.x = mvpmat[0].w - mvpmat[0].x;
	right_plane.y = mvpmat[1].w - mvpmat[1].x;
	right_plane.z = mvpmat[2].w - mvpmat[2].x;
	right_plane.w = mvpmat[3].w - mvpmat[3].x;
	right_plane /= glm::length (glm::vec3 (right_plane));

	distance = right_plane.x * center.x + right_plane.y * center.y
		 + right_plane.z * center.z + right_plane.w;
	if (distance <= -radius)
	{
		culled++;
		return false;
	}

	bottom_plane.x = mvpmat[0].w + mvpmat[0].y;
	bottom_plane.y = mvpmat[1].w + mvpmat[1].y;
	bottom_plane.z = mvpmat[2].w + mvpmat[2].y;
	bottom_plane.w = mvpmat[3].w + mvpmat[3].y;
	bottom_plane /= glm::length (glm::vec3 (bottom_plane));

	distance = bottom_plane.x * center.x + bottom_plane.y * center.y
		 + bottom_plane.z * center.z + bottom_plane.w;
	if (distance <= -radius)
	{
		culled++;
		return false;
	}

	top_plane.x = mvpmat[0].w - mvpmat[0].y;
	top_plane.y = mvpmat[1].w - mvpmat[1].y;
	top_plane.z = mvpmat[2].w - mvpmat[2].y;
	top_plane.w = mvpmat[3].w - mvpmat[3].y;
	top_plane /= glm::length (glm::vec3 (top_plane));

	distance = top_plane.x * center.x + top_plane.y * center.y
		 + top_plane.z * center.z + top_plane.w;
	if (distance <= -radius)
	{
		culled++;
		return false;
	}

	near_plane.x = mvpmat[0].w + mvpmat[0].z;
	near_plane.y = mvpmat[1].w + mvpmat[1].z;
	near_plane.z = mvpmat[2].w + mvpmat[2].z;
	near_plane.w = mvpmat[3].w + mvpmat[3].z;
	near_plane /= glm::length (glm::vec3 (near_plane));

	distance = near_plane.x * center.x + near_plane.y * center.y
		 + near_plane.z * center.z + near_plane.w;
	if (distance <= -radius)
	{
		culled++;
		return false;
	}

	far_plane.x = mvpmat[0].w - mvpmat[0].z;
	far_plane.y = mvpmat[1].w - mvpmat[1].z;
	far_plane.z = mvpmat[2].w - mvpmat[2].z;
	far_plane.w = mvpmat[3].w - mvpmat[3].z;
	far_plane /= glm::length (glm::vec3 (far_plane));
	
	distance = far_plane.x * center.x + far_plane.y * center.y
		 + far_plane.z * center.z + far_plane.w;
	if (distance <= -radius)
	{
		culled++;
		return false;
	}

	return true;
}
