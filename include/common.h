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
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <yaml-cpp/yaml.h>
#include <config.h>
#include <oglp/oglp.h>
#include <iostream>

/** Defined to avoid conflicts.
 * Defined to avoid the inclusion of an conflicting gl.h
 */
#define __gl_h__
#include <GL/glfw.h>

#include <oclp/oclp.h>

/** Convert OpenGL error code to a string.
 * Converts an OpenGL error code to a string.
 * \param err OpenGL error code
 * \returns a formatted error message
 */
const char *glErrorString (GLenum err);

/** Check for an OpenGL error.
 * Checks for an OpenGL error and logs an error message if appropriate.
 * This function is usually not called directly, but by using
 * the GL_CHECK_ERROR macro.
 * \param file The name of the source file the function is called from.
 * \param line The line within the source file from which this function
 *             was called from.
 */
void glCheckError (const std::string &file, unsigned int line);

/** Check for an OpenGL error.
 * Checks for an OpenGL error and logs an error message if appropriate.y
 */
#define GL_CHECK_ERROR   glCheckError (__FILE__,__LINE__)

/** Global config object.
 * Global config object containing all configuration options as defined
 * in the configuration file.
 */
extern YAML::Node config;

/// @cond
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
/// @endcond

/** Concatenate strings to a path.
 * Concatenates strings using the platform specific directory
 * separator as to produce a file path.
 * \param args A variadic list of strings to concatenate to a path.
 * \return The resulting path.
 */
template<typename... Args>
inline std::string MakePath (Args... args)
{
	return ConcatPath (config["basedir"].as<std::string> (), args...);
}

/** Read a file into memory.
 * Reads a file into memory.
 * \param filename File to read into memory.
 * \param str String to store the file's content.
 * \returns Whether the file was read successfully.
 */
bool ReadFile (const std::string &filename, std::string &str);

/** Global log stream.
 * Global log stream used to output all log messages.
 */
extern std::ostream *logstream;

/** PI.
 * An approximate value of PI.
 */
#ifdef M_PI
#define DRE_PI M_PI
#else
#define DRE_PI 3.14159265358979323846
#endif

#endif /* COMMON_H */
