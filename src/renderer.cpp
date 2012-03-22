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
#include "renderer.h"

Renderer::Renderer (void)
	: geometry (this), shadowpass (this),
		finalpass (this), gbuffer (this), filters (this),
		interface (this), composition (this)
{
}

Renderer::~Renderer (void)
{
}

bool Renderer::Init (void)
{
	gl::FrontFace (GL_CCW);
	gl::CullFace (GL_BACK);
	gl::Enable (GL_CULL_FACE);

	if (!interface.Init ())
		 return false;

	if (!windowgrid.Init ())
		 return false;

	if (!geometry.Init ())
		 return false;
	if (!gbuffer.Init ())
		 return false;
	if (!filters.Init ())
		 return false;
	if (!shadowpass.Init ())
		 return false;

	interface.AddLight (0);

	if (!composition.Init ())
		 return false;

	if (!finalpass.Init ())
		 return false;

	return true;
}

void Renderer::Resize (int w, int h)
{
	camera.Resize (w, h);
}

void Renderer::OnKeyUp (int key)
{
	interface.OnKeyUp (key);
}

void Renderer::OnKeyDown (int key)
{
	interface.OnKeyDown (key);
}

void Renderer::Frame (void)
{
	static float last_time = 0;
	float timefactor;

	if (last_time == 0)
		 last_time = glfwGetTime ();
	else
	{
		timefactor = glfwGetTime () - last_time;
		last_time += timefactor;
	}

	camera.Frame (timefactor);
	gbuffer.Render (geometry);

	shadowpass.FrameInit ();
	for (Shadow &shadow : shadows)
	{
		shadowpass.Render (shadow);
	}
	shadowpass.FrameFinish ();

	composition.Frame (timefactor);

	finalpass.Render ();

	interface.Frame (timefactor);

	GL_CHECK_ERROR;
}
