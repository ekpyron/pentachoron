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
#include "renderer.h"
#include "interface.h"
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <memory>
#include <AntTweakBar.h>

std::unique_ptr<Renderer> r;
YAML::Node config;
bool running;

void GLFWCALL resizecb (int w, int h)
{
	r->Resize (w, h);
	TwWindowSize (w, h);
}


int GLFWCALL closecb (void)
{
	running = false;
	return GL_FALSE;
}

void GLFWCALL keycb (int key, int action)
{
	if (TwEventKeyGLFW (key, action))
		 return;
}

int mousedownx = -1, mousedowny = -1;

void GLFWCALL mousemovecb (int x, int y)
{
	static int lastx = -1, lasty = -1;
	if (TwEventMousePosGLFW (x, y))
		 return;

	if (glfwGetMouseButton (0))
	{
		r->camera.RotateY (-0.1f * (x - mousedownx));
		r->camera.RotateUp (0.1f * (y - mousedowny));
		mousedownx = x;
		mousedowny = y;
	}
}

void GLFWCALL mousecb (int button, int action)
{
	if (TwEventMouseButtonGLFW (button, action))
		 return;
	if (action == GLFW_PRESS)
	{
		if (button == 0)
			glfwGetMousePos (&mousedownx, &mousedowny);
	}
}

void GLFWCALL mousewheelcb (int z)
{
	static int lastpos = 0;
	if (TwEventMouseWheelGLFW (z))
		 return;

	r->camera.MoveForward (z - lastpos);
	lastpos = z;
}

void GLFWCALL charcb (int c, int action)
{
	if (TwEventCharGLFW (c, action))
		 return;
}

std::ostream *logstream;

unsigned int fps = 0;

void UpdateCamera (void)
{
	static unsigned int frames = 0;
	static double last_time = -1, last_fps_time = -1;
	double timefactor;

	if (last_time < 0)
		 last_time = glfwGetTime ();

	timefactor = glfwGetTime () - last_time;
	last_time += timefactor;

	if (last_fps_time < 0)
		 last_fps_time = glfwGetTime ();

	if (glfwGetTime () - last_fps_time >= 1.0)
	{
		fps = frames;
		last_fps_time = glfwGetTime ();
		frames = 0;
	}
	frames++;

	if (glfwGetKey (GLFW_KEY_LSHIFT))
		 timefactor *= 5.0f;

	if (glfwGetKey (GLFW_KEY_LCTRL))
		 timefactor *= 10.0f;

	if (glfwGetKey (GLFW_KEY_LALT))
		 timefactor *= 20.0f;

	if (glfwGetKey (GLFW_KEY_RSHIFT))
		 timefactor *= 0.2f;

	if (glfwGetKey (GLFW_KEY_RCTRL))
		 timefactor *= 0.1f;

	if (glfwGetKey (GLFW_KEY_RALT))
		 timefactor *= 0.05f;

	if (glfwGetKey ('A'))
	{
		r->camera.RotateY (timefactor * 60.0f);
	}
	if (glfwGetKey ('D'))
	{
		r->camera.RotateY (-timefactor * 60.0f);
	}
	if (glfwGetKey ('W'))
	{
		r->camera.MoveForward (timefactor * 10.0f);
	}
	if (glfwGetKey ('S'))
	{
		r->camera.MoveForward (-timefactor * 10.0f);
	}
	if (glfwGetKey ('Q'))
	{
		r->camera.MoveUp (-4.0f * timefactor);
	}
	if (glfwGetKey ('E'))
	{
		r->camera.MoveUp (4.0f * timefactor);
	}
	if (glfwGetKey ('R'))
	{
		r->camera.RotateUp (timefactor * 60.0f);
	}
	if (glfwGetKey ('F'))
	{
		r->camera.RotateUp (-timefactor * 60.0f);
	}
}


int main (int argc, char *argv[])
{
	std::ofstream logfile;
#ifdef _WIN32
	freopen ("stdout.txt", "a", stdout);
	freopen ("stderr.txt", "a", stderr);
#endif
	try {
		logstream = &std::cerr;
	
		if (argc > 2)
		{
			(*logstream) << "Usage: " << argv[0] << " [configfile]" << std::endl;
			return -1;
		}

		{
			const char *filename;
			filename = (argc>1)?argv[1]:DEFAULT_CONFIGFILE;
			std::ifstream configfile (filename, std::ifstream::in);
			if (!configfile.is_open ())
			{
				(*logstream) << "Cannot open the config file "
										 << filename << "." << std::endl;
				return -1;
			}
			config = YAML::Load (configfile);
			if (!config.IsMap ())
			{
				(*logstream) << "The config file " << filename 
										 << " has an invalid format." << std::endl;
				return -1;
			}
		}

		if (config["logfile"])
		{
			logfile.open (config["logfile"].as<std::string> ());
			if (!logfile.is_open ())
			{
				(*logstream) << "Cannot open log file." << std::endl;
				return -1;
			}
			logstream = &logfile;
		}

		int w, h;
		if (glfwInit () != GL_TRUE)
		{
			(*logstream) << "glfwInit () failed." << std::endl;
			return -1;
		}
		glfwOpenWindowHint (GLFW_OPENGL_VERSION_MAJOR, 4);
		glfwOpenWindowHint (GLFW_OPENGL_VERSION_MINOR, 3);
		glfwOpenWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwOpenWindowHint (GLFW_OPENGL_PROFILE,
												GLFW_OPENGL_CORE_PROFILE);

		if (glfwOpenWindow (config["window"]["width"].as<int> (800),
												config["window"]["height"].as<int> (600),
												8, 8, 8, 8, 32, 8,
												config["window"]["fullscreen"].as<bool> (false)
												? GLFW_FULLSCREEN : GLFW_WINDOW) != GL_TRUE)
		{
			glfwTerminate ();
			(*logstream) << "glfwOpenWindow () failed." << std::endl;
			return -1;
		}
		glfwSetWindowTitle ("Pentachoron");
		glfwEnable (GLFW_KEY_REPEAT);

		TwInit (TW_OPENGL_CORE, NULL);

		gl::Init (glfwGetProcAddress);

		r = std::unique_ptr<Renderer> (new Renderer ());

		if (!r->Init ())
		{
			r.reset ();
			TwTerminate ();
			glfwTerminate ();
			return -1;
		}

		CreateMenus ();

		glfwSetWindowSizeCallback (resizecb);
		glfwSetMouseButtonCallback (mousecb);
		glfwSetMouseWheelCallback (mousewheelcb);
		glfwSetCharCallback (charcb);
		glfwSetMousePosCallback (mousemovecb);
		glfwSetKeyCallback (keycb);
		glfwSetWindowCloseCallback (closecb);
		glfwGetWindowSize (&w, &h);
		resizecb (w, h);

		glfwSwapInterval (0);

		running = true;
		while (running && glfwGetWindowParam (GLFW_OPENED) == GL_TRUE)
		{
			UpdateCamera ();
			r->Frame ();
			gl::BindSampler (0, 0);
			gl::BlendEquation (GL_FUNC_ADD);
			TwDraw ();
			glfwSwapBuffers ();
		}
		
		(*logstream) << "Main loop returned."	<< std::endl;

		r.reset ();
		
		(*logstream) << "Renderer class was freed." << std::endl;

		TwTerminate ();
		glfwTerminate ();
		
		(*logstream) << "Exiting." << std::endl;
		
		exit (0);
	} catch (std::exception &e) {
		(*logstream) << "Exception: " << e.what () << std::endl;
		r.reset ();
		(*logstream) << "Renderer class was freed." << std::endl;
		TwTerminate ();
		glfwTerminate ();
		(*logstream) << "Exiting." << std::endl;
		exit (-1);
	} catch (...) {
		(*logstream) << "Unknown exception." << std::endl;
		r.reset ();
		(*logstream) << "Renderer class was freed." << std::endl;
		TwTerminate ();
		glfwTerminate ();
		(*logstream) << "Exiting." << std::endl;
		exit (-1);
	}
}
