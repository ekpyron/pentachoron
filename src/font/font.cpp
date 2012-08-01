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
#include "font/font.h"

namespace gl {

class FontFace
{
public:
	 FontFace (void);
	 FontFace (const FontFace &) = delete;
	 ~FontFace (void);
	 const FontFace &operator= (const FontFace&) = delete;
	 bool Init (FT_Library &library, const char *name);
	 FT_Face face;
};

FontFace::FontFace (void) : face (NULL)
{
}


FontFace::~FontFace (void)
{
	if (face != NULL)
	{
		FT_Done_Face (face);
		face = NULL;
	}
}
bool FontFace::Init (FT_Library &library, const char *name)
{
	FT_Error err;
	err = FT_New_Face (library, name, 0, &face);
	if (err)
	{
		(*logstream) << "Cannot load the font file " << name << "." << std::endl;
		return false;
	}

	err = FT_Set_Char_Size (face, 32 << 6, 32 << 6, 96, 96);
	if (err)
	{
		(*logstream) << "Cannot set the character size for " << name
								 << "." << std::endl;
		return false;
	}
	return true;
}


Font::Font (void) : color (glm::vec3 (1, 1, 1))
{
}

Font::Font (Font &&font) : color (font.color)
{
	font.color = glm::vec3 (1, 1, 1);
}

Font::~Font (void)
{
}


inline unsigned int powerof2 (unsigned int x)
{
	x--;
	x |= x >> 1; 	x |= x >> 2; 	x |= x >> 4;
	x |= x >> 8;	x |= x >> 16;
	x++;
	return x;
}

bool Font::Load (Freetype *freetype, const std::string &name)
{
	FontFace fontface;
	parent = freetype;
	if (!fontface.Init (freetype->library, name.c_str ()))
		 return false;

	for (unsigned char c = 0; c < 128; c++)
	{
		FT_Glyph ft_glyph;
		FT_BitmapGlyph bitmapglyph;
		FT_Error err;

		if (err = FT_Load_Char (fontface.face, c, FT_LOAD_RENDER))
		{
			(*logstream) << "Freetype can't load a character ("
									 << static_cast<int> (c) << ")." << std::endl;
			(*logstream) << "ERR: " << std::hex << err << std::endl;
			return false;
		}

		if (FT_Get_Glyph (fontface.face->glyph, &ft_glyph))
		{
			(*logstream) << "Cannot obtain the glyph for a character ("
									 << static_cast<int> (c) << ")." << std::endl;
			return false;
		}

		if (ft_glyph->format != FT_GLYPH_FORMAT_BITMAP)
		{
			(*logstream) << "glyph->format != FT_GLYPH_FORMAT_BITMAP" << std::endl;
			return false;
		}

		bitmapglyph = reinterpret_cast<FT_BitmapGlyph> (ft_glyph);

		glyph[c].width = bitmapglyph->bitmap.width;
		glyph[c].height = bitmapglyph->bitmap.rows;

		glyph[c].texwidth = powerof2 (glyph[c].width);
		glyph[c].texheight = powerof2 (glyph[c].height);

		std::vector<GLubyte> ptr;
		ptr.resize (glyph[c].texwidth * glyph[c].texheight);

		for (auto y = 0; y < glyph[c].texheight; y++)
		{
			for (auto x = 0; x < glyph[c].texwidth; x++)
			{
				if ((y < glyph[c].height) && (x < glyph[c].width))
				{
					ptr[y * glyph[c].texwidth + x] =
						 bitmapglyph->bitmap.buffer [y * glyph[c].width + x];
				}
				else
				{
					ptr[y * glyph[c].texwidth + x] = 0;
				}
			}
		}

		gl::Buffer::Unbind (GL_PIXEL_UNPACK_BUFFER);
		glyph[c].texture.Image2D (GL_TEXTURE_2D, 0, GL_R8,
															glyph[c].texwidth, glyph[c].texheight,
															0, GL_RED, GL_UNSIGNED_BYTE, &ptr[0]);

		glyph[c].advance = fontface.face->glyph->advance.x >> 6;
		glyph[c].left = bitmapglyph->left;
		glyph[c].top = bitmapglyph->top;

		FT_Done_Glyph (ft_glyph);
	}

	lineheight = fontface.face->size->metrics.height >> 6;

	return true;
}

void Font::Frame (void)
{
	pos = glm::vec2 (0, 0);
}

void Font::SetColor (const glm::vec3 &c)
{
	color = c;
}

bool Font::PutChar (char c)
{
	switch (c)
	{
	case ' ':
		pos.x += glyph[c].advance;
		return true;
	case '\n':
		pos.x = 0;
		pos.y += lineheight;
		return true;
	}

	if (c >= 128 || c < 0)
	{
		(*logstream) << "Cannot print character (" << static_cast<int> (c)
								 << ")." << std::endl;
		return false;
	}		 

	glyph[c].texture.Bind (GL_TEXTURE0, GL_TEXTURE_2D);

	glm::mat4 mat;
	mat = glm::ortho (0.0f, 1280.0f, 0.0f, 1024.0f, -1.0f, 1.0f);
	mat = glm::translate (mat,
												glm::vec3 (pos.x,
																	 1024 - lineheight
																	 - pos.y,
																	 0));
	mat = glm::translate (mat, glm::vec3 (glyph[c].left,
																				glyph[c].top - glyph[c].height, 0));
	parent->Render (glm::vec2 (float (glyph[c].width) /
														 float (glyph[c].texwidth),
														 float (glyph[c].height) /
														 float (glyph[c].texheight)),
									glm::vec2 (glyph[c].width, glyph[c].height), mat,
									color);
	pos.x += glyph[c].advance;

	GL_CHECK_ERROR;

	return true;
}

bool Font::PrintStr (const std::string &str)
{
	for (auto it = str.begin (); it != str.end (); it++)
	{
		if (!PutChar (*it))
			 return false;
	}
	return true;
}

} /* namespace gl */
