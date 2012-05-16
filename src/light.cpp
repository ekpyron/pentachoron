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
#include "light.h"

void Light::CalculateFrustum (void)
{
	glm::mat4 mvpmat;

	mvpmat = glm::perspective (2 * spot.angle * 180.0f / float (DRE_PI),
														 1.0f, 0.1f, attenuation.w);
	mvpmat *= glm::lookAt (glm::vec3 (position),
												 glm::vec3 (position)
												 + glm::vec3 (direction),
												 glm::vec3 (1, 0, 0));

	frustum.left.x = mvpmat[0].w + mvpmat[0].x;
	frustum.left.y = mvpmat[1].w + mvpmat[1].x;
	frustum.left.z = mvpmat[2].w + mvpmat[2].x;
	frustum.left.w = mvpmat[3].w + mvpmat[3].x;
	frustum.left /= glm::length (glm::vec3 (frustum.left));

	frustum.right.x = mvpmat[0].w - mvpmat[0].x;
	frustum.right.y = mvpmat[1].w - mvpmat[1].x;
	frustum.right.z = mvpmat[2].w - mvpmat[2].x;
	frustum.right.w = mvpmat[3].w - mvpmat[3].x;
	frustum.right /= glm::length (glm::vec3 (frustum.right));

	frustum.bottom.x = mvpmat[0].w + mvpmat[0].y;
	frustum.bottom.y = mvpmat[1].w + mvpmat[1].y;
	frustum.bottom.z = mvpmat[2].w + mvpmat[2].y;
	frustum.bottom.w = mvpmat[3].w + mvpmat[3].y;
	frustum.bottom /= glm::length (glm::vec3 (frustum.bottom));

	frustum.top.x = mvpmat[0].w - mvpmat[0].y;
	frustum.top.y = mvpmat[1].w - mvpmat[1].y;
	frustum.top.z = mvpmat[2].w - mvpmat[2].y;
	frustum.top.w = mvpmat[3].w - mvpmat[3].y;
	frustum.top /= glm::length (glm::vec3 (frustum.top));

	frustum.near.x = mvpmat[0].w + mvpmat[0].z;
	frustum.near.y = mvpmat[1].w + mvpmat[1].z;
	frustum.near.z = mvpmat[2].w + mvpmat[2].z;
	frustum.near.w = mvpmat[3].w + mvpmat[3].z;
	frustum.near /= glm::length (glm::vec3 (frustum.near));

	frustum.far.x = mvpmat[0].w - mvpmat[0].z;
	frustum.far.y = mvpmat[1].w - mvpmat[1].z;
	frustum.far.z = mvpmat[2].w - mvpmat[2].z;
	frustum.far.w = mvpmat[3].w - mvpmat[3].z;
	frustum.far /= glm::length (glm::vec3 (frustum.far));

}
