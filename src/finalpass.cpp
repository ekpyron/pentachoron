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
#include "finalpass.h"
#include "gbuffer.h"
#include "renderer.h"

FinalPass::FinalPass (void)
	: rendermode (0), 
		tonemapping ({ 0.18f, 0.9f, 1.0f, 1.0f, 0, 0,
				{ 0.5f, 0.0f, 0.01f, 0.0f } })
{
}

FinalPass::~FinalPass (void)
{
}

const std::vector<glm::mat3x3> RGB2XYZ = {
// Adobe RGB (1998)
	glm::mat3x3 (0.5767309f, 0.1855540f, 0.1881852f,
							 0.2973769f, 0.6273491f, 0.0752741f,
							 0.0270343f, 0.0706872f, 0.9911085f),
// AppleRGB
	glm::mat3x3 (0.4497288f, 0.3162486f, 0.1844926f,
							 0.2446525f, 0.6720283f, 0.0833192f,
							 0.0251848f, 0.1411824f, 0.9224628f),
// Best RGB
	glm::mat3x3 (0.6326696f, 0.2045558f, 0.1269946f,
							 0.2284569f, 0.7373523f, 0.0341908f,
							 0.0000000f, 0.0095142f, 0.8156958f),
// Beta RGB
	glm::mat3x3 (0.6712537f, 0.1745834f, 0.1183829f,
							 0.3032726f, 0.6637861f, 0.0329413f,
							 0.0000000f, 0.0407010f, 0.7845090f),
// Bruce RGB
	glm::mat3x3 (0.4674162f, 0.2944512f, 0.1886026f,
							 0.2410115f, 0.6835475f, 0.0754410f,
							 0.0219101f, 0.0736128f, 0.9933071f),
// CIE RGB
	glm::mat3x3 (0.4887180f, 0.3106803f, 0.2006017f,
							 0.1762044f, 0.8129847f, 0.0108109f,
							 0.0000000f, 0.0102048f, 0.9897952f),
// ColorMatch RGB
	glm::mat3x3 (0.5093439f, 0.3209071f, 0.1339691f,
							 0.2748840f, 0.6581315f, 0.0669845f,
							 0.0242545f, 0.1087821f, 0.6921735f),
// Don RGB 4
	glm::mat3x3 (0.6457711f, 0.1933511f, 0.1250978f,
							 0.2783496f, 0.6879702f, 0.0336802f,
							 0.0037113f, 0.0179861f, 0.8035125),
// ECI RGB
	glm::mat3x3 (0.6502043f, 0.1780774f, 0.1359384f,
							 0.3202499f, 0.6020711f, 0.0776791f,
							 0.0000000f, 0.0678390f, 0.7573710f),
// Ekta Space PS5
	glm::mat3x3 (0.5938914f, 0.2729801f, 0.0973485f,
							 0.2606286f, 0.7349465f, 0.0044249f,
							 0.0000000f, 0.0419969f, 0.7832131f),
// NTSC RGB
	glm::mat3x3 (0.6068909f, 0.1735011f, 0.2003480f,
							 0.2989164f, 0.5865990f, 0.1144845f,
							 0.0000000f, 0.0660957f, 1.1162243f),
// PAL/SECAM RGB
	glm::mat3x3 (0.4306190f, 0.3415419f, 0.1783091f,
							 0.2220379f, 0.7066384f, 0.0713236f,
							 0.0201853f, 0.1295504f, 0.9390944f),
// ProPhoto RGB
	glm::mat3x3 (0.7976749f, 0.1351917f, 0.0313534f,
							 0.2880402f, 0.7118741f, 0.0000857f,
							 0.0000000f, 0.0000000f, 0.8252100f),
// SMPTE-C RGB
	glm::mat3x3 (0.3935891f, 0.3652497f, 0.1916313f,
							 0.2124132f, 0.7010437f, 0.0865432f,
							 0.0187423f, 0.1119313f, 0.9581563f),
// sRGB
	glm::mat3x3 (0.4124564f, 0.3575761f, 0.1804375f,
							 0.2126729f, 0.7151522f, 0.0721750f,
							 0.0193339f, 0.1191920f, 0.9503041f),
// Wide Gamut RGB
	glm::mat3x3 (0.7161046f, 0.1009296f, 0.1471858f,
							 0.2581874f, 0.7249378f, 0.0168748f,
							 0.0000000f, 0.0517813f, 0.7734287f)
};

const std::vector<glm::mat3x3> XYZ2RGB = {
// Adobe RGB (1998)
	glm::mat3x3 (2.0413690f, -0.5649464f,-0.3446944f,
							 -0.9692660f, 1.8760108f, 0.0415560f,
							 0.0134474f, -0.1183897f, 1.0154096f),
// AppleRGB
	glm::mat3x3 (2.9515373f, -1.2894116f,-0.4738445f,
							 -1.0851093f, 1.9908566f, 0.0372026f,
							 0.0854934f, -0.2694964f, 1.0912975f),
// Best RGB
	glm::mat3x3 (1.7552599f, -0.4836786f,-0.2530000f,
							 -0.5441336f, 1.5068789f, 0.0215528f,
							 0.0063467f, -0.0175761f, 1.2256959f),
// Beta RGB
	glm::mat3x3 (1.6832270f, -0.4282363f,-0.2360185,
							 -0.7710229f, 1.7065571f, 0.0446900,
							 0.0400013f, -0.0885376f, 1.2723640),
// Bruce RGB
	glm::mat3x3 (2.7454669f, -1.1358136f,-0.4350269f,
							 -0.9692660f, 1.8760108f, 0.0415560f,
							 0.0112723f, -0.1139754f, 1.0132541f),
// CIE RGB
	glm::mat3x3 (2.3706743f, -0.9000405f,-0.4706338f,
							 -0.5138850f, 1.4253036f, 0.0885814f,
							 0.0052982f, -0.0146949f, 1.0093968f),
// ColorMatch RGB
	glm::mat3x3 (2.6422874f, -1.2234270f,-0.3930143f,
							 -1.1119763f, 2.0590183f, 0.0159614f,
							 0.0821699f, -0.2807254f, 1.4559877f),
// Don RGB 4
	glm::mat3x3 (1.7603902f, -0.4881198f, -0.2536126f,
							 -0.7126288f, 1.6527432f,  0.0416715f,
							 0.0078207f, -0.0347411f,  1.2447743f),
// ECI RGB
	glm::mat3x3 (1.7827618f, -0.4969847f, -0.2690101f,
							-0.9593623f,  1.9477962f, -0.0275807f,
							 0.0859317f, -0.1744674f,  1.3228273f),
// Ekta Space PS5
	glm::mat3x3 (2.0043819f, -0.7304844f, -0.2450052f,
							-0.7110285f,  1.6202126f,  0.0792227f,
							 0.0381263f, -0.0868780f,  1.2725438f),
// NTSC RGB
	glm::mat3x3 (1.9099961f, -0.5324542f, -0.2882091f,
							-0.9846663f,  1.9991710f, -0.0283082f,
							 0.0583056f, -0.1183781f,  0.8975535f),
// PAL/SECAM RGB
	glm::mat3x3 (3.0628971f, -1.3931791f, -0.4757517f,
							-0.9692660f,  1.8760108f,  0.0415560f,
							 0.0678775f, -0.2288548f,  1.0693490f),
// ProPhoto RGB
	glm::mat3x3 (1.3459433f, -0.2556075f, -0.0511118f,
							-0.5445989f,  1.5081673f,  0.0205351f,
							 0.0000000f,  0.0000000f,  1.2118128f),
// SMPTE-C RGB
	glm::mat3x3 (3.5053960f, -1.7394894f, -0.5439640f,
							-1.0690722f,  1.9778245f,  0.0351722f,
							 0.0563200f, -0.1970226f,  1.0502026f),
// sRGB
	glm::mat3x3 (3.2404542f, -1.5371385f, -0.4985314f,
							-0.9692660f,  1.8760108f,  0.0415560f,
							 0.0556434f, -0.2040259f,  1.0572252f),
// Wide Gamut RGB
	glm::mat3x3 (1.4628067f, -0.1840623f, -0.2743606f,
							-0.5217933f,  1.4472381f,  0.0677227f,
							 0.0349342f, -0.0968930f,  1.2884099f)
};

bool FinalPass::Init (void)
{
	std::vector<const char *> fprogram_sources = {
		"compose.txt", "normal.txt", "passthrough.txt", "shadowmap.txt",
		"glow.txt", "depth.txt", "edge.txt", "luminance.txt"
	};
	for (const char *&filename : fprogram_sources)
	{
		gl::Shader obj (GL_FRAGMENT_SHADER);
		std::string source;
		if (!ReadFile (MakePath ("shaders", "finalpass", filename), source))
			 return false;
		obj.Source (source);
		if (!obj.Compile ())
		{
			(*logstream) << "Could not compile "
									 << MakePath ("shaders", "finalpass", filename)
									 << ": " << std::endl << obj.GetInfoLog () << std::endl;
			 return false;
		}

		fprograms.emplace_back ();
		fprograms.back ().Parameter (GL_PROGRAM_SEPARABLE, GL_TRUE);
		fprograms.back ().Attach (obj);
		if (!fprograms.back ().Link ())
		{
			(*logstream) << "Could not link the shader program "
									 << MakePath ("shaders", "finalpass", filename)
									 << ": " << std::endl << fprograms.back ().GetInfoLog ()
									 << std::endl;
			 return false;
		}

		gl::SmartUniform<glm::uvec2> uniform (fprograms.back ()["viewport"],
																					r->camera.GetViewport ());
		viewport_uniforms.push_back (uniform);
	}

	luminance.Image2D (GL_TEXTURE_2D, 0, GL_R32F,
										 r->gbuffer.GetWidth (),
										 r->gbuffer.GetHeight (),
										 0, GL_RED, GL_FLOAT, NULL);
#ifdef DEBUG
	r->memory += r->gbuffer.GetWidth ()
		 * r->gbuffer.GetHeight () * 4;
#endif

	sampler.Parameter (GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	sampler.Parameter (GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	sampler.Parameter (GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	sampler.Parameter (GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	framebuffer.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
												 luminance, 0);
	framebuffer.DrawBuffers ({ GL_COLOR_ATTACHMENT0 });

	for (gl::Program &fprogram : fprograms)
	{
		pipelines.emplace_back ();
		pipelines.back ().UseProgramStages (GL_VERTEX_SHADER_BIT,
																				r->windowgrid.vprogram);
		pipelines.back ().UseProgramStages (GL_FRAGMENT_SHADER_BIT, fprogram);

		fprogram["gbufferdim"] = glm::uvec2 (r->gbuffer.GetWidth (),
																				 r->gbuffer.GetHeight ());
		fprogram["nearClipPlane"] = r->camera.GetNearClipPlane ();
		fprogram["farClipPlane"] = r->camera.GetFarClipPlane ();
	}

	{
		struct {
			 glm::mat3x4 RGB2XYZ;
			 glm::mat3x4 XYZ2RGB;
			 GLfloat image_key;
			 GLfloat white_threshold;
			 GLfloat sigma;
			 GLfloat n;

			 GLfloat constant;
			 GLfloat linear;
			 GLfloat lod;
		} data;
		data.RGB2XYZ = glm::mat3x4 (RGB2XYZ[tonemapping.rgb_working_space]);
		data.image_key = tonemapping.image_key;

		data.white_threshold = tonemapping.white_threshold;
		data.XYZ2RGB = glm::mat3x4 (XYZ2RGB[tonemapping.rgb_working_space]);

		data.sigma = powf (tonemapping.sigma, tonemapping.n);
		data.n = tonemapping.n;
		data.constant = tonemapping.avgLum.constant;
		data.linear = tonemapping.avgLum.linear;
		data.lod = tonemapping.avgLum.lod;
		tonemappingBuffer.Data (sizeof (data), &data, GL_STATIC_DRAW);
		fprograms.back ()["delta"] = tonemapping.avgLum.delta;
	}

	antialiasing = gl::SmartUniform<GLuint> (fprograms[0]["antialiasing"],
																					 r->GetAntialiasing ());
	antialiasing_threshold 
		 = gl::SmartUniform<GLfloat> (fprograms[0]["antialiasing_threshold"],
																	0.02f);
	glow = gl::SmartUniform<GLint> (fprograms[0]["glow"],
																	r->composition.
																	GetGlow ().GetSize () > 0);

	return true;
}

void FinalPass::SetAntialiasingThreshold (GLfloat threshold)
{
	antialiasing_threshold.Set (threshold);
}

GLfloat FinalPass::GetAntialiasingThreshold (void)
{
	return antialiasing_threshold.Get ();
}

void FinalPass::SetRenderMode (GLuint mode)
{
	rendermode = mode;
}

GLuint FinalPass::GetRenderMode (void)
{
	return rendermode;
}

void FinalPass::SetImageKey (float key)
{
	tonemapping.image_key = key;
	tonemappingBuffer.SubData (sizeof (glm::mat3x4) * 2, sizeof (GLfloat), &key);
}

float FinalPass::GetImageKey (void)
{
	return tonemapping.image_key;
}

void FinalPass::SetWhiteThreshold (float threshold)
{
	tonemapping.white_threshold = threshold;
	tonemappingBuffer.SubData (sizeof (glm::mat3x4) * 2 + sizeof (GLfloat),
														 sizeof (GLfloat),
														 &threshold);
}

float FinalPass::GetWhiteThreshold (void)
{
	return tonemapping.white_threshold;
}

void FinalPass::SetTonemappingSigma (float sigma)
{
	tonemapping.sigma = sigma;
	tonemappingBuffer.SubData (sizeof (glm::mat3x4) * 2 + sizeof (GLfloat) * 2,
														 sizeof (GLfloat),
														 &sigma);
}

float FinalPass::GetTonemappingSigma (void)
{
	return tonemapping.sigma;
}

void FinalPass::SetTonemappingExponent (float n)
{
	tonemapping.n = n;
	tonemappingBuffer.SubData (sizeof (glm::mat3x4) * 2 + sizeof (GLfloat) * 3,
														 sizeof (GLfloat),
														 &n);
}

float FinalPass::GetTonemappingExponent (void)
{
	return tonemapping.n;
}

void FinalPass::SetRGBWorkingSpace (GLuint ws)
{
	tonemapping.rgb_working_space = ws;
	if (tonemapping.rgb_working_space >= RGB2XYZ.size ())
		 tonemapping.rgb_working_space = 0;

	glm::mat3x4 mat (RGB2XYZ[tonemapping.rgb_working_space]);
	tonemappingBuffer.SubData (0, sizeof (glm::mat3x4), &mat[0][0]);
	mat = glm::mat3x4 (XYZ2RGB[tonemapping.rgb_working_space]);
	tonemappingBuffer.SubData (sizeof (glm::mat3x4), sizeof (glm::mat3x4),
														 &mat[0][0]);
}

GLuint FinalPass::GetRGBWorkingSpace (void)
{
	return tonemapping.rgb_working_space;
}

void FinalPass::SetTonemappingMode (GLuint mode)
{
	tonemapping.mode = mode;
}

GLuint FinalPass::GetTonemappingMode (void)
{
	return tonemapping.mode;
}

void FinalPass::SetAvgLumConst (float constant)
{
	tonemapping.avgLum.constant = constant;
	tonemappingBuffer.SubData (sizeof (glm::mat3x4) * 2 + sizeof (GLfloat) * 4,
														 sizeof (GLfloat), &constant);
}

float FinalPass::GetAvgLumConst (void)
{
	return tonemapping.avgLum.constant;
}

void FinalPass::SetAvgLumLinear (float linear)
{
	tonemapping.avgLum.linear = linear;
	tonemappingBuffer.SubData (sizeof (glm::mat3x4) * 2 + sizeof (GLfloat) * 5,
														 sizeof (GLfloat), &linear);
}

float FinalPass::GetAvgLumLinear (void)
{
	return tonemapping.avgLum.linear;
}

void FinalPass::SetAvgLumDelta (float delta)
{
	tonemapping.avgLum.delta = delta;
	fprograms.back ()["delta"] = tonemapping.avgLum.delta;
}

float FinalPass::GetAvgLumDelta (void)
{
	return tonemapping.avgLum.delta;
}

void FinalPass::SetAvgLumLod (float lod)
{
	tonemapping.avgLum.lod = lod;
	tonemappingBuffer.SubData (sizeof (glm::mat3x4) * 2 + sizeof (GLfloat) * 6,
														 sizeof (GLfloat), &lod);
}

float FinalPass::GetAvgLumLod (void)
{
	return tonemapping.avgLum.lod;
}

void FinalPass::Render (void)
{
	glm::uvec2 viewport;
	GLuint program;
	const char *tonemapNames[] = {
		"tonemapDefault",
		"tonemapReinhard",
		"tonemapLogarithmic",
		"tonemapURQ",
		"tonemapExponential",
		"tonemapDrago"
	};

	framebuffer.Bind (GL_FRAMEBUFFER);
	pipelines.back ().Bind ();
	r->composition.GetScreen ().Bind (GL_TEXTURE0, GL_TEXTURE_2D);
	r->windowgrid.sampler.Bind (0);
	r->windowgrid.Render ();
	gl::Framebuffer::Unbind (GL_FRAMEBUFFER);
	luminance.GenerateMipmap (GL_TEXTURE_2D);

	viewport = r->camera.GetViewport ();
	gl::Viewport (0, 0, viewport.x, viewport.y);

	switch (rendermode)
	{
	case 0:
		r->windowgrid.sampler.Bind (0);
		r->composition.GetScreen ().Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		if (r->GetAntialiasing ())
		{
			r->windowgrid.sampler.Bind (1);
			r->gbuffer.msdepthtexture.Bind (GL_TEXTURE1,
																						 GL_TEXTURE_2D_MULTISAMPLE);
			r->windowgrid.sampler.Bind (2);
			r->gbuffer.depthtexture.Bind (GL_TEXTURE2, GL_TEXTURE_2D);
		}
		sampler.Bind (3);
		luminance.Bind (GL_TEXTURE3, GL_TEXTURE_2D);
		program = 0;
		break;
	case 1:
		r->windowgrid.sampler.Bind (0);
		r->gbuffer.colorbuffer.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		program = 2;
		break;
	case 2:
		r->windowgrid.sampler.Bind (0);
		r->gbuffer.normalbuffer.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		program = 1;
		break;
	case 3:
		r->windowgrid.sampler.Bind (0);
		r->gbuffer.specularbuffer.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		program = 2;
		break;
	case 4:
		r->windowgrid.sampler.Bind (0);
		r->gbuffer.depthtexture.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		program = 5;
		break;
	case 5:
		r->windowgrid.sampler.Bind (0);
		r->composition.GetGlow ().GetMap ().Bind (GL_TEXTURE0,
																										 GL_TEXTURE_2D);
		program = 4;
		break;
	case 6:
		r->windowgrid.sampler.Bind (0);
//		r->shadowmap.GetMap ().Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		r->composition.dummy.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		program = 2;
		break;
	default:
		throw std::runtime_error ("Invalid render mode.");
		break;
	}

	pipelines[program].Bind ();

	if (program == 0)
	{
		GLuint idx;
		idx = fprograms[0].GetSubroutineIndex (GL_FRAGMENT_SHADER,
																					 tonemapNames[tonemapping.mode]);
		gl::UniformSubroutinesuiv (GL_FRAGMENT_SHADER, 1, &idx);
	}

	viewport_uniforms[program].Set (viewport);
	tonemappingBuffer.BindBase (GL_UNIFORM_BUFFER, 0);
	glow.Set (r->composition.GetGlow ().GetSize () > 0);
	antialiasing.Set (r->GetAntialiasing ());

	gl::Disable (GL_DEPTH_TEST);

	r->windowgrid.Render ();

	GL_CHECK_ERROR;
}
