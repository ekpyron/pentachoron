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
#include "camera.h"
#include "renderer.h"

Camera::Camera (void)
	: center (0, 0, 0), horizontal_angle (0), viewport (glm::ivec2 (0, 0)),
		nearClipPlane (0.1f), farClipPlane (100.0f), up_angle (0)

{
}

Camera::~Camera (void)
{
}

void Camera::Frame (float timefactor)
{
	vmat = glm::lookAt (GetEye (), center, glm::vec3 (0, 1, 0));
}

void Camera::Resize (int w, int h)
{
	viewport.x = w;
	viewport.y = h;

	projmat = glm::perspective (45.0f, float (viewport.x) / float (viewport.y),
															nearClipPlane, farClipPlane);
	r->gbuffer.SetProjMatrix (projmat);
}

glm::vec3 Camera::GetEye (void) const
{
	return center - GetDirection () * 1.0f;
}

glm::vec3 Camera::GetCenter (void) const
{
	return center;
}

glm::mat4 Camera::GetViewMatrix (void) const
{
	return vmat;
}

glm::mat4 Camera::GetProjMatrix (void) const
{
	return projmat;
}

glm::uvec2 Camera::GetViewport (void) const
{
	return viewport;
}

int Camera::GetViewportWidth (void) const
{
	return viewport.x;
}

int Camera::GetViewportHeight (void) const
{
	return viewport.y;
}

float Camera::GetNearClipPlane (void) const
{
	return nearClipPlane;
}

float Camera::GetFarClipPlane (void) const
{
	return farClipPlane;
}

void Camera::RotateY (float angle)
{
	horizontal_angle += angle;
}

glm::vec3 Camera::GetDirection (void) const
{
	glm::vec3 dir (sin (horizontal_angle), sin (up_angle),
								 cos (horizontal_angle));
	return glm::normalize (dir);
}

glm::vec4 Camera::GetProjInfo (void) const
{
	return glm::vec4 (1.0f / projmat[0].x, 1.0f / projmat[1].y,
										nearClipPlane, farClipPlane);
}

void Camera::MoveForward (float distance)
{
	glm::vec3 dir (sin (horizontal_angle), 0, cos (horizontal_angle));
	center += dir * distance;
}

void Camera::MoveUp (float distance)
{
	center += distance * glm::vec3 (0, 1, 0);
}

void Camera::RotateUp (float angle)
{
	up_angle += angle;
}
