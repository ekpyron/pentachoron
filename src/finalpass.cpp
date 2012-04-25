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

FinalPass::FinalPass (Renderer *parent)
	: renderer (parent), rendermode (0), 
		tonemapping ({ 0.18f, 1.0f, 1.0f, 1.0f, 0, 0 })
{
}

FinalPass::~FinalPass (void)
{
}

bool FinalPass::Init (void)
{
	std::vector<const char *> fprogram_sources = {
		"compose.txt", "normal.txt", "passthrough.txt", "diffuse.txt",
		"shadowmap.txt", "glow.txt", "depth.txt", "edge.txt"
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
	}

	for (gl::Program &fprogram : fprograms)
	{
		pipelines.emplace_back ();
		pipelines.back ().UseProgramStages (GL_VERTEX_SHADER_BIT,
																				renderer->windowgrid.vprogram);
		pipelines.back ().UseProgramStages (GL_FRAGMENT_SHADER_BIT, fprogram);
	}

	sampler.Parameter (GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	sampler.Parameter (GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	sampler.Parameter (GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	sampler.Parameter (GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return true;
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
}

float FinalPass::GetImageKey (void)
{
	return tonemapping.image_key;
}

void FinalPass::SetWhiteThreshold (float threshold)
{
	tonemapping.white_threshold = threshold;
}

float FinalPass::GetWhiteThreshold (void)
{
	return tonemapping.white_threshold;
}

void FinalPass::SetTonemappingSigma (float sigma)
{
	tonemapping.sigma = sigma;
}

float FinalPass::GetTonemappingSigma (void)
{
	return tonemapping.sigma;
}

void FinalPass::SetTonemappingExponent (float n)
{
	tonemapping.n = n;
}

float FinalPass::GetTonemappingExponent (void)
{
	return tonemapping.n;
}

void FinalPass::SetRGBWorkingSpace (GLuint ws)
{
	tonemapping.rgb_working_space = ws;
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

void FinalPass::Render (void)
{
	glm::uvec2 viewport;
	viewport = renderer->camera.GetViewport ();
	for (gl::Program &program : fprograms)
	{
		program["viewport"] = viewport;
		program["nearClipPlane"] = renderer->camera.GetNearClipPlane ();
		program["farClipPlane"] = renderer->camera.GetFarClipPlane ();
		program["tonemapping.image_key"] = tonemapping.image_key;
		program["tonemapping.white_threshold"] = tonemapping.white_threshold;
		if (tonemapping.rgb_working_space >= RGB2XYZ.size ())
			 tonemapping.rgb_working_space = 0;
		program["tonemapping.RGB2XYZ"] = RGB2XYZ[tonemapping.rgb_working_space];
		program["tonemapping.XYZ2RGB"] = XYZ2RGB[tonemapping.rgb_working_space];

		program["tonemapping.mode"] = tonemapping.mode;
		program["tonemapping.sigma"] = powf (tonemapping.sigma, tonemapping.n);
		program["tonemapping.n"] = tonemapping.n;
		program["antialiasing"] = (GLint) renderer->composition.GetAntialiasing ();
		program["glow"] = renderer->composition.GetGlowSize () > 0;
	}
	gl::Viewport (0, 0, viewport.x, viewport.y);

	gl::ClearColor (0, 0, 0, 0);
	gl::Clear (GL_COLOR_BUFFER_BIT);

	switch (rendermode)
	{
	case 0:
		renderer->windowgrid.sampler.Bind (0);
		renderer->composition.screen.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		sampler.Bind (1);
		renderer->composition.glow.Bind (GL_TEXTURE1, GL_TEXTURE_2D);
		renderer->windowgrid.sampler.Bind (2);
		renderer->composition.edgemap.Bind (GL_TEXTURE2,
																				GL_TEXTURE_2D);
		renderer->windowgrid.sampler.Bind (3);
		renderer->gbuffer.depthtexture[0].Bind (GL_TEXTURE3, GL_TEXTURE_2D);
		pipelines[0].Bind ();
		break;
	case 1:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.colorbuffer[0].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 2:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.colorbuffer[1].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 3:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.colorbuffer[2].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 4:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.colorbuffer[3].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 5:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.normalbuffer[0].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[1].Bind ();
		break;
	case 6:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.normalbuffer[1].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[1].Bind ();
		break;
	case 7:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.normalbuffer[2].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[1].Bind ();
		break;
	case 8:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.normalbuffer[3].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[1].Bind ();
		break;
	case 9:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.specularbuffer[0].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 10:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.specularbuffer[1].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 11:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.specularbuffer[2].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 12:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.specularbuffer[3].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[2].Bind ();
		break;
	case 13:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.depthtexture[0].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[6].Bind ();
		break;
  case 14:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.depthtexture[1].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[6].Bind ();
		break;
  case 15:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.depthtexture[2].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[6].Bind ();
		break;
  case 16:
		renderer->windowgrid.sampler.Bind (0);
		renderer->gbuffer.depthtexture[3].Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[6].Bind ();
		break;
	case 17:
		renderer->windowgrid.sampler.Bind (0);
		renderer->shadowmap.shadowmap.Bind (GL_TEXTURE0,
																				GL_TEXTURE_2D);
		pipelines[4].Bind ();
		break;
	case 18:
		sampler.Bind (0);
		renderer->composition.glow.Bind (GL_TEXTURE0, GL_TEXTURE_2D);
		pipelines[5].Bind ();
		break;
	case 19:
		renderer->windowgrid.sampler.Bind (0);
		renderer->composition.edgemap.Bind (GL_TEXTURE0,
																				GL_TEXTURE_2D);
		pipelines[7].Bind ();
		break;
	default:
		throw std::runtime_error ("Invalid render mode.");
		break;
	}

	gl::Disable (GL_DEPTH_TEST);

	renderer->windowgrid.Render ();

	GL_CHECK_ERROR;
}
