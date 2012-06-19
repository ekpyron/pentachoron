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
#include <fstream>

Renderer::Renderer (void)
	: geometry (this), shadowpass (this),
		finalpass (this), gbuffer (this),
#ifdef DEBUG
		memory (0),
#endif
		interface (this), composition (this), antialiasing (0),
		culling (this), camera (this)
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

#ifdef DEBUG
	{
		size_t len;
		clctx.GetDeviceInfo (CL_DEVICE_EXTENSIONS, 0, NULL, &len);
		std::vector<char> ext;
		ext.resize (len);
		clctx.GetDeviceInfo (CL_DEVICE_EXTENSIONS, len, &ext[0], NULL);
		std::cout << "OpenCL Extensions: " << &ext[0] << std::endl;
	}
#endif

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

	(*logstream) << "Initialize Shadow Pass..." << std::endl;
	if (!shadowpass.Init ())
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
			light.CalculateFrustum ();
			lights.push_back (light);
		}
	}
	lightmem = clctx.CreateBuffer
		 (CL_MEM_READ_ONLY,	sizeof (Light) * lights.size (), NULL);
	queue.EnqueueWriteBuffer
		 (lightmem, CL_TRUE, 0,
			sizeof (Light) * lights.size (),
			&lights[0], 0, NULL, NULL);

	interface.AddLight ();
	interface.AddShadow ();

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
			{
				YAML::Node specular = node["specular"];
				float default_smoothness;
				std::string model = specular["model"].as<std::string> ("none");
				if (!model.compare ("gaussian"))
				{
					parameter.specular.model = 1;
					default_smoothness = 0.25;
				}
				else if (!model.compare ("phong"))
				{
					parameter.specular.model = 2;
					default_smoothness = 2.0;
				}
				else if (!model.compare ("beckmann"))
				{
					parameter.specular.model = 3;
					default_smoothness = 0.25;
				}
				else if (!model.compare ("cooktorrance"))
				{
					parameter.specular.model = 4;
					default_smoothness = 0.25;
					parameter.specular.fresnel = specular["fresnel"].as<float> (1.0);
				}
				else
				{
					(*logstream) << "The parameter file " << filename
											 << " contains an unknown specular model:"
											 << model << std::endl;
					parameter.specular.model = 0;
				}
				parameter.specular.smoothness = specular["smoothness"].as<float>
					 (default_smoothness);
			}
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

	(*logstream) << "Initialization complete." << std::endl;

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

void Renderer::SetAntialiasing (GLuint value)
{
	antialiasing = value;
	gbuffer.SetAntialiasing (value);
}

GLuint Renderer::GetAntialiasing (void)
{
	return antialiasing;
}

Light &Renderer::GetLight (GLuint light)
{
	return lights[light];
}

GLuint Renderer::GetNumLights (void)
{
	return lights.size ();
}

void Renderer::RemoveLight (GLuint light)
{
	if (lights.size () < 2)
		 return;
	lights.erase (lights.begin () + light);
	queue.EnqueueWriteBuffer (lightmem, CL_TRUE, 0,
														sizeof (Light) * lights.size (),
														&lights[0], 0, NULL, NULL);
}

void Renderer::AddLight (const Light &light)
{
	lights.push_back (light);
	lights.back ().CalculateFrustum ();
	lightmem = clctx.CreateBuffer (CL_MEM_READ_ONLY,
																 sizeof (Light) * lights.size (),
																 NULL);
	queue.EnqueueWriteBuffer (lightmem, CL_TRUE, 0,
														sizeof (Light) * lights.size (),
														&lights[0], 0, NULL, NULL);
}

void Renderer::UpdateLight (GLuint light)
{
	lights[light].spot.cosine = cosf (lights[light].spot.angle);
	lights[light].spot.tangent = tanf (lights[light].spot.angle);
	lights[light].spot.penumbra_cosine = cosf (lights[light].spot.penumbra_angle);
	lights[light].direction = glm::normalize (lights[light].direction);
	lights[light].color = glm::clamp (lights[light].color, 0.0f, 1.0f);
	lights[light].specular.color = glm::clamp (lights[light].specular.color,
																						 0.0f, 1.0f);
	if (lights[light].attenuation.x < 0)
		 lights[light].attenuation.x = 0;
	if (lights[light].attenuation.y < 0)
		 lights[light].attenuation.y = 0;
	if (lights[light].attenuation.z < 0)
		 lights[light].attenuation.z = 0;
	if (lights[light].attenuation.w < 0)
		 lights[light].attenuation.w = 0;
	lights[light].CalculateFrustum ();
	queue.EnqueueWriteBuffer (lightmem, CL_TRUE,
														intptr_t (&lights[light]) - intptr_t (&lights[0]),
														sizeof (lights[light]), &lights[light],
														0, NULL, NULL);
}

const cl::Memory &Renderer::GetLightMem (void)
{
	return lightmem;
}

GLuint Renderer::GetNumParameters (void)
{
	return parameters.size ();
}

const cl::Memory &Renderer::GetParameterMem (void)
{
	return parametermem;
}

extern bool running;

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

	shadowpass.Render (0, geometry, shadows[0]);

	composition.Frame (timefactor);

	finalpass.Render ();

	interface.Frame (timefactor);

	GL_CHECK_ERROR;
}
