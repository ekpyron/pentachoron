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
#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <stdexcept>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <yaml-cpp/yaml.h>
#include <config.h>
#include <oglp/oglp.h>
#include <iostream>

#define __gl_h__
#include <GL/glfw.h>

#include <oclp/oclp.h>

void glCheckError (const std::string &file, unsigned int line);
#define GL_CHECK_ERROR   glCheckError (__FILE__,__LINE__)

extern YAML::Node config;

template<typename T>
inline std::string ConcatPath (const std::string &str1, T str2)
{
	return str1 + DIR_SEPARATOR + str2;
}

template<typename... Args>
inline std::string ConcatPath (const std::string &str, Args... args)
{
	return ConcatPath (str, ConcatPath (args...));
}

template<typename... Args>
inline std::string MakePath (Args... args)
{
	return ConcatPath (config["basedir"].as<std::string> (), args...);
}

bool ReadFile (const std::string &filename, std::string &str);

extern std::ostream *logstream;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#endif /* COMMON_H */
