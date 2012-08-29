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
#include "glow.h"
#include "renderer.h"
#include <vector>

Glow::Glow (void)
	: limit (1.0f), exponent (1.0f), size (0)
{
}

Glow::~Glow (void)
{
}

bool Glow::Init (gl::Texture &screenmap, gl::Texture &gm,
								 GLuint mipmap_level)
{
	glowmap = &gm;
	if (!LoadProgram (hblur.prog, MakePath ("shaders", "bin", "glow_hblur.bin"),
										GL_FRAGMENT_SHADER, std::string (),
										{ MakePath ("shaders", "glow", "hblur.txt") }))
		 return false;
	if (!LoadProgram (vblur.prog, MakePath ("shaders", "bin", "glow_vblur.bin"),
										GL_FRAGMENT_SHADER, std::string (),
										{ MakePath ("shaders", "glow", "vblur.txt") }))
		 return false;
	if (!LoadProgram (blendprog, MakePath ("shaders", "bin", "glow_blend.bin"),
										GL_FRAGMENT_SHADER, std::string (),
										{ MakePath ("shaders", "glow", "blend.txt") }))
		 return false;

	width = r->gbuffer.GetWidth () >> mipmap_level;
	height = r->gbuffer.GetHeight () >> mipmap_level;
	hblur.pipeline.UseProgramStages (GL_VERTEX_SHADER_BIT,
																	 r->windowgrid.vprogram);
	hblur.pipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT, hblur.prog);
	vblur.pipeline.UseProgramStages (GL_VERTEX_SHADER_BIT,
																	 r->windowgrid.vprogram);
	vblur.pipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT, vblur.prog);
	blendpipeline.UseProgramStages (GL_VERTEX_SHADER_BIT,
																	r->windowgrid.vprogram);
	blendpipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT, blendprog);

	hblur.prog["invviewport"] = glm::vec2 (1.0f / width, 1.0f / height);
	vblur.prog["invviewport"] = glm::vec2 (1.0f / width, 1.0f / height);

	map.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F, width, height,
							 0, GL_RGBA, GL_FLOAT, NULL);
	map2.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F, width, height,
								0, GL_RGBA, GL_FLOAT, NULL);
#ifdef DEBUG
	r->memory += width * height * 4 * 2 * 2;
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
	std::vector<float> data;

	if (!s)
	{
		size = 0;
		return;
	}
	size = (s&~3) + 1;

	if (!size)
		 return;

	ComputeWeightOffsets (data, size);

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
	const gl::Sampler &mipmapsampler
		 = r->GetSampler (GL_LINEAR_MIPMAP_LINEAR,
											GL_LINEAR, GL_CLAMP_TO_EDGE,
											GL_CLAMP_TO_EDGE);
	const gl::Sampler &sampler = r->GetSampler (GL_LINEAR, GL_LINEAR,
																							GL_CLAMP_TO_EDGE,
																							GL_CLAMP_TO_EDGE);
	if (GetSize () == 0)
		 return;

	buffertex.Bind (GL_TEXTURE1, GL_TEXTURE_BUFFER);

	hblur.fb.Bind (GL_FRAMEBUFFER);
	hblur.pipeline.Bind ();
	gl::Viewport (0, 0, width, height);

	glowmap->Bind (GL_TEXTURE0, GL_TEXTURE_2D);
	mipmapsampler.Bind (0);
	r->windowgrid.Render ();

	vblur.fb.Bind (GL_FRAMEBUFFER);
	vblur.pipeline.Bind ();

	map.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
	sampler.Bind (0);

	r->windowgrid.Render ();

	blendfb.Bind (GL_FRAMEBUFFER);
	blendpipeline.Bind ();
	gl::Viewport (0, 0, r->gbuffer.GetWidth (),
								r->gbuffer.GetHeight ());

	gl::Enable (GL_BLEND);
	gl::BlendFunc (GL_ONE, GL_ONE);
	gl::BlendEquation (GL_FUNC_ADD);
	map2.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
	sampler.Bind (0);
	r->windowgrid.Render ();
	gl::Disable (GL_BLEND);

	gl::Framebuffer::Unbind (GL_FRAMEBUFFER);
	GL_CHECK_ERROR;
}
