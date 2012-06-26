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
	: glow (), luminance_threshold (0.75),
		sky ( { 3.0, 50.0, 142, 10.0 } )
{
}

Composition::~Composition (void)
{
}

bool Composition::Init (void)
{
	if (!info.Init ())
		 return false;

	std::string src;
	if (!ReadFile (MakePath ("kernels", "composition.cl"), src))
		 return false;

	program = r->clctx.CreateProgramWithSource (src);
	{
		std::string options ("-cl-fast-relaxed-math -cl-mad-enable "
												 "-cl-no-signed-zeros");
		if (r->clctx.IsExtensionSupported ("cl_nv_compiler_options"))
		{
			options.append (" -cl-nv-maxrregcount=32");
		}
		program.Build (options);
	}
	composition = program.CreateKernel ("composition");

	screen.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F,
									r->gbuffer.GetWidth (),
									r->gbuffer.GetHeight (),
									0, GL_RGBA, GL_FLOAT, NULL);
	glowmap.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F,
								r->gbuffer.GetWidth (),
								r->gbuffer.GetHeight (),
								0, GL_RGBA, GL_FLOAT, NULL);

	glowmap.GenerateMipmap (GL_TEXTURE_2D);

#ifdef DEBUG
	r->memory += r->gbuffer.GetWidth ()
		 * r->gbuffer.GetHeight () * (4 * 2 + 4 * 2 + 4);
	r->memory += (r->gbuffer.GetWidth () >> 1) *
		 (r->gbuffer.GetHeight () >> 1) * 4;
	r->memory += (r->gbuffer.GetWidth () >> 1) *
		 (r->gbuffer.GetHeight () >> 2) * 4;
  /* ignore smaller mipmap levels... */
#endif

	if (!glow.Init (screen, glowmap,
									config["glow"]["mipmaplevel"].as<unsigned int> (2)))
		 return false;

	screenmem = r->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, screen);
	glowmem = r->clctx.CreateFromGLTexture2D
		 (CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, glowmap);

	composition.SetArg (0, screenmem);
	composition.SetArg (1, glowmem);
	composition.SetArg (2, r->gbuffer.colormem);
	composition.SetArg (3, r->gbuffer.depthmem);
	composition.SetArg (4, r->gbuffer.normalmem);
	composition.SetArg (5, r->gbuffer.specularmem);
	composition.SetArg (6, r->shadowmap.GetMem ());
	composition.SetArg (7, r->gbuffer.fragidxmem);
	composition.SetArg (8, r->gbuffer.fraglistmem);

	composition.SetArg (9, r->GetLightMem ());

	composition.SetArg (10, info.GetMem ());

	cl_uint num_parameters = r->GetNumParameters ();
	composition.SetArg (11, sizeof (cl_uint), &num_parameters);
	composition.SetArg (12, r->GetParameterMem ());

	glow.SetSize (0);

	GeneratePerezCoefficients ();

	return true;
}

void Composition::SetLightMem (cl::Memory &m)
{
	composition.SetArg (9, m);
}

void Composition::GeneratePerezCoefficients (void)
{
	float T = sky.turbidity;
	sky.perezY[0] = 0.17872 * T - 1.46303;
	sky.perezY[1] = -0.35540 * T + 0.42749;
	sky.perezY[2] = -0.02266 * T + 5.32505;
	sky.perezY[3] = 0.12064 * T - 2.57705;
	sky.perezY[4] = -0.06696 * T + 0.37027;
	
	sky.perezx[0] = -0.01925 * T - 0.25922;
	sky.perezx[1] = -0.06651 * T + 0.00081;
	sky.perezx[2] = -0.00041 * T + 0.21247;
	sky.perezx[3] = -0.06409 * T - 0.89887;
	sky.perezx[4] = -0.00325 * T + 0.04517;

	sky.perezy[0] = -0.01669 * T - 0.26078;
	sky.perezy[1] = -0.09495 * T + 0.00921;
	sky.perezy[2] = -0.00792 * T + 0.21023;
	sky.perezy[3] = -0.04405 * T - 1.65369;
	sky.perezy[4] = -0.01092 * T + 0.05291;
}

float Composition::GetPerezY (int idx)
{
	return sky.perezY[idx];
}

void Composition::SetPerezY (int idx, float val)
{
	sky.perezY[idx] = val;
}


float Composition::GetPerezx (int idx)
{
	return sky.perezx[idx];
}

void Composition::SetPerezx (int idx, float val)
{
	sky.perezx[idx] = val;
}


float Composition::GetPerezy (int idx)
{
	return sky.perezy[idx];
}

void Composition::SetPerezy (int idx, float val)
{
	sky.perezy[idx] = val;
}

Glow &Composition::GetGlow (void)
{
	return glow;
}

void Composition::SetLuminanceThreshold (float threshold)
{
	luminance_threshold = threshold;
}

float Composition::GetLuminanceThreshold (void)
{
	return luminance_threshold;
}

void Composition::SetScreenLimit (float limit)
{
	info.SetScreenLimit (limit);
}

GLfloat Composition::GetScreenLimit (void)
{
	return info.GetScreenLimit ();
}

void Composition::SetShadowAlpha (float alpha)
{
	if (alpha < 0)
		 alpha = 0;
	if (alpha > 1)
		 alpha = 1;
	info.SetShadowAlpha (alpha);
}

float Composition::GetShadowAlpha (void)
{
	return info.GetShadowAlpha ();
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
	info.SetMode (m);
}

GLuint Composition::GetMode (void)
{
	return info.GetMode ();
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
}

int Composition::GetDate (void)
{
	return sky.date;
}

void Composition::SetDate (int d)
{
	sky.date = d;
}

float Composition::GetTimeOfDay (void)
{
	return sky.time;
}

void Composition::SetTimeOfDay (float t)
{
	sky.time = t;
}

glm::vec3 Composition::GetSunDirection (void)
{
	float theta, cos_theta;
	return GetSunDirection (theta, cos_theta);
}

glm::vec3 Composition::GetSunDirection (float &theta, float &cos_theta)
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
		theta = DRE_PI / 2.0 - solarAltitude;

		float sin_theta = sin (info.GetSunTheta ());
		cos_theta = cos (info.GetSunTheta ());
		return glm::vec3 (sin_theta * sin (phi),
											cos_theta,
											sin_theta * cos (phi));

}

void Composition::Frame (float timefactor)
{
	std::vector<cl::Memory> mem = { screenmem, glowmem,
																	r->gbuffer.colormem,
																	r->gbuffer.normalmem,
																	r->gbuffer.specularmem,
																	r->gbuffer.depthmem,
																	r->gbuffer.fraglistmem,
																	r->gbuffer.fragidxmem,
																	r->shadowmap.GetMem () };

	info.SetNumLights (r->GetNumLights ());

	info.SetProjInfo (r->camera.GetProjInfo ());
	info.SetInverseViewMatrix
		 (glm::transpose (glm::inverse (r->camera.GetViewMatrix ())));
	info.SetShadowMatrix (glm::transpose (r->shadowmap.GetMat ()));
	info.SetEye (glm::vec4 (r->camera.GetEye (), 0.0));
	info.SetCenter (glm::vec4 (r->camera.GetCenter (), 0.0));
	info.SetGlowThreshold (luminance_threshold);
	info.SetGlowSize (glow.GetSize ());
	info.SetGlowLimit (glow.GetLimit ());
	info.SetGlowExponent (glow.GetExponent ());

	{
		float theta, cos_theta;
		info.SetSunDirection 
			 (glm::vec4 (GetSunDirection (theta, cos_theta), 0));
		info.SetSunTheta (theta);
		float T = sky.turbidity;

		info.SetSkyPerezY (sky.perezY);
		info.SetSkyPerezx (sky.perezx);
		info.SetSkyPerezy (sky.perezy);

		{
			glm::vec4 zenithYxy;
			float chi = (4.0 / 9.0 - T / 120.0) * (M_PI - 2.0 * info.GetSunTheta ());
			zenithYxy.x = (4.0453 * T - 4.9710) * tan (chi)
				 - 0.2155 * T + 2.4192;
			glm::vec4 th;
			th.w = 1;
			th.z = info.GetSunTheta ();
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
			info.SetSkyZenithYxy (zenithYxy);
		}
	}

	const size_t work_dim[] = { r->gbuffer.GetWidth (),
															r->gbuffer.GetHeight () };
	const size_t local_dim[] = { 16, 16 };

	r->queue.EnqueueAcquireGLObjects (mem, 0, NULL, NULL);
	r->queue.EnqueueNDRangeKernel (composition, 2, NULL, work_dim,
																 local_dim, 0, NULL, NULL);
	r->queue.EnqueueReleaseGLObjects (mem, 0, NULL, NULL);

	if (glow.GetSize () > 0)
	{
		glowmap.GenerateMipmap (GL_TEXTURE_2D);
		glow.Apply ();
	}
}

Composition::Info::Info (void)
	: data ( { glm::vec4 (0), glm::mat4 (0), glm::mat4 (0),
				 glm::vec4 (0), glm::vec4 (0), { 0, 0.0f, 0.0f, 0.0f },
			{ glm::vec4 (0), 0.0f, 0.0f }, { 3.0f }, 0.7, 0, 0, 2.0 } )
{
}

Composition::Info::~Info (void)
{
}

bool Composition::Info::Init (void)
{
	mem = r->clctx.CreateBuffer (CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
															 sizeof (data), &data);
	return true;
}

void Composition::Info::SetProjInfo (const glm::vec4 &i)
{
	if (data.projinfo != i)
	{
		data.projinfo = i;
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.projinfo)
																 - intptr_t (&data), sizeof (data.projinfo),
																 &data.projinfo, 0, NULL, NULL);
	}
}

const glm::vec4 &Composition::Info::GetProjInfo (void)
{
	return data.projinfo;
}

void Composition::Info::SetInverseViewMatrix (const glm::mat4 &m)
{
	if (data.vmatinv != m)
	{
		data.vmatinv = m;
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.vmatinv)
																 - intptr_t (&data), sizeof (data.vmatinv),
																 &data.vmatinv, 0, NULL, NULL);
	}
}

const glm::mat4 &Composition::Info::GetInverseViewMatrix (void)
{
	return data.vmatinv;
}

void Composition::Info::SetShadowMatrix (const glm::mat4 &m)
{
	if (data.shadowmat != m)
	{
		data.shadowmat = m;
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.shadowmat)
																 - intptr_t (&data), sizeof (data.shadowmat),
																 &data.shadowmat, 0, NULL, NULL);
	}
}

const glm::mat4 &Composition::Info::GetShadowMatrix (void)
{
	return data.shadowmat;
}

void Composition::Info::SetEye (const glm::vec4 &e)
{
	if (data.eye != e)
	{
		data.eye = e;
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.eye)
																 - intptr_t (&data), sizeof (data.eye),
																 &data.eye, 0, NULL, NULL);
	}
}

const glm::vec4 &Composition::Info::GetEye (void)
{
	return data.eye;
}

void Composition::Info::SetCenter (const glm::vec4 &c)
{
	if (data.center != c)
	{
		data.center = c;
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.center)
																 - intptr_t (&data), sizeof (data.center),
																 &data.center, 0, NULL, NULL);
	}
}

const glm::vec4 &Composition::Info::GetCenter (void)
{
	return data.center;
}

void Composition::Info::SetGlowSize (GLuint s)
{
	if (data.glow.size != s)
	{
		data.glow.size = s;
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.glow.size)
																 - intptr_t (&data), sizeof (data.glow.size),
																 &data.glow.size, 0, NULL, NULL);
	}
}

GLuint Composition::Info::GetGlowSize (void)
{
	return data.glow.size;
}

void Composition::Info::SetGlowExponent (GLfloat e)
{
	if (data.glow.exponent != e)
	{
		data.glow.exponent = e;
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.glow.exponent)
																 - intptr_t (&data),
																 sizeof (data.glow.exponent),
																 &data.glow.exponent, 0, NULL, NULL);
	}
}

GLfloat Composition::Info::GetGlowExponent (void)
{
	return data.glow.exponent;
}

void Composition::Info::SetGlowThreshold (GLfloat t)
{
	if (data.glow.threshold != t)
	{
		data.glow.threshold = t;
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.glow.threshold)
																 - intptr_t (&data),
																 sizeof (data.glow.threshold),
																 &data.glow.threshold, 0, NULL, NULL);
	}
}

GLfloat Composition::Info::GetGlowThreshold (void)
{
	return data.glow.threshold;
}

void Composition::Info::SetGlowLimit (GLfloat l)
{
	if (data.glow.glowlimit != l)
	{
		data.glow.glowlimit = l;
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.glow.glowlimit)
																 - intptr_t (&data),
																 sizeof (data.glow.glowlimit),
																 &data.glow.glowlimit, 0, NULL, NULL);

	}
}

GLfloat Composition::Info::GetGlowLimit (void)
{
	return data.glow.glowlimit;
}

void Composition::Info::SetSunDirection (const glm::vec4 &d)
{
	if (data.sun.direction != d)
	{
		data.sun.direction = d;
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.sun.direction)
																 - intptr_t (&data),
																 sizeof (data.sun.direction),
																 &data.sun.direction, 0, NULL, NULL);

	}
}

const glm::vec4 &Composition::Info::GetSunDirection (void)
{
	return data.sun.direction;
}

void Composition::Info::SetSunTheta (GLfloat theta)
{
	if (data.sun.theta != theta)
	{
		data.sun.theta = theta;
		data.sun.cos_theta = cosf (theta);
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.sun.theta)
																 - intptr_t (&data), sizeof (data.sun.theta)
																 + sizeof (data.sun.cos_theta),
																 &data.sun.theta, 0, NULL, NULL);
	}
}

GLfloat Composition::Info::GetSunTheta (void)
{
	return data.sun.theta;
}

GLfloat Composition::Info::GetCosSunTheta (void)
{
	return data.sun.cos_theta;
}

void Composition::Info::SetSkyTurbidity (GLfloat T)
{
	if (data.sky.turbidity != T)
	{
		data.sky.turbidity = T;
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.sky.turbidity)
																 - intptr_t (&data),
																 sizeof (data.sky.turbidity),
																 &data.sky.turbidity, 0, NULL, NULL);

	}
}

GLfloat Composition::Info::GetSkyTurbidity (void)
{
	return data.sky.turbidity;
}

void Composition::Info::SetSkyPerezY (const GLfloat *p)
{
	for (auto i = 0; i < 5; i++)
	{
		if (data.sky.perezY[i] != p[i])
		{
			data.sky.perezY[i] = p[i];
			r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.sky.perezY[i])
																	 - intptr_t (&data),
																	 sizeof (data.sky.perezY[i]),
																	 &data.sky.perezY[i], 0, NULL, NULL);

		}
	}
}

const GLfloat *Composition::Info::GetSkyPerezY (void)
{
	return &data.sky.perezY[0];
}

void Composition::Info::SetSkyPerezx (const GLfloat *p)
{
	for (auto i = 0; i < 5; i++)
	{
		if (data.sky.perezx[i] != p[i])
		{
			data.sky.perezx[i] = p[i];
			r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.sky.perezx[i])
																	 - intptr_t (&data),
																	 sizeof (data.sky.perezx[i]),
																	 &data.sky.perezx[i], 0, NULL, NULL);

		}
	}
}

const GLfloat *Composition::Info::GetSkyPerezx (void)
{
	return &data.sky.perezx[0];
}

void Composition::Info::SetSkyPerezy (const GLfloat *p)
{
	for (auto i = 0; i < 5; i++)
	{
		if (data.sky.perezy[i] != p[i])
		{
			data.sky.perezy[i] = p[i];
			r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.sky.perezy[i])
																	 - intptr_t (&data),
																	 sizeof (data.sky.perezy[i]),
																	 &data.sky.perezy[i], 0, NULL, NULL);

		}
	}
}

const GLfloat *Composition::Info::GetSkyPerezy (void)
{
	return &data.sky.perezy[0];
}

void Composition::Info::SetSkyZenithYxy (const glm::vec4 &Yxy)
{
	if (data.sky.zenithYxy != Yxy)
	{
		data.sky.zenithYxy = Yxy;
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.sky.zenithYxy)
																 - intptr_t (&data),
																 sizeof (data.sky.zenithYxy),
																 &data.sky.zenithYxy, 0, NULL, NULL);

	}
}

const glm::vec4 &Composition::Info::GetSkyZenithYxy (void)
{
	return data.sky.zenithYxy;
}

void Composition::Info::SetShadowAlpha (GLfloat a)
{
	if (data.shadow_alpha != a)
	{
		data.shadow_alpha = a;
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.shadow_alpha)
																 - intptr_t (&data),
																 sizeof (data.shadow_alpha),
																 &data.shadow_alpha, 0, NULL, NULL);

	}
}

GLfloat Composition::Info::GetShadowAlpha (void)
{
	return data.shadow_alpha;
}

void Composition::Info::SetMode (GLuint m)
{
	if (data.mode != m)
	{
		data.mode = m;
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.mode)
																 - intptr_t (&data),
																 sizeof (data.mode),
																 &data.mode, 0, NULL, NULL);

	}
}

GLuint Composition::Info::GetMode (void)
{
	return data.mode;
}

void Composition::Info::SetNumLights (GLuint n)
{
	if (data.num_lights != n)
	{
		data.num_lights = n;
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.num_lights)
																 - intptr_t (&data),
																 sizeof (data.num_lights),
																 &data.num_lights, 0, NULL, NULL);
	}
}

GLuint Composition::Info::GetNumLights (void)
{
	return data.num_lights;
}

void Composition::Info::SetScreenLimit (GLfloat l)
{
	if (data.screenlimit != l)
	{
		data.screenlimit = l;
		r->queue.EnqueueWriteBuffer (mem, CL_TRUE, intptr_t (&data.screenlimit)
																 - intptr_t (&data),
																 sizeof (data.screenlimit),
																 &data.screenlimit, 0, NULL, NULL);

	}
}

GLfloat Composition::Info::GetScreenLimit (void)
{
	return data.screenlimit;
}

const cl::Memory &Composition::Info::GetMem (void)
{
	return mem;
}
