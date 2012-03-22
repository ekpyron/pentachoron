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
#ifndef CAMERA_H
#define CAMERA_H

#include <common.h>

class Camera
{
public:
	 Camera (void);
	 ~Camera (void);
	 void Frame (float timefactor);
	 void Resize (int w, int h);
	 glm::vec3 GetEye (void) const;
	 glm::mat4x4 GetViewMatrix (void) const;
	 glm::mat4x4 GetProjMatrix (void) const;
	 glm::uvec2 GetViewport (void) const;
	 int GetViewportWidth (void) const;
	 int GetViewportHeight (void) const;
	 float GetNearClipPlane (void) const;
	 float GetFarClipPlane (void) const;
	 glm::vec4 GetProjInfo (void) const;
private:
	 glm::uvec2 viewport;
	 float farClipPlane;
	 float nearClipPlane;
	 glm::mat4x4 projmat;
	 glm::mat4x4 vmat;
	 glm::vec3 center;
	 float angle;

	 friend class Interface;
};

#endif /* !defined CAMERA_H */
