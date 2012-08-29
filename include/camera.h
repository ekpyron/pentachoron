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
#ifndef CAMERA_H
#define CAMERA_H

#include <common.h>

/** Camera class.
 * Handles the camera.
 */
class Camera
{
public:
	/** Constructor.
	 */
	 Camera (void);
	 /** Destructor.
		*/
	 ~Camera (void);
   /** Initialization.
		* Initialize the camera.
		* \returns Whether the initialization was successful.
		*/
	 bool Init (void);
	 /** Per-frame subroutine.
		* All per-frame camera operations are done here.
		* \param timefactor Fraction of seconds since the last frame.
		*/
	 void Frame (float timefactor);
	 /** Handling window resize.
		* Adjustments necessary after a window resize are made here.
		* \param w New viewport width.
		* \param h New viewport height
		*/
	 void Resize (int w, int h);
	 /** Return eye vector.
		* Returns the eye vector, pointing from the camera to the center
		* of the scene.
		* \returns The current eye vector.
		*/
	 glm::vec3 GetEye (void) const;
	 /** Return center of scene.
		* Returns a vector pointing to the center of the scene.
		* \returns The center of the scene.
		*/
	 glm::vec3 GetCenter (void) const;
	 /** Return view matrix.
		* Returns the current view matrix.
		* \returns The current view matrix.
		*/
	 glm::mat4x4 GetViewMatrix (void) const;
	 /** Return projection matrix.
		* Returns the current projection matrix.
		* \returns The current projection matrix.
		*/
	 glm::mat4x4 GetProjMatrix (void) const;
	 /** Return viewport dimensions.
		* Returns the dimensions of the current viewport.
		* \returns The viewport dimensions.
		*/
	 glm::uvec2 GetViewport (void) const;
	 /** Return viewport width.
		* Returns the width of the current viewport.
		* \returns The width of the current viewport.
		*/
	 int GetViewportWidth (void) const;
	 /** Return viewport height.
		* Returns the height of the current viewport.
		* \returns The height of the current viewport.
		*/
	 int GetViewportHeight (void) const;
	 /** Return near clipping plane.
		* Returns the distance of the near clipping plane.
		* \returns The near clipping plane.
		*/
	 float GetNearClipPlane (void) const;
	 /** Return far clipping plane.
		* Returns the distance of the far clipping plane.
		* \returns The far clipping plane.
		*/
	 float GetFarClipPlane (void) const;
	 /** Return projection information.
		* Returns a vector containing information about the current
		* projection used to reconstruct eye coordinates
		* from the per pixel depth.
		* \returns A vector containing projection information.
		*/
	 glm::vec4 GetProjInfo (void) const;
	 /** Rotate camera around y.
		* Rotates the camera around the y-axis.
		* \param angle Angle around which to rotate.
		*/
	 void RotateY (float angle);
	 /** Rotate camera up/down.
		* Rotates the camera up/down.
		* \param angle Angle around which to rotate.
		*/
	 void RotateUp (float angle);
	 /** Get camera direction.
		* Obtains the direction vector of the camera.
		* \returns The camera direction.
		*/
	 glm::vec3 GetDirection (void) const;
	 /** Move forward.
		* Moves the camera forward.
		* \param distance Distance to move the camera.
		*/
	 void MoveForward (float distance);
	 /** Move upwards.
		* Moves the camera upwards.
		* \param distance Distance to move the camera.
		*/
	 void MoveUp (float distance);
	 /** Get uniform buffer.
		* Obtain an uniform buffer containing camera information.
		*/
	 const gl::Buffer &GetBuffer (void) const;
private:
	 /** Generate uniform buffer.
		* Generates a uniform buffer containing camera information.
		*/
	 void GenerateBuffer (void);
	 /** Viewport dimension.
		* Stores the dimensions of the viewport.
		*/
	 glm::uvec2 viewport;
	 /** Far clipping plane.
		* Stores the distance of the far clipping plane.
		*/
	 float farClipPlane;
	 /** Near clipping plane.
		* Stores the distance of the near clipping plane.
		*/
	 float nearClipPlane;
	 /** Projection matrix.
		* Stores the projection matrix.
		*/
	 glm::mat4x4 projmat;
	 /** View matrix.
		* Stores the view matrix.
		*/
	 glm::mat4x4 vmat;
	 /** Center of scene.
		* Stores the center of the scene
		*/
	 glm::vec3 center;
	 /** Horizontal view angle.
		* Stores the horizontal camera rotation.
		*/
	 float horizontal_angle;
	 /** Up view angle.
		* Stores the vertical camera rotation.
		*/
	 float up_angle;
	 /** Buffer.
		* Uniform buffer storing camera infomation.
		*/
	 gl::Buffer buffer;
};

#endif /* !defined CAMERA_H */
