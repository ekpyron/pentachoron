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
#include "light.h"

void Light::CalculateFrustum (void)
{
	glm::mat4 mvpmat;

	mvpmat = glm::perspective (2 * spot.angle * 180.0f / float (PCH_PI),
														 1.0f, 0.1f, attenuation.w);
	mvpmat *= glm::lookAt (glm::vec3 (position),
												 glm::vec3 (position)
												 + glm::vec3 (direction),
												 glm::vec3 (1, 0, 0));

	frustum.l.x = mvpmat[0].w + mvpmat[0].x;
	frustum.l.y = mvpmat[1].w + mvpmat[1].x;
	frustum.l.z = mvpmat[2].w + mvpmat[2].x;
	frustum.l.w = mvpmat[3].w + mvpmat[3].x;
	frustum.l /= glm::length (glm::vec3 (frustum.l));

	frustum.r.x = mvpmat[0].w - mvpmat[0].x;
	frustum.r.y = mvpmat[1].w - mvpmat[1].x;
	frustum.r.z = mvpmat[2].w - mvpmat[2].x;
	frustum.r.w = mvpmat[3].w - mvpmat[3].x;
	frustum.r /= glm::length (glm::vec3 (frustum.r));

	frustum.b.x = mvpmat[0].w + mvpmat[0].y;
	frustum.b.y = mvpmat[1].w + mvpmat[1].y;
	frustum.b.z = mvpmat[2].w + mvpmat[2].y;
	frustum.b.w = mvpmat[3].w + mvpmat[3].y;
	frustum.b /= glm::length (glm::vec3 (frustum.b));

	frustum.t.x = mvpmat[0].w - mvpmat[0].y;
	frustum.t.y = mvpmat[1].w - mvpmat[1].y;
	frustum.t.z = mvpmat[2].w - mvpmat[2].y;
	frustum.t.w = mvpmat[3].w - mvpmat[3].y;
	frustum.t /= glm::length (glm::vec3 (frustum.t));

	frustum.n.x = mvpmat[0].w + mvpmat[0].z;
	frustum.n.y = mvpmat[1].w + mvpmat[1].z;
	frustum.n.z = mvpmat[2].w + mvpmat[2].z;
	frustum.n.w = mvpmat[3].w + mvpmat[3].z;
	frustum.n /= glm::length (glm::vec3 (frustum.n));

	frustum.f.x = mvpmat[0].w - mvpmat[0].z;
	frustum.f.y = mvpmat[1].w - mvpmat[1].z;
	frustum.f.z = mvpmat[2].w - mvpmat[2].z;
	frustum.f.w = mvpmat[3].w - mvpmat[3].z;
	frustum.f /= glm::length (glm::vec3 (frustum.f));

}
