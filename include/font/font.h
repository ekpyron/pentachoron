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
#ifndef GL_FONT_H
#define GL_FONT_H

#include <common.h>
#include "freetype.h"

namespace gl {

class Freetype;

/** Font class.
 * A Font rendering class.
 */
class Font
{
public:
	 /** Default constructor.
		* Creates a new Font object.
		*/
	 Font (void);
	 /** Move constructor.
		* Moves the Font to another object.
		*/
	 Font (Font &&font);
	 /** Deleted copy constructor.
		* Font objects cannot be copy constructed.
		*/
	 Font (const Font&) = delete;
	 /** Destructor.
		* Deletes the Font object.
		*/
	 ~Font (void);
	 /** Move constructor.
		* Moves the Font to another object.
		*/
	 Font &operator= (Font &&font);
	 /** Deleted copy assignment.
		* Font objects cannot be copy assigned.
		*/
	 Font &operator= (const Font &) = delete;
	 /** Render a character.
		* Renders the specified character to the screen.
		* \param c Character to render.
		* \return Whether the character was rendered successfully.
		*/
	 bool PutChar (char c);
	 /** Render a string.
		* Renders the specified string to the screen.
		* \param str Specifies the string to render.
		* \return Whether the string was rendered successfully.
		*/
	 bool PrintStr (const std::string &str);
	 template<typename T, typename... Args>
	 inline bool Print (T v, Args... args);
	 template<typename... Args>
	 inline bool Print (glm::vec3 v, Args... args);
	 inline bool Print (void);
	 /** Per-Frame initialization of the font renderer.
		*/
	 void Frame (void);
	 void SetColor (const glm::vec3 &color);
private:
	 /** Private Font loading.
		* Used by the Freetype class to load a Font.
		* \param library The FT_Library object contained in the Freetype class.
		* \param name The Name of the font to load.
		* \return Whether the Font was loaded successfully.
		*/
	 bool Load (Freetype *freetype, const std::string &name);
	 glm::vec2 pos;
	 glm::vec3 color;
	 struct
	 {
			gl::Texture texture;
			float left, top, advance;
			GLuint width, height;
			GLuint texwidth, texheight;
	 } glyph[128];
	 GLuint lineheight;
	 Freetype *parent;
	 friend class Freetype;
};

template<typename T, typename... Args>
inline bool Font::Print (T v, Args... args)
{
	std::ostringstream stream;
	stream.precision (2);
	stream << v;
	if (!PrintStr (stream.str ()))
		 return false;
	return Print (args...);
}

template<typename... Args>
inline bool Font::Print (glm::vec3 v, Args... args)
{
	std::ostringstream stream;
	stream.precision (2);
	stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	if (!PrintStr (stream.str ()))
		 return false;
	return Print (args...);
}

inline bool Font::Print (void)
{
	return true;
}

} /* namespace gl */

#endif /* !defined GL_FONT_H */
