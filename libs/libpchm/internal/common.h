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
#ifndef COMMON_H
#define COMMON_H

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <iostream>

#ifdef M_PI
#define PCH_PI M_PI
#else
#define PCH_PI 3.14159265358979323846
#endif


inline std::ostream &operator<< (std::ostream &os, const glm::vec3 &v)
{
	os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return os;
}

namespace std {
template<typename T>
inline bool operator< (const glm::detail::tvec2<T> &v1,
											 const glm::detail::tvec2<T> &v2)
{
	if (v1.x < v2.x)
		 return true;
	if (v1.x > v2.x)
		 return false;
	return v1.y < v2.y;
}


template<typename T>
inline bool operator< (const glm::detail::tvec3<T> &v1,
											 const glm::detail::tvec3<T> &v2)
{
	if (v1.x < v2.x)
		 return true;
	if (v1.x > v2.x)
		 return false;
	if (v1.y < v2.y)
		 return true;
	if (v1.y > v2.y)
		 return false;
	return v1.z < v2.z;
}

template<typename T>
inline bool operator> (const glm::detail::tvec3<T> &v1,
											 const glm::detail::tvec3<T> &v2)
{
	if (v1.x > v2.x)
		 return true;
	if (v1.x < v2.x)
		 return false;
	if (v1.y > v2.y)
		 return true;
	if (v1.y < v2.y)
		 return false;
	return v1.z > v2.z;
}
}

using std::operator<;
using std::operator>;

#endif /* !defined COMMON_H */
