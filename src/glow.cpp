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
#include "glow.h"
#include "renderer.h"
#include <vector>

Glow::Glow (Renderer *parent)
	: renderer (parent), limit (1.0f), exponent (1.0f), size (0)
{
}

Glow::~Glow (void)
{
}

bool Glow::Init (gl::Texture &screenmap, gl::Texture &gm,
								 GLuint mipmap_level)
{
	glowmap = &gm;
	{
		gl::Shader obj (GL_FRAGMENT_SHADER);
		std::string source;
		if (!ReadFile (MakePath ("shaders", "glow", "hblur.txt"), source))
			 return false;
		obj.Source (source);
		if (!obj.Compile ())
		{
			(*logstream) << "Could not compile "
									 << MakePath ("shaders", "glow", "hblur.txt")
									 << ": " << std::endl << obj.GetInfoLog () << std::endl;
			 return false;
		}

		hblur.prog.Parameter (GL_PROGRAM_SEPARABLE, GL_TRUE);
		hblur.prog.Attach (obj);
		if (!hblur.prog.Link ())
		{
			(*logstream) << "Could not link the shader program "
									 << MakePath ("shaders", "glow", "hblur.txt")
									 << ": " << std::endl << hblur.prog.GetInfoLog ()
									 << std::endl;
			 return false;
		}
	}
	{
		gl::Shader obj (GL_FRAGMENT_SHADER);
		std::string source;
		if (!ReadFile (MakePath ("shaders", "glow", "vblur.txt"), source))
			 return false;
		obj.Source (source);
		if (!obj.Compile ())
		{
			(*logstream) << "Could not compile "
									 << MakePath ("shaders", "glow", "vblur.txt")
									 << ": " << std::endl << obj.GetInfoLog () << std::endl;
			 return false;
		}

		vblur.prog.Parameter (GL_PROGRAM_SEPARABLE, GL_TRUE);
		vblur.prog.Attach (obj);
		if (!vblur.prog.Link ())
		{
			(*logstream) << "Could not link the shader program "
									 << MakePath ("shaders", "glow", "vblur.txt")
									 << ": " << std::endl << vblur.prog.GetInfoLog ()
									 << std::endl;
			 return false;
		}
	}
	{
		gl::Shader obj (GL_FRAGMENT_SHADER);
		std::string source;
		if (!ReadFile (MakePath ("shaders", "glow", "blend.txt"), source))
			 return false;
		obj.Source (source);
		if (!obj.Compile ())
		{
			(*logstream) << "Could not compile "
									 << MakePath ("shaders", "glow", "blend.txt")
									 << ": " << std::endl << obj.GetInfoLog () << std::endl;
			 return false;
		}

		blendprog.Parameter (GL_PROGRAM_SEPARABLE, GL_TRUE);
		blendprog.Attach (obj);
		if (!blendprog.Link ())
		{
			(*logstream) << "Could not link the shader program "
									 << MakePath ("shaders", "glow", "blend.txt")
									 << ": " << std::endl << blendprog.GetInfoLog ()
									 << std::endl;
			 return false;
		}
	}
	width = renderer->gbuffer.GetWidth () >> mipmap_level;
	height = renderer->gbuffer.GetHeight () >> mipmap_level;
	hblur.pipeline.UseProgramStages (GL_VERTEX_SHADER_BIT,
																	 renderer->windowgrid.vprogram);
	hblur.pipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT, hblur.prog);
	vblur.pipeline.UseProgramStages (GL_VERTEX_SHADER_BIT,
																	 renderer->windowgrid.vprogram);
	vblur.pipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT, vblur.prog);
	blendpipeline.UseProgramStages (GL_VERTEX_SHADER_BIT,
																	renderer->windowgrid.vprogram);
	blendpipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT, blendprog);

	blendprog["invviewport"] = glm::vec2 (1.0f / renderer->gbuffer.GetWidth (),
																				1.0f / renderer->gbuffer.GetHeight ());

	hblur.prog["invviewport"] = glm::vec2 (1.0f / width, 1.0f / height);
	vblur.prog["invviewport"] = glm::vec2 (1.0f / width, 1.0f / height);

	sampler.Parameter (GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	sampler.Parameter (GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	sampler.Parameter (GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	sampler.Parameter (GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	sampler2.Parameter (GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	sampler2.Parameter (GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	sampler2.Parameter (GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	sampler2.Parameter (GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	map.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F, width, height,
							 0, GL_RGBA, GL_FLOAT, NULL);
	map2.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F, width, height,
								0, GL_RGBA, GL_FLOAT, NULL);
#ifdef DEBUG
	renderer->memory += width * height * 4 * 4 * 2;
#endif

	hblur.fb.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
											map, 0);
	hblur.fb.DrawBuffers ({ GL_COLOR_ATTACHMENT0 });
	vblur.fb.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
											map2, 0);
	vblur.fb.DrawBuffers ({ GL_COLOR_ATTACHMENT0 });
	blendfb.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
										 screenmap, 0);
	blendfb.DrawBuffers ({ GL_COLOR_ATTACHMENT0 });

	SetSize (4);

	return true;
}

float compute_weight (unsigned long n, unsigned long k)
{
	long double tmp;
	tmp = 1.0 / (powl (2, n) - 2 * (1 + n));
	for (int i = 1; i <= k; i++)
	{
		tmp *= (n - k + i);
		tmp /= i;
	}
	return tmp;
	
}

void Glow::SetSize (GLuint s)
{
	std::vector<float> weights_data;
	std::vector<float> offsets_data;
	std::vector<float> data;

	if (!s)
	{
		size = 0;
		return;
	}
	size = (s&~3) + 1;

	if (!size)
		 return;

	for (int i = 0; i < (size+1)/2; i++)
	{
		weights_data.push_back
			 (compute_weight (size + 3, ((size - 1) / 2) - i + 2));
	}

	for (int i = 0; i < weights_data.size (); i++)
	{
		offsets_data.push_back (float (i));
	}

	data.push_back (weights_data[0]);
	data.push_back (offsets_data[0]);

	for (int i = 0; i < weights_data.size() >> 1; i++)
	{
		float weight;
		weight = weights_data[i * 2 + 1] +
			 weights_data[i * 2];
		data.push_back (weight);
		data.push_back ((offsets_data[i * 2 + 1]
										 * weights_data[i * 2 + 1]
										 + offsets_data[i * 2]
										 * weights_data[i * 2])
										/ weight);
	}

	buffer.Data (sizeof (GLfloat) * data.size (),
								&data[0], GL_STATIC_DRAW);

	buffertex.Buffer (GL_RG32F, buffer);
}

GLuint Glow::GetSize (void)
{
	return size;
}

GLfloat Glow::GetExponent (void)
{
	return exponent;
}

void Glow::SetExponent (GLfloat exp)
{
	exponent = exp;
}

GLfloat Glow::GetLimit (void)
{
	return limit;
}

void Glow::SetLimit (GLfloat l)
{
	limit = l;
}

const gl::Texture &Glow::GetMap (void)
{
	return map2;
}

void Glow::Apply (void)
{
	if (GetSize () == 0)
		 return;

	buffertex.Bind (GL_TEXTURE1, GL_TEXTURE_BUFFER);

	hblur.fb.Bind (GL_FRAMEBUFFER);
	hblur.pipeline.Bind ();
	gl::Viewport (0, 0, width, height);

	glowmap->Bind (GL_TEXTURE0, GL_TEXTURE_2D);
	sampler.Bind (0);
	renderer->windowgrid.Render ();

	vblur.fb.Bind (GL_FRAMEBUFFER);
	vblur.pipeline.Bind ();

	map.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
	sampler2.Bind (0);

	renderer->windowgrid.Render ();

	blendfb.Bind (GL_FRAMEBUFFER);
	blendpipeline.Bind ();
	gl::Viewport (0, 0, renderer->gbuffer.GetWidth (),
								renderer->gbuffer.GetHeight ());

	gl::Enable (GL_BLEND);
	gl::BlendFunc (GL_ONE, GL_ONE);
	gl::BlendEquation (GL_FUNC_ADD);
	map2.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
	sampler2.Bind (0);
	renderer->windowgrid.Render ();
	gl::Disable (GL_BLEND);

	gl::Framebuffer::Unbind (GL_FRAMEBUFFER);
	GL_CHECK_ERROR;
}
