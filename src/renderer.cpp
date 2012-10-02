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
#include <fstream>

Renderer::Renderer (void)
	: antialiasing (0)
#ifdef DEBUG
	,memory (0)
#endif
{
}

Renderer::~Renderer (void)
{
}

bool Renderer::Init (void)
{
	(*logstream) << glfwGetTime () << " Initialization starts..." << std::endl;

	gl::FrontFace (GL_CCW);
	gl::CullFace (GL_BACK);
	gl::Enable (GL_CULL_FACE);

	gl::Hint (GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);

	(*logstream) << glfwGetTime ()  << " Initialize Camera..." << std::endl;
	if (!camera.Init ())
		 return false;

	(*logstream) << glfwGetTime ()  << " Initialize Window Grid..." << std::endl;
	if (!windowgrid.Init ())
		 return false;

	(*logstream) << glfwGetTime () << " Initialize Geometry..." << std::endl;
	if (!geometry.Init ())
		 return false;

	(*logstream) << glfwGetTime () << " Initialize GBuffer..." << std::endl;
	if (!gbuffer.Init ())
		 return false;

	(*logstream) << glfwGetTime () << " Initialize Shadow Map..." << std::endl;
	if (!shadowmap.Init ())
		 return false;

	if (config["random_lights"].as<bool> (false))
	{
		float max = -100;

		srand (time (NULL));
		for (int y = -7; y <= 7; y++)
		{
			for (int x = -7; x <= 7; x++)
			{
				Light light;
				light.position = glm::vec4 (x * 3, 3, y * 3, 0);
				light.position.x += ((float (rand ()) / float (RAND_MAX))
														 - 0.5f) * 2.0f;
				if (light.position.x > max)
					 max = light.position.x;
				light.position.y += ((float (rand ()) / float (RAND_MAX))
														 - 0.5f) * 1.0f;
				light.position.z += ((float (rand ()) / float (RAND_MAX))
														 - 0.5f) * 2.0f;
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
				light.spot.angle = PCH_PI / 6.0f;
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
		lightbuffer.Data (sizeof (Light) * lights.size (),
											&lights[0], GL_STATIC_DRAW);
	}
	else
	{
		lightbuffer.Data (sizeof (glm::vec4),	NULL, GL_STATIC_DRAW);
	}

	{
		std::vector<YAML::Node> parameterlist;
		std::string filename (MakePath ("materials", "parameters.yaml"));
		std::ifstream file (filename, std::ifstream::in);
		if (!file.is_open ())
		{
			(*logstream) << "Cannot open " << filename << std::endl;
			return false;
		}

		parameterlist = YAML::LoadAll (file);

		for (const YAML::Node &node : parameterlist)
		{
			Parameter parameter;
			{
				YAML::Node specular = node["specular"];
				std::string model = specular["model"].as<std::string> ("none");
				if (!model.compare ("gaussian"))
				{
					parameter.specular.model = 1;
					parameter.specular.smoothness
						 = specular["smoothness"].as<float> (0.25f);
					parameter.specular.gaussfactor
						 = specular["gaussfactor"].as<float> (1.0f);
				}
				else if (!model.compare ("phong"))
				{
					parameter.specular.model = 2;
					parameter.specular.shininess
						 = specular["shininess"].as<float> (2.0f);
					parameter.specular.param2 = 1.0f;
				}
				else if (!model.compare ("beckmann"))
				{
					parameter.specular.model = 3;
					parameter.specular.smoothness
						 = specular["smoothness"].as<float> (0.25);
					parameter.specular.param2 = 1.0f;
				}
				else
				{
					if (model.compare ("none"))
						 (*logstream) << "The parameter file " << filename
													<< " contains an unknown specular model:"
													<< model << std::endl;
					parameter.specular.model = 0;
					parameter.specular.param1 = 0.25f;
					parameter.specular.param2 = 1.0f;
				}
				parameter.specular.fresnel.n
					 = specular["fresnel"]["n"].as<float> (0.0f);
				parameter.specular.fresnel.k
					 = specular["fresnel"]["k"].as<float> (0.0f);
			}
			{
				YAML::Node reflection = node["reflection"];
				parameter.reflection.factor
					 = reflection["factor"].as<float> (0.0f);
				parameter.reflection.fresnel.n
					 = reflection["fresnel"]["n"].as<float> (0.0f);
				parameter.reflection.fresnel.k
					 = reflection["fresnel"]["k"].as<float> (0.0f);
			}
			parameters.push_back (parameter);
			parameter_names.push_back (node["name"].as<std::string> ("unnamed"));
		}
	}
	parameterbuffer.Data (sizeof (Parameter) * parameters.size (),
												&parameters[0], GL_STATIC_DRAW);

	(*logstream) << glfwGetTime () << " Initialize Composition..." << std::endl;
	if (!composition.Init ())
		 return false;

	(*logstream) << glfwGetTime () << " Initialize Postprocessing..."
							 << std::endl;
	if (!postprocess.Init ())
		 return false;

	gl::ReleaseShaderCompiler ();
	GL_CHECK_ERROR;

	(*logstream) << glfwGetTime () << " Initialization complete." << std::endl;

	return true;
}

void Renderer::Resize (int w, int h)
{
	camera.Resize (w, h);
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

Parameter &Renderer::GetParameters (GLuint param)
{
	if (param >= parameters.size ())
		 throw std::runtime_error ("Parameter index out of bounds.");
	return parameters[param];
}

void Renderer::UpdateParameters (GLuint param)
{
	parameterbuffer.SubData (sizeof (Parameter) * param,
													 sizeof (Parameter), &parameters[param]);
}

const gl::Buffer &Renderer::GetParameterBuffer (void) const
{
	return parameterbuffer;
}

GLuint Renderer::GetNumParameters (void)
{
	return parameters.size ();
}

const std::string &Renderer::GetParameterName (GLuint param)
{
	return parameter_names[param];
}

Light &Renderer::GetLight (GLuint light)
{
	if (light >= lights.size ())
		 throw std::runtime_error ("Light index out of bounds.");
	return lights[light];
}

GLuint Renderer::GetNumLights (void)
{
	return lights.size ();
}

void Renderer::RemoveLight (GLuint light)
{
	if (lights.size () == 0)
		 return;
	if (lights.size () == 1)
	{
		lights.erase (lights.begin () + light);
		lightbuffer.Data (sizeof (glm::vec4),	NULL, GL_STATIC_DRAW);
		return;
	}
	lights.erase (lights.begin () + light);
	lightbuffer.Data (sizeof (Light) * lights.size (),
										&lights[0], GL_STATIC_DRAW);
}

void Renderer::AddLight (const Light &light)
{
	lights.push_back (light);
	lights.back ().CalculateFrustum ();
	lightbuffer.Data (sizeof (Light) * lights.size (),
					  &lights[0], GL_STATIC_DRAW);
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
	lightbuffer.SubData (intptr_t (&lights[light]) - intptr_t (&lights[0]),
											 sizeof (lights[light]), &lights[light]);
}

Renderer::SamplerParams::SamplerParams (GLint _minfilter,
																				GLint _magfilter,
																				GLint _wrap_s,
																				GLint _wrap_t)
	: minfilter (_minfilter), magfilter (_magfilter),
		wrap_s (_wrap_s), wrap_t (_wrap_t)
{
}

bool Renderer::SamplerParams::operator< (const SamplerParams &params) const
{
	if (minfilter < params.minfilter)
		 return true;
	if (params.minfilter < minfilter)
		 return false;
	if (magfilter < params.magfilter)
		 return true;
	if (params.magfilter < magfilter)
		 return false;
	if (wrap_s < params.wrap_s)
		 return true;
	if (params.wrap_s < wrap_s)
		 return false;
	return wrap_t < params.wrap_t;
}

const gl::Sampler &Renderer::GetSampler (GLint minfilter, GLint magfilter,
																				 GLint wrap_s, GLint wrap_t)
{
	SamplerParams params (minfilter, magfilter, wrap_s, wrap_t);
	auto it = samplers.find (params);
	if (it == samplers.end ())
	{
		auto it = samplers.insert (std::make_pair (params, gl::Sampler ()));
		it.first->second.Parameter (GL_TEXTURE_MIN_FILTER, minfilter);
		it.first->second.Parameter (GL_TEXTURE_MAG_FILTER, magfilter);
		it.first->second.Parameter (GL_TEXTURE_WRAP_S, wrap_s);
		it.first->second.Parameter (GL_TEXTURE_WRAP_T, wrap_t);
		return it.first->second;
	}
	else
	{
		return it->second;
	}
}

gl::Buffer &Renderer::GetLightBuffer (void)
{
	return lightbuffer;
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
	camera.GetBuffer ().BindBase (GL_UNIFORM_BUFFER, 0);

	culling.Frame ();

	gbuffer.Render (geometry);	

	{
		Shadow sunshadow;
		sunshadow.direction = -glm::vec4 (composition.sun.direction.Get (),
																			 0.0f);
		sunshadow.position = -sunshadow.direction * 25;
		
		if (composition.sun.cos_theta.Get () < 0.05)
			 shadowmap.Clear ();
		else
			 shadowmap.Render (0, geometry, sunshadow);
	}

	composition.Frame (timefactor);

	postprocess.Frame ();

	GL_CHECK_ERROR;
}
