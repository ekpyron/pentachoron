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
#ifndef WINDOWGRID_H
#define WINDOWGRID_H

#include <common.h>

class WindowGrid
{
public:
	 WindowGrid (void);
	 ~WindowGrid (void);
	 bool Init (void);
	 void Render (void);
	 gl::Program vprogram;
private:
	 gl::Buffer arraybuffer;
	 gl::VertexArray vao;
};

#endif /* !defined WINDOWGRID_H */
