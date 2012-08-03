/*  
 * This file is part of conv2pchm.
 *
 * conv2pchm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * conv2pchm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with conv2pchm.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef GLWINDOW_H
#define GLWINDOW_H

#include "common.h"
#include "mesh.h"

class GlWindow : public Fl_Gl_Window
{
public:
	 GlWindow (int X, int Y, int W, int H, const char *L = 0);
	 void DrawMesh (Mesh *m);
	 void rotate (int X, int Y);
	 void reset (void);
	 void zoom (int z);
private:
	 float anglex, angley;
	 float zoomfactor;
	 Mesh *mesh;
	 void draw (void);
	 void resize (int X, int Y, int W, int H);
	 void SetViewport (int W, int H);
};

#endif /* !defined GLWINDOW_H */
