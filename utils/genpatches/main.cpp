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
#include <iostream>
#include <sstream>
#include "mesh.h"

#include <glm/gtc/quaternion.hpp>
#include <GL/glew.h>
#include <GL/glfw.h>
#include <GL/gl.h>
#include <GL/glu.h>

Mesh mesh;

GLuint current_patch = 0;

void DrawScene (void)
{
	glPointSize (10.0f);
	glLineWidth (2.0f);
	glBegin (GL_POINTS);
	for (auto i = 0; i < mesh.GetNumFaces (); i++)
	{
		const Face &f = mesh.GetFace (i);
		for (auto c = 0; c < f.GetNumVertices (); c++)
		{
			if (mesh.IsBorder (f.GetVertex (c)))
				 glColor3ub (128, 128, 128);
			else
				 glColor3ub (255, 255, 255);
			glVertex3fv (&f.GetVertex (c).x);
		}
	}
	glEnd ();
	glBegin (GL_LINES);
	for (const Edge &edge : mesh.GetEdges ())
	{
			if (mesh.IsBorder (edge))
				 glColor3ub (128, 128, 128);
			else
				 glColor3ub (255, 255, 255);
			glVertex3fv (&edge.GetFirst ().x);
			glVertex3fv (&edge.GetSecond ().x);
	}
	glEnd ();

	const QuadPatch &patch = mesh.GetQuadPatch (current_patch);

	glColor3ub (255, 0, 0);
	glBegin (GL_POINTS);
	glVertex3fv (&patch.p[0].x);
	glVertex3fv (&patch.p[1].x);
	glVertex3fv (&patch.p[2].x);
	glVertex3fv (&patch.p[3].x);
	for (int i = 0; i < 4; i++)
	{
		glColor3ub (255, 255, 0);
		glVertex3fv (&patch.eplus[i].x);
		glColor3ub (0, 255, 255);
		glVertex3fv (&patch.eminus[i].x);
		glColor3ub (255, 0, 255);
		glVertex3fv (&patch.fplus[i].x);
		glVertex3fv (&patch.fminus[i].x);
	}
	glEnd ();
	glColor3ub (255, 0, 0);
	glBegin (GL_LINE_STRIP);
	glVertex3fv (&patch.p[0].x);
	glVertex3fv (&patch.eplus[0].x);
	glVertex3fv (&patch.eminus[1].x);
	glVertex3fv (&patch.p[1].x);
	glVertex3fv (&patch.eplus[1].x);
	glVertex3fv (&patch.eminus[2].x);
	glVertex3fv (&patch.p[2].x);
	glVertex3fv (&patch.eplus[2].x);
	glVertex3fv (&patch.eminus[3].x);
	glVertex3fv (&patch.p[3].x);
	glVertex3fv (&patch.eplus[3].x);
	glVertex3fv (&patch.eminus[0].x);
	glVertex3fv (&patch.p[0].x);
	glEnd ();
	glColor3ub (255, 0, 255);
	glBegin (GL_LINES);
	for (int i = 0; i < 4; i++)
	{
		glVertex3fv (&patch.eminus[i].x);
		glVertex3fv (&patch.fminus[i].x);
		glVertex3fv (&patch.eplus[i].x);
		glVertex3fv (&patch.fplus[i].x);
	}
	glEnd ();
}

void keyfn (int key, int action)
{
	if (action == GLFW_RELEASE)
	{
		switch (key)
		{
		case GLFW_KEY_TAB:
			current_patch++;
			if (current_patch >= mesh.GetNumQuadPatches ())
				 current_patch = 0;
			break;
		}
	}
}

int main (int argc, char *argv[])
{
	unsigned int meshid;

	if (argc != 4)
	{
		std::cerr << "Usage: " << argv[0] << " [input] [mesh] [output]"
							<< std::endl;
		return -1;
	}

	{
		std::stringstream stream (argv[2]);
		if ((stream >> meshid).bad ())
		{
			std::cerr << "Invalid mesh." << std::endl;
			std::cerr << "Usage: " << argv[0] << " [input] [mesh] [output]"
								<< std::endl;
			return -1;
		}
	}

	mesh.Import (argv[1], meshid);

	mesh.GeneratePatches ();

	mesh.Export (argv[3]);

	glfwInit ();

	glfwOpenWindow (0, 0, 0, 0, 0, 0, 24, 8, GLFW_WINDOW);

	glewInit ();

	glm::vec3 center (0, 0, 0);
	float angley = 0, anglex = 0;
	glfwSetKeyCallback (keyfn);
	while (glfwGetWindowParam (GLFW_OPENED) && !glfwGetKey (GLFW_KEY_ESC))
	{
		int w, h;
		float f = 1.0f;
		glfwGetWindowSize (&w, &h);

		glViewport (0, 0, w, h);
		glMatrixMode (GL_PROJECTION);

		glLoadIdentity ();
		gluPerspective (60.0f, float (w) / float (h), 0.1f, 100.0f);

		glMatrixMode (GL_MODELVIEW);

		glLoadIdentity ();
		glm::vec3 dir (0, 0, 1);
		glm::vec3 up (0, 1, 0);
		glm::quat rotation;
		rotation = glm::rotate (rotation, angley, glm::vec3 (0, 1, 0));
		rotation = glm::rotate (rotation, anglex, glm::vec3 (1, 0, 0));
		dir = rotation * dir;
		up = rotation * up;
		glm::vec3 eye = center - 1.0f * dir;
		if (glfwGetKey (GLFW_KEY_LSHIFT))
			 f *= 2.0f;
		if (glfwGetKey (GLFW_KEY_LCTRL))
			 f *= 5.0f;
		if (glfwGetKey ('D'))
			 angley += -0.2f * f;
		if (glfwGetKey ('A'))
			 angley += 0.2f * f;
		if (glfwGetKey ('Q'))
			 anglex += 0.2f * f;
		if (glfwGetKey ('E'))
			 anglex += -0.2f * f;
		if (glfwGetKey ('W'))
			 center += dir * 0.01f * f;
		if (glfwGetKey ('S'))
			 center -= dir * 0.01f * f;

		gluLookAt (eye.x, eye.y, eye.z, center.x, center.y, center.z,
							 up.x, up.y, up.z);

		glClearColor (0, 0, 0, 0);
		glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		glEnable (GL_DEPTH_TEST);
		glDepthFunc (GL_LEQUAL);

		DrawScene ();

		glfwSwapBuffers ();
	}

}
