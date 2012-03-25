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
#ifndef OPACITYPASS_H
#define OPACITYPASS_H

#include <common.h>

class Renderer;

class OpacityPass
{
public:
	 OpacityPass (Renderer *parent);
	 ~OpacityPass (void);
	 bool Init (void);
	 void Render (void);
	 gl::Texture texture;
private:
	 gl::Program program;
	 gl::Framebuffer framebuffer;
	 Renderer *renderer;
};

#endif /* !defined OPACITYPASS_H */
