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
#include "glwindow.h"

GlWindow::GlWindow (int X, int Y, int W, int H, const char *L)
		 : Fl_Gl_Window (X, Y, W, H, L)
{
	end ();
	mesh = NULL;
}

void GlWindow::rotate (int X, int Y)
{
	anglex += float (X);
	angley += float (Y);
}

void GlWindow::reset (void)
{
	anglex = angley = 0.0f;
	zoomfactor = 1.0f;
}

void GlWindow::resize (int X, int Y, int W, int H)
{
	Fl_Gl_Window::resize (X, Y, W, H);
	SetViewport (W, H);
}

void GlWindow::zoom (int z)
{
	zoomfactor += float (z) * 0.05f;
}

void GlWindow::SetViewport (int W, int H)
{
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glViewport (0, 0, W, H);
	glMultMatrixf (glm::value_ptr (glm::perspective (60.0f,
																									 float (W) / float (H),
																									 0.01f, 1000.0f)));
	glMatrixMode (GL_MODELVIEW);
}

void GlWindow::draw (void)
{
	if (!valid ())
	{
		valid (1);
		SetViewport (w (), h ());
	}
	glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth (1.0f);
	glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	if (mesh != NULL)
	{
		glLoadIdentity ();
		glEnable (GL_DEPTH_TEST);

		glm::vec3 center;
		center = 0.5f * (mesh->min + mesh->max);

		float len = glm::distance (mesh->min, mesh->max);
		len *= zoomfactor;

		glm::vec3 dir, up;

		glm::quat rotation;
		rotation = glm::rotate (rotation, -anglex,
														glm::vec3 (0.0f, 1.0f, 0.f));
		rotation = glm::rotate (rotation, -angley,
														glm::vec3 (1.0f, 0.0f, 0.f));
		
		dir = rotation * glm::vec3 (0.0f, 0.0f, -1.0f);
		up = rotation * glm::vec3 (0.0f, 1.0f, 0.0f);

		glMultMatrixf (glm::value_ptr (glm::lookAt (center - dir * len,
																								center, up)));
		
		glEnable (GL_LIGHT0);
		glEnable (GL_LIGHTING);
		glEnable (GL_SMOOTH);

		glLightModeli (GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

		glVertexPointer (3, GL_FLOAT, 0, &mesh->vertices[0]);
		glNormalPointer (GL_FLOAT, 0, &mesh->normals[0]);
		glEnableClientState (GL_VERTEX_ARRAY);
		glEnableClientState (GL_NORMAL_ARRAY);

		GLuint indices[] = { 0, 1, 2, 3 };
		
		switch (mesh->edges)
		{
		case 3:
			glDrawElements (GL_TRIANGLES, mesh->indices.size (),
											GL_UNSIGNED_INT, &mesh->indices[0]);
			break;
		case 4:
			glDrawElements (GL_QUADS, mesh->indices.size (),
											GL_UNSIGNED_INT, &mesh->indices[0]);
			break;
		}
	}
}

void GlWindow::DrawMesh (Mesh *m)
{
	mesh = m;
}
