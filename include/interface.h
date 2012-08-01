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
#ifndef INTERFACE_H
#define INTERFACE_H

#include <common.h>
#include "font/all.h"

class Interface
{
public:
	 Interface (void);
	 ~Interface (void);
	 bool Init (void);
	 void Frame (float timefactor);
	 void OnKeyUp (int key);
	 void OnKeyDown (int key);

	 void AddLight (void);
	 void AddShadow (void);
private:
	 gl::Freetype freetype;
	 gl::Font font;

	 GLuint showInterface;
	 unsigned int menu, submenu;

	 typedef struct MenuEntry {
			std::string name;
			std::function<void(void)> info;
			std::function<void(int)> handler;
			bool repeating;
	 } MenuEntry;
	 
	 typedef struct Menu {
			std::string title;
			std::function<void(void)> info;
			std::vector<MenuEntry> entries;
	 } Menu;

	 std::vector<Menu> menus;

	 int active_light;
	 int active_shadow;
	 int active_parameter;

	 float timefactor;
};

#endif /* !defined INTERFACE_H */
