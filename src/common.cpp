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
#include <common.h>
#include <iostream>
#include <fstream>

const char *glErrorString (GLenum err)
{
	switch (err)
	{
	case GL_NO_ERROR:
		return "GL_NO_ERROR";
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	default:
		return "UNKNOWN ERROR";
	}
}

void glCheckError (const std::string &file, unsigned int line)
{
	GLenum err;
	err = gl::GetError ();
	if (err != 0)
	{
		(*logstream) << "OpenGL Error detected in " << file
							 << " line " << std::dec << line << ": "
							 << glErrorString (err) << std::endl;
	}
}

std::string MakePath (const std::string &subdir,
											const std::string &filename)
{
	std::string path;
	path = config["directories"]["base"].as<std::string> () + DIR_SEPARATOR
		 + config["directories"][subdir].as<std::string> ()
		 + DIR_SEPARATOR + filename;
	return path;
}

bool ReadFile (const std::string &filename, std::string &str)
{
	std::ifstream file (filename, std::ios::in);
	if (!file.is_open ())
	{
		(*logstream) << "Cannot open " << filename << std::endl;
		return false;
	}
	str.clear ();
	while (file.good ())
	{
		char buffer[256];
		file.read (buffer, 256);
		str.append (buffer, file.gcount ());
	}
	return true;
}
