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
#ifndef COMMON_H
#define COMMON_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <cmath>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Wizard.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Simple_Counter.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Input.H>
#include <FL/fl_ask.H>
#include <FL/gl.h>
#include <vector>
#include <iostream>
#include <GL/glu.h>
#include <Importer.hpp>
#include <scene.h>
#include <postprocess.h>
#include <DefaultLogger.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#endif /* !defined COMMON_H */
