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
#ifndef GL_FREETYPE_H
#define GL_FREETYPE_H

#include <common.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_RENDER_H
#include FT_BITMAP_H

namespace gl {

class Font;

/** Freetype class.
 * A wrapper around an FT_Library object
 */
class Freetype
{
public:
	 /** Default constructor.
		* Creates a new Freetype object.
		*/
	 Freetype (void);
	 /** Deleted copy constructor.
		* Freetype objects can't be copy constructed.
		*/
	 Freetype (const Freetype&) = delete;
	 /** Destructor.
		* Deletes the Freetype object.
		*/
	 ~Freetype (void);
	 /** Deleted copy assignment.
		* Freetype objects can't be copy assigned.
		*/
	 Freetype &operator= (const Freetype &) = delete;
	 /** Initialize Freetype.
		* Initializes the font rendering engine.
		* \return Whether the font rendering engine was sucessfully initialized.
		*/
	 bool Init (void);
	 /** Load a Font.
		* Load a font with the specified name.
		* \param Font Font object to store the font to.
		* \param name Name of the font to load.
		* \return Whether the Font was loaded successfully.
		*/
	 bool Load (Font &font, const std::string &name);
private:
	 /** Private rendering command for a character.
		*/
	 void Render (const glm::vec2 &texfactor,
								const glm::vec2 &sizefactor,
								const glm::mat4 &mat,
								const glm::vec3 &color);
	 /** internal FT_Library object
		*/
	 FT_Library library;

	 /** vertex shader program used to render text
		*/
	 gl::Program vshader;
	 /** fragment shader program used to render text
		*/
	 gl::Program fshader;
	 /** shader pipeline used to render text
		*/
	 gl::ProgramPipeline pipeline;
	 /** vertex buffer used for rendering text
		*/
	 gl::Buffer buffer;
	 /** sampler object used for rendering text
		*/
	 gl::Sampler sampler;
	 /** vertex array object used for rendering text
		*/
	 gl::VertexArray vertexarray;
	 friend class Font;
};

} /* namespace gl */

#endif /* !defined GL_FREETYPE_H */
