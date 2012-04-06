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
	: geometry (this), shadowmap (this),
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

	(*logstream) << "Initialize Interface..." << std::endl;

	queue = clctx.CreateCommandQueue (0);

	if (!interface.Init ())
		 return false;

	(*logstream) << "Initialize Window Grid..." << std::endl;
	if (!windowgrid.Init ())
		 return false;

	(*logstream) << "Initialize Geometry..." << std::endl;
	if (!geometry.Init ())
		 return false;

	(*logstream) << "Initialize GBuffer..." << std::endl;
	if (!gbuffer.Init ())
		 return false;

	(*logstream) << "Initialize Filters..." << std::endl;
	if (!filters.Init ())
		 return false;

	(*logstream) << "Initialize Shadow Map..." << std::endl;
	if (!shadowmap.Init ())
		 return false;

	interface.AddLight (0);
	interface.AddShadow (0);

	(*logstream) << "Initialize Composition..." << std::endl;
	if (!composition.Init ())
		 return false;

	(*logstream) << "Initialize Final Pass..." << std::endl;
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

	Model::culled = 0;
	Culling::culled = 0;

	if (last_time == 0)
		 last_time = glfwGetTime ();
	else
	{
		timefactor = glfwGetTime () - last_time;
		last_time += timefactor;
	}

	camera.Frame (timefactor);
	culling.Frame ();
	gbuffer.Render (geometry);

	for (GLuint i = 0; i < shadows.size (); i++)
	{
		shadowmap.Render (i, geometry, shadows[i]);
	}

	composition.Frame (timefactor);

	finalpass.Render ();

	interface.Frame (timefactor);

	GL_CHECK_ERROR;
}
