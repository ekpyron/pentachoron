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
#include "font/font.h"

namespace gl {

Font::Font (void) : face (NULL), color (glm::vec3 (1, 1, 1))
{
}

Font::Font (Font &&font) : face (font.face), color (font.color)
{
	font.face = NULL;
	font.color = glm::vec3 (1, 1, 1);
}

Font::~Font (void)
{
	if (face)
	{
		FT_Done_Face (face);
		face = NULL;
	}
}

bool Font::Load (Freetype *freetype, const std::string &name)
{
	FT_Error err;
	parent = freetype;
	if (face)
	{
		FT_Done_Face (face);
		face = NULL;
	}

	err = FT_New_Face (freetype->library, name.c_str (), 0, &face);
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

inline unsigned int powerof2 (unsigned int x)
{
	x--;
	x |= x >> 1; 	x |= x >> 2; 	x |= x >> 4;
	x |= x >> 8;	x |= x >> 16;
	x++;
	return x;
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
	FT_Glyph glyph;
	FT_BitmapGlyph bitmapglyph;
	FT_Error err;

	if (!face)
	{
		(*logstream) << "PutChar called without a loaded font." << std::endl;
		return false;
	}

	switch (c)
	{
	case ' ':
		pos.x += face->glyph->advance.x >> 6;
		return true;
	case '\n':
		pos.x = 0;
		pos.y += (face->size->metrics.height >> 6);
		return true;
	}

	if (err = FT_Load_Char (face, c, FT_LOAD_RENDER))
	{
		(*logstream) << "Freetype can't load a character ("
								 << static_cast<int> (c) << ")." << std::endl;
		(*logstream) << "ERR: " << std::hex << err << std::endl;
		return false;
	}

	if (FT_Get_Glyph (face->glyph, &glyph))
	{
		(*logstream) << "Cannot obtain the glyph for a character ("
								 << static_cast<int> (c) << ")." << std::endl;
	}

	if (glyph->format != FT_GLYPH_FORMAT_BITMAP)
	{
		(*logstream) << "glyph->format != FT_GLYPH_FORMAT_BITMAP" << std::endl;
	}

	bitmapglyph = reinterpret_cast<FT_BitmapGlyph> (glyph);

	gl::Buffer data;

	int width, height;
	int texwidth, texheight;

	width = bitmapglyph->bitmap.width;
	height = bitmapglyph->bitmap.rows;

	texwidth = powerof2 (width);
	texheight = powerof2 (height);

	data.Data (texwidth * texheight, NULL, GL_STREAM_DRAW);

	GLubyte *ptr;

	ptr = static_cast<GLubyte*> (data.Map (GL_WRITE_ONLY));
	for (auto y = 0; y < texheight; y++)
	{
		for (auto x = 0; x < texwidth; x++)
		{
			if ((y < height) && (x < width))
			{
				ptr[y * texwidth + x] = bitmapglyph->bitmap.buffer [y * width + x];
			}
			else
			{
				ptr[y * texwidth + x] = 0;
			}
		}
	}
	data.Unmap ();

	data.Bind (GL_PIXEL_UNPACK_BUFFER);
	texture.Image2D (GL_TEXTURE_2D, 0, GL_R8, texwidth, texheight, 0, GL_RED,
									 GL_UNSIGNED_BYTE, NULL);
	gl::Buffer::Unbind (GL_PIXEL_UNPACK_BUFFER);

	texture.Bind (GL_TEXTURE0, GL_TEXTURE_2D);

	glm::mat4 mat;
	mat = glm::ortho (0.0f, 1280.0f, 0.0f, 1024.0f, -1.0f, 1.0f);
	mat = glm::translate (mat,
												glm::vec3 (pos.x,
																	 1024 - (face->size->metrics.height >> 6)
																	 - pos.y,
																	 0));
	mat = glm::translate (mat, glm::vec3 (bitmapglyph->left,
																				bitmapglyph->top - height, 0));
	parent->Render (glm::vec2 (float (width) / float (texwidth),
														 float (height) / float (texheight)),
									glm::vec2 (width, height), mat,
									color);
	pos.x += face->glyph->advance.x >> 6;

	FT_Done_Glyph (glyph);

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
