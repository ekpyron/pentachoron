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
#include "composition.h"
#include "renderer.h"

Composition::Composition (void)
	: glow (),
		sky ( { 3.0, 50.0, 142, 10.0 } )
{
}

Composition::~Composition (void)
{
}

bool Composition::Init (void)
{
	{
		std::stringstream stream;
		stream << "#version 420 core" << std::endl;
		stream << "#define MAX_DEPTH_LAYERS "
					 << config["max_depth_layers"].as<GLuint> (4) << std::endl;
		stream << "#define SIZEOF_PARAMETER "
					 << sizeof (Parameter) / sizeof (glm::vec4)
					 << std::endl;
		stream << "#define SIZEOF_LIGHT "
					 << sizeof (Light) / sizeof (glm::vec4)
					 << std::endl;
		stream << "#define NUM_TILES_X "
					 << (r->gbuffer.GetWidth () >> 5)
					 << std::endl;
		stream << "#define NUM_TILES_Y "
					 << (r->gbuffer.GetHeight () >> 5)
					 << std::endl;
		std::vector<std::string> sources;
		const char *sourcefiles [] = {
			"header.txt", "light.txt", "parameter.txt", "specular.txt",
			"getpos.txt", "sky.txt", "shadow.txt", "composition.txt"
		};
		for (auto i = 0; i < sizeof (sourcefiles) / sizeof (sourcefiles[0]); i++)
		{
			sources.push_back (MakePath ("shaders", "composition",
																	 sourcefiles[i]));
		}
		if (!LoadProgram (fprogram, MakePath ("shaders", "bin", "composition.bin"),
											GL_FRAGMENT_SHADER, stream.str (), sources))
			 return false;
	}

	{
		std::stringstream stream;
		stream << "#version 420 core" << std::endl;
		stream << "#define SIZEOF_LIGHT "
					 << sizeof (Light) / sizeof (glm::vec4)
					 << std::endl;
		stream << "#define NUM_TILES_X "
					 << (r->gbuffer.GetWidth () >> 5)
					 << std::endl;
		stream << "#define NUM_TILES_Y "
					 << (r->gbuffer.GetHeight () >> 5)
					 << std::endl;
		if (!LoadProgram (lightcullprog, MakePath ("shaders", "bin",
																							 "lightculling.bin"),
											GL_FRAGMENT_SHADER, stream.str (), {
												MakePath ("shaders", "lightculling.txt") }))
			 return false;
	}

	if (!LoadProgram (minmaxdepthprog, MakePath ("shaders", "bin",
																							 "minmaxdepth.bin"),
										GL_FRAGMENT_SHADER, std::string (), {
											MakePath ("shaders", "minmaxdepth.txt") }))
		 return false;

	tile_based = gl::SmartUniform<bool>
		 (fprogram["tile_based"], true);
	luminance_threshold = gl::SmartUniform<GLfloat>
		 (fprogram["glow.threshold"], 0.75);
	screenlimit = gl::SmartUniform<GLfloat>
		 (fprogram["screenlimit"], 2.0);
	shadow_alpha = gl::SmartUniform<GLfloat>
		 (fprogram["shadow_alpha"], 0.7);
	shadowmat = gl::SmartUniform<glm::mat4>
		 (fprogram["shadowmat"], glm::mat4 (1));
	eye = gl::SmartUniform<glm::vec3>
		 (fprogram["eye"], glm::vec3 (0));
	sun.theta = gl::SmartUniform<GLfloat>
		 (fprogram["sun.theta"], 0.0f);
	sun.cos_theta = gl::SmartUniform<GLfloat>
		 (fprogram["sun.cos_theta"], 1.0f);
	sun.direction = gl::SmartUniform<glm::vec3>
		 (fprogram["sun.direction"], glm::vec3 ());

	sky.perezY = gl::SmartUniform<std::array<GLfloat, 5>>
		 (fprogram["sky.perezY"], std::array<GLfloat, 5> ());
	sky.perezx = gl::SmartUniform<std::array<GLfloat, 5>>
		 (fprogram["sky.perezx"], std::array<GLfloat, 5> ());
	sky.perezy = gl::SmartUniform<std::array<GLfloat, 5>>
		 (fprogram["sky.perezy"], std::array<GLfloat, 5> ());
	sky.zenithYxy = gl::SmartUniform<glm::vec3>
		 (fprogram["sky.zenithYxy"], glm::vec3 ());
	sky.luminosity = gl::SmartUniform<GLfloat>
		 (fprogram["sky.luminosity"], 0.04f);

	SetupSunPosition ();
	GeneratePerezCoefficients ();

	fprogram["invviewport"]
		 = glm::vec2 (1.0f / float (r->gbuffer.GetWidth ()),
									1.0f / float (r->gbuffer.GetHeight ()));
	minmaxdepthprog["invviewport"]
		 = glm::vec2 (1.0f / float (r->gbuffer.GetWidth ()),
									1.0f / float (r->gbuffer.GetHeight ()));
	lightcullprog["invviewport"]
		 = glm::vec2 (1.0f / float (r->gbuffer.GetWidth ()),
									1.0f / float (r->gbuffer.GetHeight ()));

	pipeline.UseProgramStages (GL_VERTEX_SHADER_BIT,
														 r->windowgrid.vprogram);
	pipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT,
														 fprogram);

	minmaxdepthpipeline.UseProgramStages (GL_VERTEX_SHADER_BIT,
																				r->windowgrid.vprogram);
	minmaxdepthpipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT,
																				minmaxdepthprog);

	lightcullpipeline.UseProgramStages (GL_VERTEX_SHADER_BIT,
																			r->windowgrid.vprogram);
	lightcullpipeline.UseProgramStages (GL_FRAGMENT_SHADER_BIT,
																			lightcullprog);

	screen.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F,
									r->gbuffer.GetWidth (),
									r->gbuffer.GetHeight (),
									0, GL_RGBA, GL_FLOAT, NULL);
#ifdef DEBUG
	r->memory += r->gbuffer.GetWidth () * r->gbuffer.GetHeight () * 4 * 2;
#endif

	glowmap.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F,
								r->gbuffer.GetWidth (),
								r->gbuffer.GetHeight (),
								0, GL_RGBA, GL_FLOAT, NULL);
	glowmap.GenerateMipmap (GL_TEXTURE_2D);
#ifdef DEBUG
	for (size_t mem = r->gbuffer.GetWidth () * r->gbuffer.GetHeight () * 4 * 2;
			 mem > 1; mem >>= 2)
		 r->memory += mem;
#endif

	if (!glow.Init (screen, glowmap,
									config["glow"]["mipmaplevel"].as<unsigned int> (2)))
		 return false;

	framebuffer.Texture2D (GL_COLOR_ATTACHMENT0,
												 GL_TEXTURE_2D,
												 screen, 0);
	framebuffer.Texture2D (GL_COLOR_ATTACHMENT1,
												 GL_TEXTURE_2D,
												 glowmap, 0);
	framebuffer.DrawBuffers ({ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 });

	lighttex.Image2D (GL_TEXTURE_2D, 0, GL_R16UI, r->gbuffer.GetWidth (),
										r->gbuffer.GetHeight (), 0, GL_RED_INTEGER,
										GL_UNSIGNED_INT, NULL);
#ifdef DEBUG
	r->memory += r->gbuffer.GetWidth () * r->gbuffer.GetHeight () * 2;
#endif

	numlights.Data (sizeof (GLuint)
									* (r->gbuffer.GetWidth () >> 5)
									* (r->gbuffer.GetHeight () >> 5),
									NULL,	GL_DYNAMIC_DRAW);

	dummy.Image2D (GL_TEXTURE_2D, 0, GL_R8, r->gbuffer.GetWidth (),
								 r->gbuffer.GetHeight (), 0, GL_RED,
								 GL_UNSIGNED_BYTE, NULL);
#ifdef DEBUG
	r->memory += r->gbuffer.GetWidth () * r->gbuffer.GetHeight ();
#endif

	lightcullfb.Texture2D (GL_COLOR_ATTACHMENT0,
												 GL_TEXTURE_2D,
												 dummy, 0);
	lightcullfb.DrawBuffers ({ });

	mindepthtex.Image2D (GL_TEXTURE_2D, 0, GL_R32F,
											 r->gbuffer.GetWidth () >> 5,
											 r->gbuffer.GetHeight () >> 5,
											 0, GL_RED,
											 GL_FLOAT, NULL);
#ifdef DEBUG
	r->memory += (r->gbuffer.GetWidth () >> 5)
		 * (r->gbuffer.GetHeight () >> 5) * 4;
#endif

	maxdepthtex.Image2D (GL_TEXTURE_2D, 0, GL_R32F,
											 r->gbuffer.GetWidth ()  >> 5,
											 r->gbuffer.GetHeight () >> 5,
											 0, GL_RED,
											 GL_FLOAT, NULL);
#ifdef DEBUG
	r->memory += (r->gbuffer.GetWidth () >> 5)
		 * (r->gbuffer.GetHeight () >> 5) * 4;
#endif

	minmaxdepthfb.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
													 mindepthtex, 0);
	minmaxdepthfb.Texture2D (GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
													 maxdepthtex, 0);
	minmaxdepthfb.DrawBuffers ({ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 });

	lightbuffertex.Buffer (GL_RGBA32F, r->GetLightBuffer ());

	clearfb.Texture2D (GL_COLOR_ATTACHMENT0,
										 GL_TEXTURE_2D,
										 mindepthtex, 0);
	clearfb.Texture2D (GL_COLOR_ATTACHMENT1,
										 GL_TEXTURE_2D,
										 maxdepthtex, 0);

	
	clearfb.DrawBuffers ({ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
				 GL_COLOR_ATTACHMENT2 });

	glow.SetSize (0);

	GeneratePerezCoefficients ();

	return true;
}

void Composition::SetTileBased (bool tb)
{
	tile_based.Set (tb);
}

bool Composition::GetTileBased (void)
{
	return tile_based.Get ();
}

void Composition::GeneratePerezCoefficients (void)
{
	float T = sky.turbidity;
	std::array<GLfloat, 5> perezY, perezx, perezy;
	perezY[0] = 0.17872 * T - 1.46303;
	perezY[1] = -0.35540 * T + 0.42749;
	perezY[2] = -0.02266 * T + 5.32505;
	perezY[3] = 0.12064 * T - 2.57705;
	perezY[4] = -0.06696 * T + 0.37027;
	sky.perezY.Set (perezY);
	
	perezx[0] = -0.01925 * T - 0.25922;
	perezx[1] = -0.06651 * T + 0.00081;
	perezx[2] = -0.00041 * T + 0.21247;
	perezx[3] = -0.06409 * T - 0.89887;
	perezx[4] = -0.00325 * T + 0.04517;
	sky.perezx.Set (perezx);

	perezy[0] = -0.01669 * T - 0.26078;
	perezy[1] = -0.09495 * T + 0.00921;
	perezy[2] = -0.00792 * T + 0.21023;
	perezy[3] = -0.04405 * T - 1.65369;
	perezy[4] = -0.01092 * T + 0.05291;
	sky.perezy.Set (perezy);
}

float Composition::GetPerezY (int idx)
{
	return sky.perezY.Get ()[idx];
}

void Composition::SetPerezY (int idx, float val)
{
	std::array<GLfloat, 5> perezY;
	perezY = sky.perezY.Get ();
	perezY[idx] = val;
	sky.perezY.Set (perezY);
}


float Composition::GetPerezx (int idx)
{
	return sky.perezx.Get ()[idx];
}

void Composition::SetPerezx (int idx, float val)
{
	std::array<GLfloat, 5> perezx;
	perezx = sky.perezx.Get ();
	perezx[idx] = val;
	sky.perezx.Set (perezx);
}


float Composition::GetPerezy (int idx)
{
	return sky.perezy.Get ()[idx];
}

void Composition::SetPerezy (int idx, float val)
{
	std::array<GLfloat, 5> perezy;
	perezy = sky.perezy.Get ();
	perezy[idx] = val;
	sky.perezy.Set (perezy);
}

Glow &Composition::GetGlow (void)
{
	return glow;
}

void Composition::SetLuminanceThreshold (float threshold)
{
	luminance_threshold.Set (threshold);
}

float Composition::GetLuminanceThreshold (void)
{
	return luminance_threshold.Get ();
}

void Composition::SetScreenLimit (float limit)
{
	screenlimit.Set (limit);
}

GLfloat Composition::GetScreenLimit (void)
{
	return screenlimit.Get ();
}

void Composition::SetShadowAlpha (float alpha)
{
	if (alpha < 0)
		 alpha = 0;
	if (alpha > 1)
		 alpha = 1;
	shadow_alpha.Set (alpha);
}

float Composition::GetShadowAlpha (void)
{
	return shadow_alpha.Get ();
}

const gl::Texture &Composition::GetScreen (void)
{
	return screen;
}

#define NUM_COMPOSITIONMODES   3

void Composition::SetMode (GLuint m)
{
	if (m >= NUM_COMPOSITIONMODES)
		 m = NUM_COMPOSITIONMODES - 1;
//	info.SetMode (m);
}

GLuint Composition::GetMode (void)
{
//	return info.GetMode ();
}

float Composition::GetTurbidity (void)
{
	return sky.turbidity;
}

void Composition::SetTurbidity (float f)
{
	sky.turbidity = f;
	GeneratePerezCoefficients ();
}

float Composition::GetLatitude (void)
{
	return sky.latitude;
}

void Composition::SetLatitude (float l)
{
	sky.latitude = l;
	SetupSunPosition ();
}

int Composition::GetDate (void)
{
	return sky.date;
}

void Composition::SetDate (int d)
{
	sky.date = d;
	SetupSunPosition ();
}

float Composition::GetTimeOfDay (void)
{
	return sky.time;
}

void Composition::SetTimeOfDay (float t)
{
	sky.time = t;
	SetupSunPosition ();
}

void Composition::SetupSunPosition (void)
{
		float latitude = sky.latitude * DRE_PI / 180.0;

		int day = sky.date;

		float solarTime = sky.time +
			 (0.170 * sin (4 * DRE_PI * (sky.date - 80) / 373)
				- 0.129 * sin (2 * DRE_PI * (sky.date - 8) / 355));
		float solarDeclination;
		solarDeclination = (0.4093 * sin (2 * DRE_PI * (sky.date - 81) / 368));
		float solarAltitude = asin (sin (latitude) * sin (solarDeclination)
																- cos (latitude) * cos (solarDeclination)
																* cos (DRE_PI * solarTime / 12));
		float opp = -cos (solarDeclination) * sin (DRE_PI * solarTime / 12);
		float adj = -(cos (latitude) * sin (solarDeclination)
									+ sin (latitude) * cos (solarDeclination)
									* cos (DRE_PI * solarTime / 12));

		float phi = -atan2 (opp, adj);
		float theta = DRE_PI / 2.0 - solarAltitude;

		float sin_theta = sin (theta);
		float cos_theta = cos (theta);

		sun.theta.Set (theta);
		sun.cos_theta.Set (cos_theta);

		sun.direction.Set (glm::vec3 (sin_theta * sin (phi),
																	cos_theta,
																	sin_theta * cos (phi)));
		SetupSkyZenithYxy ();
}

void Composition::SetupSkyZenithYxy (void)
{
	glm::vec3 zenithYxy;
	float T = sky.turbidity;
	float chi = (4.0 / 9.0 - T / 120.0) * (DRE_PI - 2.0 * sun.theta.Get ());
	zenithYxy.x = (4.0453 * T - 4.9710) * tan (chi)
		 - 0.2155 * T + 2.4192;
	glm::vec4 th;
	th.w = 1;
	th.z = sun.theta.Get ();
	th.y = th.z * th.z;
	th.x = th.y * th.z;
	
	glm::vec3 T3 (T * T, T, 1);
	
	glm::mat4x3 matx (0.00165, -0.02902, 0.11693,
										-0.00374, 0.06377, -0.21196,
										0.00208, -0.03202, 0.06052,
										0, 0.00394, 0.25886);

	zenithYxy.y = glm::dot (T3, matx * th);
	
	glm::mat4x3 maty (0.00275, -0.04214, 0.15346,
										-0.00610, 0.08970, -0.26756,
										0.00316, -0.04153, 0.06669,
										0, 0.00515, 0.26688);
	
	zenithYxy.z = glm::dot (T3, maty * th);

	sky.zenithYxy.Set (zenithYxy);
}

GLfloat Composition::GetSkyLuminosity (void)
{
	return sky.luminosity.Get ();
}

void Composition::SetSkyLuminosity (GLfloat l)
{
	sky.luminosity.Set (l);
}

void Composition::Frame (float timefactor)
{
/*
	info.eye = glm::vec4 (r->camera.GetEye (), 0.0);
	info.center = glm::vec4 (r->camera.GetCenter (), 0.0);
	info.glow.threshold = luminance_threshold;
	info.glow.size = glow.GetSize ();
	info.glow.glowlimit = glow.GetLimit ();
	info.glow.exponent = glow.GetExponent ();
*/
	shadowmat.Set (r->shadowmap.GetMat ());
	eye.Set (r->camera.GetEye ());

	if (GetTileBased ())
	{
		clearfb.Bind (GL_FRAMEBUFFER);
		gl::ClearBufferfv (GL_COLOR, 0, (const GLfloat[]) { 1.0f, 0, 0, 0 });
		gl::ClearBufferfv (GL_COLOR, 1, (const GLfloat[]) { 0.0f, 0, 0, 0 });
		
		minmaxdepthfb.Bind (GL_FRAMEBUFFER);
		gl::Viewport (0, 0, r->gbuffer.GetWidth () >> 5,
									r->gbuffer.GetHeight () >> 5);
		minmaxdepthpipeline.Bind ();

		r->windowgrid.sampler.Bind (0);
		r->gbuffer.depthbuffer.Bind (GL_TEXTURE0, GL_TEXTURE_2D);

		r->windowgrid.sampler.Bind (1);
		r->gbuffer.fragidx.Bind (GL_TEXTURE1, GL_TEXTURE_2D);

		r->windowgrid.sampler.Bind (2);
		r->gbuffer.fraglisttex.Bind (GL_TEXTURE2, GL_TEXTURE_BUFFER);

		gl::BlendFunc (GL_SRC_COLOR, GL_DST_COLOR);
		gl::BlendEquationi (0, GL_MIN);
		gl::BlendEquationi (1, GL_MAX);
		gl::Enable (GL_BLEND);

		for (auto y = 0; y < 32; y++)
		{
			for (auto x = 0; x < 32; x++)
			{
				minmaxdepthprog["offset"] = glm::uvec2 (x, y);
				r->windowgrid.Render ();
			}
		}

		gl::Disable (GL_BLEND);

		lightcullfb.Bind (GL_FRAMEBUFFER);
		gl::Viewport (0, 0, r->gbuffer.GetWidth (),
									r->gbuffer.GetHeight ());
		
		GLuint *ptr = (GLuint*) numlights.MapRange
			 (0, sizeof (GLuint) * (r->gbuffer.GetWidth () >> 5)
				* (r->gbuffer.GetHeight () >> 5),
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
				| GL_MAP_UNSYNCHRONIZED_BIT);
		for (auto i = 0; i < (r->gbuffer.GetWidth () >> 5)
						* (r->gbuffer.GetHeight () >> 5); i++)
		{
			ptr[i] = 0;
		}
		numlights.Unmap ();
		lightcullpipeline.Bind ();

		lightcullprog["vmatinv"] = glm::inverse (r->camera.GetViewMatrix ());
		lightcullprog["projinfo"] = r->camera.GetProjInfo ();

		numlights.BindBase (GL_ATOMIC_COUNTER_BUFFER, 0);
		mindepthtex.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		maxdepthtex.Bind (GL_TEXTURE1, GL_TEXTURE_2D);
		lightbuffertex.Bind (GL_TEXTURE2, GL_TEXTURE_BUFFER);
		lighttex.BindImage (0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16UI);
		r->windowgrid.Render ();
	}


	framebuffer.Bind (GL_FRAMEBUFFER);
	pipeline.Bind ();

	fprogram["vmatinv"] = glm::inverse (r->camera.GetViewMatrix ());
	fprogram["projinfo"] = r->camera.GetProjInfo ();

	gl::Viewport (0, 0, r->gbuffer.GetWidth (),
								r->gbuffer.GetHeight ());

	r->windowgrid.sampler.Bind (0);
	r->gbuffer.colorbuffer.Bind (GL_TEXTURE0, GL_TEXTURE_2D);

	r->windowgrid.sampler.Bind (1);
	r->gbuffer.depthbuffer.Bind (GL_TEXTURE1, GL_TEXTURE_2D);

	r->windowgrid.sampler.Bind (2);
	r->gbuffer.normalbuffer.Bind (GL_TEXTURE2, GL_TEXTURE_2D);

	r->windowgrid.sampler.Bind (3);
	r->gbuffer.specularbuffer.Bind (GL_TEXTURE3, GL_TEXTURE_2D);

	r->windowgrid.sampler.Bind (4);
	r->shadowmap.GetMap ().Bind (GL_TEXTURE4, GL_TEXTURE_2D);

	r->windowgrid.sampler.Bind (5);
	r->gbuffer.fragidx.Bind (GL_TEXTURE5, GL_TEXTURE_2D);

	r->windowgrid.sampler.Bind (6);
	r->gbuffer.fraglisttex.Bind (GL_TEXTURE6, GL_TEXTURE_BUFFER);

	r->windowgrid.sampler.Bind (7);
	lightbuffertex.Bind (GL_TEXTURE7, GL_TEXTURE_BUFFER);

	r->windowgrid.sampler.Bind (8);
	lighttex.Bind (GL_TEXTURE8, GL_TEXTURE_2D);

	r->windowgrid.sampler.Bind (9);
	r->GetParameterTexture ().Bind (GL_TEXTURE9, GL_TEXTURE_BUFFER);

	r->windowgrid.Render ();

	gl::Framebuffer::Unbind (GL_FRAMEBUFFER);

	if (glow.GetSize () > 0)
	{
		glowmap.GenerateMipmap (GL_TEXTURE_2D);
		glow.Apply ();
	}
}
