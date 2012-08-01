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
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <memory>

std::unique_ptr<Renderer> r;
YAML::Node config;
bool running;

void GLFWCALL resizecb (int w, int h)
{
	r->Resize (w, h);
}


int GLFWCALL closecb (void)
{
	running = false;
	return GL_FALSE;
}

void GLFWCALL keycb (int key, int action)
{
	if (action == GLFW_PRESS)
		 r->OnKeyDown (key);
	else if (action == GLFW_RELEASE)
		 r->OnKeyUp (key);
}

std::ostream *logstream;

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
		glfwOpenWindowHint (GLFW_OPENGL_VERSION_MINOR, 2);
		glfwOpenWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//		glfwOpenWindowHint (GLFW_OPENGL_PROFILE,
//												GLFW_OPENGL_CORE_PROFILE);

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

		gl::Init (glfwGetProcAddress);

		r = std::unique_ptr<Renderer> (new Renderer ());

		if (!r->Init ())
		{
			r.reset ();
			glfwTerminate ();
			return -1;
		}

		glfwSetWindowSizeCallback (resizecb);
		glfwSetKeyCallback (keycb);
		glfwSetWindowCloseCallback (closecb);
		glfwGetWindowSize (&w, &h);
		resizecb (w, h);

		glfwSwapInterval (0);

		running = true;
		while (running && glfwGetWindowParam (GLFW_OPENED) == GL_TRUE)
		{
			r->Frame ();
			glfwSwapBuffers ();
		}
		
		(*logstream) << "Main loop returned."	<< std::endl;

		r.reset ();
		
		(*logstream) << "Renderer class was freed." << std::endl;

		glfwTerminate ();
		
		(*logstream) << "Exiting." << std::endl;
		
		exit (0);
	} catch (std::exception &e) {
		(*logstream) << "Exception: " << e.what () << std::endl;
		r.reset ();
		(*logstream) << "Renderer class was freed." << std::endl;
		glfwTerminate ();
		(*logstream) << "Exiting." << std::endl;
		exit (-1);
	} catch (...) {
		(*logstream) << "Unknown exception." << std::endl;
		r.reset ();
		(*logstream) << "Renderer class was freed." << std::endl;
		glfwTerminate ();
		(*logstream) << "Exiting." << std::endl;
		exit (-1);
	}
}
