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
#ifndef EDGE_H
#define EDGE_H

#include "common.h"

class Edge
{
public:
	 Edge (void);
	 Edge (const Edge &e);
	 Edge (const glm::vec3 &_v1, const glm::vec3 &_v2);
	 ~Edge (void);
	 Edge &operator= (const Edge &e);
	 bool operator< (const Edge &e) const;
	 bool operator== (const Edge &e) const;
	 glm::vec3 GetMidpoint (void) const;
	 const glm::vec3 &GetOther (const glm::vec3 &v) const;
	 const glm::vec3 &GetFirst (void) const;
	 const glm::vec3 &GetSecond (void) const;
private:
	 std::pair<glm::vec3, glm::vec3> endpoints;
};

inline std::ostream &operator<< (std::ostream &os, const Edge &e)
{
	os << "[ " << e.GetFirst () << " - " << e.GetSecond () << " ]";
	return os;
}

#endif /* !defined EDGE_H */
