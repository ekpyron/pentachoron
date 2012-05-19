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
#ifndef CULLING_H
#define CULLING_H

#include <common.h>

class Renderer;

/** Culling class.
 * This class handles frustum culling.
 */
class Culling
{
public:
	 /** Constructor.
		*/
	 Culling (Renderer *parent);
	 /** Destructor
		*/
	 ~Culling (void);
	 /** Visibility query.
		* Checks whether a given bounding sphere intersects with the visible
		* view frustum.
		* \param center Center of the bounding sphere.
		* \param radius Radius of the bounding sphere.
		* \returns The visibility of the bounding sphere.
		*/
	 bool IsVisible (const glm::vec3 &center, float radius);
	 /** Set the projection matrix.
		* Sets the projection matrix used to do the culling calculations.
		* \param mat The projection matrix to use.
		*/
	 void SetProjMatrix (const glm::mat4 &mat);
	 /** Get the projection matrix.
		* Obtainss the projection matrix currently used for
		* the culling calculations.
		* \returns The projection matrix currently in use by this class.
		*/
	 const glm::mat4 &GetProjMatrix (void);
	 /** Set the model view matrix.
		* Sets the model view matrix used to do the culling calculations.
		* \param mat The model view matrix to use.
		*/	 
	 void SetModelViewMatrix (const glm::mat4 &mat);
	 /** Get the model view matrix.
		* Obtainss the model view matrix currently used for
		* the culling calculations.
		* \returns The model view matrix currently in use by this class.
		*/
	 const glm::mat4 &GetModelViewMatrix (void);
	 /** Per-frame initialization.
		* Initializes the culling class every frame.
		*/
	 void Frame (void);
	 /** Object count.
		* Counts the objects culled in this frame.
		*/
	 GLuint culled;
private:
	 /** Projection matrix.
		* Stores the projection matrix used for culling.
		*/
	 glm::mat4 projmat;
	 /** Model view matrix.
		* Stores the model view matrix used for culling.
		*/
	 glm::mat4 mvmat;
	 /** Parnent.
		* Parent Renderer.
		*/
	 Renderer *renderer;
};

#endif /* !defined CULLING_H */
