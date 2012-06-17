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
	hblur.pipeline.UseProgramStages (GL_VERTEX_SHADER_BIT,
																	 renderer->windowgrid.vprogram);
	hblur.pipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT, hblur.prog);
	vblur.pipeline.UseProgramStages (GL_VERTEX_SHADER_BIT,
																	 renderer->windowgrid.vprogram);
	vblur.pipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT, vblur.prog);

	hblur.prog["invviewport"] = glm::vec2 (1.0f / renderer->gbuffer.GetWidth (),
																				 1.0f / renderer->gbuffer.GetHeight ());
	vblur.prog["invviewport"] = glm::vec2 (1.0f / renderer->gbuffer.GetWidth (),
																				 1.0f / renderer->gbuffer.GetHeight ());

	sampler.Parameter (GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	sampler.Parameter (GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	sampler.Parameter (GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	sampler.Parameter (GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	map.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F,
							 renderer->gbuffer.GetWidth (),
							 renderer->gbuffer.GetHeight (),
							 0, GL_RGBA, GL_FLOAT, NULL);
#ifdef DEBUG
	renderer->memory += renderer->gbuffer.GetWidth ()
		 * renderer->gbuffer.GetHeight () * 4 * 4;
#endif

	hblur.fb.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
											map, 0);
	hblur.fb.DrawBuffers ({ GL_COLOR_ATTACHMENT0 });
	vblur.fb.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
											screenmap, 0);
	vblur.fb.DrawBuffers ({ GL_COLOR_ATTACHMENT0 });

	SetSize (4);

	return true;
}

long double Coef (unsigned long n, unsigned long k)
{
	long double ret;
	ret = 1.0 / powl (2.0, n);
	for (unsigned long i = 0; i < k; i++)
	{
		ret *= (long double) (n - i);
		ret /= (long double) (i+1);
	}
	return ret;
}

void nextrow (std::vector<unsigned long> &row)
{
	std::vector<unsigned long> nextrow;
	nextrow.push_back (1);
	for (int i = 1; i < row.size (); i++)
	{
		nextrow.push_back (row[i-1] + row[i]);
	}
	nextrow.push_back (1);

	row = std::move (nextrow);
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

	std::vector<unsigned long> pascal;

	for (int i = 0; i < size + 3; i++)
		 nextrow (pascal);

	unsigned long sum = (8UL<<size) - 2 * (pascal[0] + pascal[1]);

	for (int i = 0; i < (size+1)/2; i++)
	{
		weights_data.push_back ((long double)(pascal[((size - 1) / 2) - i+2])
														/ (long double)(sum));
	}

	for (int i = 0; i < weights_data.size (); i++)
	{
		offsets_data.push_back (float (i));
	}

	data.push_back (weights_data[0]);
	data.push_back (offsets_data[0]);

	for (int i = 1; i <= weights_data.size() >> 1; i++)
	{
		float weight;
		weight = weights_data[i * 2 - 1] +
			 weights_data[i * 2];
		data.push_back (weight);
		data.push_back ((offsets_data[i * 2 - 1]
										 * weights_data[i * 2 - 1]
										 + offsets_data[i * 2]
										 * weights_data[i * 2])
										/ weight);
	}

	buffer.Data (sizeof (GLfloat) * data.size (),
								&data[0], GL_STATIC_DRAW);

	buffertex.Buffer (GL_RG32F, buffer);

	hblur.prog["size"] = (GLuint)(data.size () >> 1);
	vblur.prog["size"] = (GLuint)(data.size () >> 1);
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
	return map;
}

void Glow::Apply (void)
{
	if (GetSize () == 0)
		 return;

	buffertex.Bind (GL_TEXTURE1, GL_TEXTURE_BUFFER);

	hblur.fb.Bind (GL_FRAMEBUFFER);
	hblur.pipeline.Bind ();
	gl::Viewport (0, 0, renderer->gbuffer.GetWidth (),
								renderer->gbuffer.GetHeight ());

	glowmap->Bind (GL_TEXTURE0, GL_TEXTURE_2D);
	sampler.Bind (0);
	renderer->windowgrid.Render ();

	vblur.fb.Bind (GL_FRAMEBUFFER);
	vblur.pipeline.Bind ();

	map.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
	sampler.Bind (0);

	gl::Enable (GL_BLEND);
	gl::BlendFunc (GL_ONE, GL_ONE);
	gl::BlendEquation (GL_FUNC_ADD);
	renderer->windowgrid.Render ();
	gl::Disable (GL_BLEND);

	gl::Framebuffer::Unbind (GL_FRAMEBUFFER);
	GL_CHECK_ERROR;
}
