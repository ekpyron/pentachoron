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
#include "camera.h"
#include "renderer.h"

typedef struct bufferdata {
	 glm::mat4x4 projmat;
	 glm::mat4x4 vmat;
	 glm::mat4x4 vmatinv;
	 glm::vec4 projinfo;
	 glm::uvec2 viewport;
	 glm::vec2 invviewport;
	 glm::vec4 center;
	 glm::vec4 eye;
	 float farClipPlane;
	 float nearClipPlane;
} bufferdata_t;

Camera::Camera (void)
	: center (0, 0, 0), horizontal_angle (0), viewport (glm::ivec2 (0, 0)),
		nearClipPlane (0.1f), farClipPlane (100.0f), up_angle (0)

{
}

Camera::~Camera (void)
{
}

bool Camera::Init (void)
{
	buffer.Data (sizeof (bufferdata_t), NULL, GL_DYNAMIC_DRAW);
	GenerateBuffer ();
	return true;
}

void Camera::Frame (float timefactor)
{
	glm::quat rot;
	rot = glm::rotate (rot, horizontal_angle, glm::vec3 (0, 1, 0));
	rot = glm::rotate (rot, up_angle, glm::vec3 (1, 0, 0));
	vmat = glm::lookAt (GetEye (), center, rot * glm::vec3 (0, 1, 0));
}

void Camera::Resize (int w, int h)
{
	viewport.x = w;
	viewport.y = h;

	projmat = glm::perspective (45.0f, float (viewport.x) / float (viewport.y),
															nearClipPlane, farClipPlane);

	GenerateBuffer ();
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
	GenerateBuffer ();
}

glm::vec3 Camera::GetDirection (void) const
{
	glm::quat rot;
	glm::vec3 dir (0, 0, 1);
	rot = glm::rotate (rot, horizontal_angle, glm::vec3 (0, 1, 0));
	rot = glm::rotate (rot, up_angle, glm::vec3 (1, 0, 0));
	dir = rot * dir;
	return glm::normalize (dir);
}

glm::vec4 Camera::GetProjInfo (void) const
{
	return glm::vec4 (1.0f / projmat[0].x, 1.0f / projmat[1].y,
										nearClipPlane, farClipPlane);
}

void Camera::MoveForward (float distance)
{
	center += GetDirection () * distance;
	GenerateBuffer ();
}

void Camera::MoveUp (float distance)
{
	center += distance * glm::vec3 (0, 1, 0);
	GenerateBuffer ();
}

void Camera::RotateUp (float angle)
{
	up_angle += angle;
	GenerateBuffer ();
}

const gl::Buffer &Camera::GetBuffer (void) const
{
	return buffer;
}

void Camera::GenerateBuffer (void)
{
	bufferdata_t *data;

	buffer.InvalidateData ();
	data = reinterpret_cast<bufferdata_t*> (buffer.Map (GL_WRITE_ONLY));

	data->viewport = viewport;
	data->invviewport = glm::vec2 (1.0f / float (viewport.x),
																1.0f / float (viewport.y));
	data->farClipPlane = farClipPlane;
	data->nearClipPlane = nearClipPlane;
	data->projmat = projmat;
	glm::quat rot;
	rot = glm::rotate (rot, horizontal_angle, glm::vec3 (0, 1, 0));
	rot = glm::rotate (rot, up_angle, glm::vec3 (1, 0, 0));
	data->vmat = glm::lookAt (GetEye (), center,
														rot * glm::vec3 (0, 1, 0));
	data->vmatinv = glm::inverse (data->vmat);
	data->projinfo = GetProjInfo ();
	data->center = glm::vec4 (center, 0.0f);
	data->eye = glm::vec4 (GetEye (), 0.0f);

	buffer.Unmap ();
}
