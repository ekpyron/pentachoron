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
#ifndef PATCH_H
#define PATCH_H

#include <glm/glm.hpp>
#include <functional>

template<unsigned int n>
class Patch
{
public:
	 Patch (void)
			{
			}
	 Patch (const Patch<n> &q)
			{
				*this = q;
			}
	 ~Patch (void)
			{
			}
	 Patch &operator= (const Patch<n> &q)
			{
				for (auto i = 0; i < n; i++)
				{
					p[i] = q.p[i];
					eplus[i] = q.eplus[i];
					eminus[i] = q.eminus[i];
					fplus[i] = q.fplus[i];
					fminus[i] = q.fminus[i];
				}
				texcoords.clear ();
				for (const texcoord_t &t : q.texcoords)
				{
					texcoords.push_back (texcoord_t ());
					texcoord_t &texcoord = texcoords.back ();
					for (auto i = 0; i < n; i++)
					{
						texcoord.p[i] = t.p[i];
						texcoord.eplus[i] = t.eplus[i];
						texcoord.eminus[i] = t.eminus[i];
						texcoord.fplus[i] = t.fplus[i];
						texcoord.fminus[i] = t.fminus[i];
					}
				}
				return *this;
			}
	 glm::vec3 p[n];
	 glm::vec3 eplus[n];
	 glm::vec3 eminus[n];
	 glm::vec3 fplus[n];
	 glm::vec3 fminus[n];
	 typedef struct texcoord
	 {
			glm::vec2 p[n];
			glm::vec2 eplus[n];
			glm::vec2 eminus[n];
			glm::vec2 fplus[n];
			glm::vec2 fminus[n];
	 } texcoord_t;
	 std::vector<texcoord_t> texcoords;
};

template<unsigned int n>
bool operator< (const Patch<n> &p1, const Patch<n> &p2)
{
	for (auto i = 0; i < n; i++)
	{
		if (p1.p[i] < p2.p[i])
			 return true;
		if (p2.p[i] < p1.p[i])
			 return false;
		if (p1.eplus[i] < p2.eplus[i])
			 return true;
		if (p2.eplus[i] < p1.eplus[i])
			 return false;
		if (p1.eminus[i] < p2.eminus[i])
			 return true;
		if (p2.eminus[i] < p1.eminus[i])
			 return false;
		if (p1.fplus[i] < p2.fplus[i])
			 return true;
		if (p2.fplus[i] < p1.fplus[i])
			 return false;
		if (p1.fminus[i] < p2.fminus[i])
			 return true;
		if (p2.fminus[i] < p1.fminus[i])
			 return false;
	}

	if (p1.texcoords.size () < p2.texcoords.size ())
		 return true;
	if (p2.texcoords.size () < p1.texcoords.size ())
		 return false;

	for (auto i = 0; i < n; i++)
	{
		for (auto t = 0; t < p1.texcoords.size (); t++)
		{
			if (p1.texcoords[t].p[i] < p2.texcoords[t].p[i])
				 return true;
			if (p2.texcoords[t].p[i] < p1.texcoords[t].p[i])
				 return false;
			if (p1.texcoords[t].eplus[i] < p2.texcoords[t].eplus[i])
				 return true;
			if (p2.texcoords[t].eplus[i] < p1.texcoords[t].eplus[i])
				 return false;
			if (p1.texcoords[t].eminus[i] < p2.texcoords[t].eminus[i])
				 return true;
			if (p2.texcoords[t].eminus[i] < p1.texcoords[t].eminus[i])
				 return false;
			if (p1.texcoords[t].fplus[i] < p2.texcoords[t].fplus[i])
				 return true;
			if (p2.texcoords[t].fplus[i] < p1.texcoords[t].fplus[i])
				 return false;
			if (p1.texcoords[t].fminus[i] < p2.texcoords[t].fminus[i])
				 return true;
			if (p2.texcoords[t].fminus[i] < p1.texcoords[t].fminus[i])
				 return false;
		}
	}
	return false;
}

typedef Patch<3> TrianglePatch;
typedef Patch<4> QuadPatch;


#endif /* !defined PATCH_H */
