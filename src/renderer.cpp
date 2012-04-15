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
#include <ctime>

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

	gl::Hint (GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);

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

	srand (time (NULL));
	for (int y = -7; y <= 7; y++)
	{
		for (int x = -7; x <= 7; x++)
		{
			Light light;
			light.position = glm::vec4 (x * 3, 3, y * 3, 0);
			light.direction = glm::vec4 (0, -1, 0, 0);
			const glm::vec4 colors[] = {
				glm::vec4 (0, 0, 1, 1),
				glm::vec4 (0, 1, 0, 1),
				glm::vec4 (1, 0, 0, 1),
				glm::vec4 (0, 1, 1, 1),
				glm::vec4 (1, 1, 0, 1),
				glm::vec4 (1, 0, 1, 1),
			};
			
			light.color = colors[rand () % 6];
			light.spot.angle = M_PI/8.0f;
			light.spot.cosine = cosf (light.spot.angle);
			light.spot.exponent = 42.0f;
			light.spot.tangent = tanf (light.spot.angle * 1.2);
			light.spot.inner_angle = light.spot.angle * 0.8;
			light.spot.inner_cosine = cosf (light.spot.inner_angle);
			light.specular.color = glm::vec3 (light.color);
			light.specular.shininess = 8.0f;
			light.attenuation = glm::vec4 (0.0f, 0.0f, 0.07f, 50.0f);
			lights.push_back (light);
		}
	}
	lightmem = clctx.CreateBuffer
		 (CL_MEM_READ_ONLY,	sizeof (Light) * lights.size (), NULL);
	queue.EnqueueWriteBuffer
		 (lightmem, CL_TRUE, 0,
			sizeof (Light) * lights.size (),
			&lights[0], 0, NULL, NULL);

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
