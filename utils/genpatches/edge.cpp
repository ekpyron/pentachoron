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
#include "common.h"
#include "edge.h"
#include <stdexcept>

Edge::Edge (void)
{
}

Edge::Edge (const Edge &e) : endpoints (e.endpoints)
{
}

Edge::Edge (const glm::vec3 &v1, const glm::vec3 &v2)
{
	if (v1 < v2)
		endpoints = std::make_pair (v1, v2);
	else
		endpoints = std::make_pair (v2, v1);
}

Edge::~Edge (void)
{
}

Edge &Edge::operator= (const Edge &e)
{
	endpoints = e.endpoints;
	return *this;
}

bool Edge::operator< (const Edge &e) const
{
	return endpoints < e.endpoints;
}


bool Edge::operator== (const Edge &e) const
{
	return (endpoints == e.endpoints);
}

glm::vec3 Edge::GetMidpoint (void) const
{
	return 0.5f * (endpoints.first + endpoints.second);
}

const glm::vec3 &Edge::GetOther (const glm::vec3 &v) const
{
	if (endpoints.first == v)
		 return endpoints.second;
	else if (endpoints.second == v)
		 return endpoints.first;
	else
		 throw std::runtime_error ("invalid vertex");
}

const glm::vec3 &Edge::GetFirst (void) const
{
	return endpoints.first;
}

const glm::vec3 &Edge::GetSecond (void) const
{
	return endpoints.second;
}
