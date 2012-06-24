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

Composition::Composition (Renderer *parent)
	: renderer (parent), glow (parent),
		luminance_threshold (0.75),
		sky ( { 3.0, 50.0, 142, 10.0 } ),
		info ( { glm::vec4 (0), glm::mat4 (0), glm::mat4 (0),
					 glm::vec4 (0), glm::vec4 (0), { 0, 0.0f, 0.0f, 0.0f },
				{}, 0.7, 0, 0, 2.0 } )
{
}

Composition::~Composition (void)
{
}

bool Composition::Init (void)
{
	std::string src;
	if (!ReadFile (MakePath ("kernels", "composition.cl"), src))
		 return false;

	program = renderer->clctx.CreateProgramWithSource (src);
	{
		std::string options ("-cl-fast-relaxed-math -cl-mad-enable "
												 "-cl-no-signed-zeros");
		if (renderer->clctx.IsExtensionSupported ("cl_nv_compiler_options"))
		{
			options.append (" -cl-nv-maxrregcount=32");
		}
		program.Build (options);
	}
	composition = program.CreateKernel ("composition");

	screen.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F,
									renderer->gbuffer.GetWidth (),
									renderer->gbuffer.GetHeight (),
									0, GL_RGBA, GL_FLOAT, NULL);
	glowmap.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F,
								renderer->gbuffer.GetWidth (),
								renderer->gbuffer.GetHeight (),
								0, GL_RGBA, GL_FLOAT, NULL);

	glowmap.GenerateMipmap (GL_TEXTURE_2D);

#ifdef DEBUG
	renderer->memory += renderer->gbuffer.GetWidth ()
		 * renderer->gbuffer.GetHeight () * (4 * 2 + 4 * 2 + 4);
	renderer->memory += (renderer->gbuffer.GetWidth () >> 1) *
		 (renderer->gbuffer.GetHeight () >> 1) * 4;
	renderer->memory += (renderer->gbuffer.GetWidth () >> 1) *
		 (renderer->gbuffer.GetHeight () >> 2) * 4;
  /* ignore smaller mipmap levels... */
#endif

	if (!glow.Init (screen, glowmap,
									config["glow"]["mipmaplevel"].as<unsigned int> (2)))
		 return false;

	screenmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, screen);
	glowmem = renderer->clctx.CreateFromGLTexture2D
		 (CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, glowmap);

	composition.SetArg (0, screenmem);
	composition.SetArg (1, glowmem);
	composition.SetArg (2, renderer->gbuffer.colormem);
	composition.SetArg (3, renderer->gbuffer.depthmem);
	composition.SetArg (4, renderer->gbuffer.normalmem);
	composition.SetArg (5, renderer->gbuffer.specularmem);
	composition.SetArg (6, renderer->shadowmap.GetMem ());
	composition.SetArg (7, renderer->gbuffer.fragidxmem);
	composition.SetArg (8, renderer->gbuffer.fraglistmem);

	cl_uint num_parameters = renderer->GetNumParameters ();
	composition.SetArg (11, sizeof (cl_uint), &num_parameters);
	composition.SetArg (12, renderer->GetParameterMem ());

	glow.SetSize (0);

	GeneratePerezCoefficients ();

	return true;
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
	info.screenlimit = limit;
}

GLfloat Composition::GetScreenLimit (void)
{
	return info.screenlimit;
}

void Composition::SetShadowAlpha (float alpha)
{
	if (alpha < 0)
		 alpha = 0;
	if (alpha > 1)
		 alpha = 1;
	info.shadow_alpha = alpha;
}

float Composition::GetShadowAlpha (void)
{
	return info.shadow_alpha;
}

const gl::Texture &Composition::GetScreen (void)
{
	return screen;
}

#define NUM_COMPOSITIONMODES   3

void Composition::SetMode (GLuint m)
{
	info.mode = m;
	if (info.mode >= NUM_COMPOSITIONMODES)
		 info.mode = NUM_COMPOSITIONMODES - 1;
}

GLuint Composition::GetMode (void)
{
	return info.mode;
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

		float sin_theta = sin (info.sky.sun.theta);
		cos_theta = cos (info.sky.sun.theta);
		return glm::vec3 (sin_theta * sin (phi),
											info.sky.sun.cos_theta,
											sin_theta * cos (phi));

}

void Composition::Frame (float timefactor)
{
	std::vector<cl::Memory> mem = { screenmem, glowmem,
																	renderer->gbuffer.colormem,
																	renderer->gbuffer.normalmem,
																	renderer->gbuffer.specularmem,
																	renderer->gbuffer.depthmem,
																	renderer->gbuffer.fraglistmem,
																	renderer->gbuffer.fragidxmem,
																	renderer->shadowmap.GetMem () };

	info.num_lights = renderer->GetNumLights ();

	info.projinfo = renderer->camera.GetProjInfo ();
	info.vmatinv = glm::transpose
		 (glm::inverse (renderer->camera.GetViewMatrix ()));
	info.shadowmat = glm::transpose
		 (renderer->shadowmap.GetMat ());
	info.eye = glm::vec4 (renderer->camera.GetEye (), 0.0);
	info.center = glm::vec4 (renderer->camera.GetCenter (), 0.0);
	info.glow.threshold = luminance_threshold;
	info.glow.size = glow.GetSize ();
	info.glow.glowlimit = glow.GetLimit ();
	info.glow.exponent = glow.GetExponent ();

	{
		info.sky.sun.direction = glm::vec4
			 (GetSunDirection (info.sky.sun.theta, info.sky.sun.cos_theta), 0);
		float T = sky.turbidity;
		info.sky.turbidity = T;

		for (auto i = 0; i < 5; i++)
		{
			info.sky.perezY[i] = sky.perezY[i];
			info.sky.perezx[i] = sky.perezx[i];
			info.sky.perezy[i] = sky.perezy[i];
		}

		{
			float chi = (4.0 / 9.0 - T / 120.0) * (M_PI - 2.0 * info.sky.sun.theta);
			info.sky.zenithYxy.x = (4.0453 * T - 4.9710) * tan (chi)
				 - 0.2155 * T + 2.4192;
		}
		{
			glm::vec4 th;
			th.w = 1;
			th.z = info.sky.sun.theta;
			th.y = th.z * th.z;
			th.x = th.y * th.z;

			glm::vec3 T3 (T * T, T, 1);

			glm::mat4x3 matx (0.00165, -0.02902, 0.11693,
											 -0.00374, 0.06377, -0.21196,
											 0.00208, -0.03202, 0.06052,
											 0, 0.00394, 0.25886);

			info.sky.zenithYxy.y = glm::dot (T3, matx * th);

			glm::mat4x3 maty (0.00275, -0.04214, 0.15346,
												-0.00610, 0.08970, -0.26756,
												0.00316, -0.04153, 0.06669,
												0, 0.00515, 0.26688);

			info.sky.zenithYxy.z = glm::dot (T3, maty * th);
		}
	}

	composition.SetArg (9, renderer->GetLightMem ());
	composition.SetArg (10, sizeof (Info), &info);

	const size_t work_dim[] = { renderer->gbuffer.GetWidth (),
															renderer->gbuffer.GetHeight () };
	const size_t local_dim[] = { 16, 16 };

	renderer->queue.EnqueueAcquireGLObjects (mem, 0, NULL, NULL);
	renderer->queue.EnqueueNDRangeKernel (composition, 2, NULL, work_dim,
																				local_dim, 0, NULL, NULL);
	renderer->queue.EnqueueReleaseGLObjects (mem, 0, NULL, NULL);

	if (glow.GetSize () > 0)
	{
		glowmap.GenerateMipmap (GL_TEXTURE_2D);
		glow.Apply ();
	}
}
