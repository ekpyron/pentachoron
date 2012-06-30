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
#ifndef CONFIG_H
#define CONFIG_H

/** Directory separator.
 * The platform specific separator between directories in a path.
 */
#ifdef _WIN32
#define DIR_SEPARATOR        '\\'
#else
#define DIR_SEPARATOR        '/'
#endif
/** Default config file.
 * Defines the default config file name.
 */
#define DEFAULT_CONFIGFILE   "config.yaml"
/** Debug output.
 * Turns on debug output.
 */
#define DEBUG

/**
 * #define ASSIMP_DEBUG
 */

#endif /* !defined CONIG_H */
