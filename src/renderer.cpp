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
#include <fstream>

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

	float max = -100;

	srand (time (NULL));
	for (int y = -7; y <= 7; y++)
	{
		for (int x = -7; x <= 7; x++)
		{
			Light light;
			light.position = glm::vec4 (x * 3, 3, y * 3, 0);
			light.position.x += ((float (rand ()) / float (RAND_MAX)) - 0.5f) * 2.0f;
			if (light.position.x > max)
				 max = light.position.x;
			light.position.y += ((float (rand ()) / float (RAND_MAX)) - 0.5f) * 1.0f;
			light.position.z += ((float (rand ()) / float (RAND_MAX)) - 0.5f) * 2.0f;
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
			light.spot.angle = DRE_PI / 6.0f;
			light.spot.cosine = cosf (light.spot.angle);
			light.spot.exponent = 30.0f;
			light.spot.tangent = tanf (light.spot.angle * 1.2);
			light.spot.penumbra_angle = light.spot.angle * 0.8;
			light.spot.penumbra_cosine = cosf (light.spot.penumbra_angle);
			light.specular.color = glm::vec3 (light.color);
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

	{
		YAML::Node parameterlist;
		std::string filename (MakePath ("materials", "parameters.yaml"));
		std::ifstream file (filename, std::ifstream::in);
		if (!file.is_open ())
		{
			(*logstream) << "Cannot open " << filename << std::endl;
			return false;
		}
		parameterlist = YAML::Load (file);

		if (!parameterlist.IsSequence ())
		{
			(*logstream) << "The parameter file " << filename
									 << " has an invalid format." << std::endl;
		}

		for (const YAML::Node &node : parameterlist)
		{
			Parameter parameter;
			parameter.specular.exponent = node["specular_exponent"].as<float> (2.0f);
			parameters.push_back (parameter);
		}
		parametermem = clctx.CreateBuffer
		 (CL_MEM_READ_ONLY,	sizeof (Parameter) * parameters.size (), NULL);
		queue.EnqueueWriteBuffer
			 (parametermem, CL_TRUE, 0,
				sizeof (Parameter) * parameters.size (),
				&parameters[0], 0, NULL, NULL);
	}

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
