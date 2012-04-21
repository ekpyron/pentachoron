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
#include "interface.h"
#include "renderer.h"

typedef struct MenuEntry {
	 std::string name;
	 void (Interface::*info) (void);
	 void (Interface::*handler) (int what);
	 bool repeating;
} MenuEntry;

typedef struct Menu {
	 std::string title;
	 void (Interface::*info) (void);
	 std::vector<MenuEntry> entries;
} Menu;

#define MAIN_MENU                0
#define EDIT_LIGHTS              1
#define EDIT_SHADOWS             2
#define EDIT_LIGHT_POSITION      3
#define EDIT_LIGHT_DIRECTION     4
#define EDIT_LIGHT_DIFFUSE       5
#define EDIT_LIGHT_SPECULAR      6
#define EDIT_LIGHT_ATTENUATION   7
#define EDIT_SHADOW_POSITION     8
#define EDIT_TONE_MAPPING        9
#define EDIT_GLOW                10

const std::vector<Menu> menus = {
	{
		"Main Menu", NULL,
		{
			{ "Lights", NULL, &Interface::EditLights, false },
			{ "Shadows", NULL, &Interface::EditShadows, false },
			{ "Tone Mapping", NULL, &Interface::EditToneMapping, false },
			{ "Glow", NULL, &Interface::EditGlow, false },
			{ "Antialiasing: ", &Interface::PrintAntialiasing,
				&Interface::EditAntialiasing, false },
			{ "Rendermode: ", &Interface::PrintRendermode,
				&Interface::ToggleRendermode, false },
			{ "Exit", NULL, &Interface::Exit, false }
		}
	},
	{
		"Edit Lights ", &Interface::PrintActiveLight,
		{
			{ "Add Light", NULL, &Interface::AddLight, false },
			{ "Select Light ", NULL , &Interface::SelectLight, false },
			{ "Edit Position", NULL, &Interface::EditLightPosition, false },
			{ "Edit Direction", NULL, &Interface::EditLightDirection, false },
			{ "Edit Diffuse Color", NULL, &Interface::EditDiffuseColor, false },
			{ "Edit Specular Color", NULL, &Interface::EditSpecularColor, false },
			{ "Edit Attenuation", NULL, &Interface::EditAttenuation, false },
			{ "Spot Angle ", &Interface::PrintSpotAngle,
				&Interface::EditSpotAngle, true },
			{ "Spot Penumbra Angle ", &Interface::PrintSpotPenumbraAngle,
				&Interface::EditSpotPenumbraAngle, true },
			{ "Spot Exponent ", &Interface::PrintSpotExponent,
				&Interface::EditSpotExponent, true },
			{ "Remove", NULL, &Interface::RemoveLight, false },
			{ "Randomize lights", NULL, &Interface::RandomizeLights, false },
			{ "Back", NULL, &Interface::MainMenu, false }
		}
	},
	{
		"Edit Shadows ", &Interface::PrintActiveShadow,
		{
			{ "Add Shadow", NULL, &Interface::AddShadow, false },
			{ "Select Shadow ", NULL , &Interface::SelectShadow, false },
			{ "Edit Position", NULL, &Interface::EditShadowPosition, false },
			{ "Remove", NULL, &Interface::RemoveShadow, false },
			{ "Shadow Alpha: ", &Interface::PrintShadowAlpha,
				&Interface::EditShadowAlpha, true },
			{ "Soft Shadows: ", &Interface::PrintSoftShadow,
				&Interface::ToggleSoftShadow, false },
			{ "Back", NULL, &Interface::MainMenu, false }
		}
	},
	{
		"Light Position ", &Interface::PrintActiveLight,
		{
			{ "X ", &Interface::PrintLightX, &Interface::MoveLightX, true },
			{ "Y ", &Interface::PrintLightY, &Interface::MoveLightY, true },
			{ "Z ", &Interface::PrintLightZ, &Interface::MoveLightZ, true },
			{ "Back", NULL, &Interface::EditLights, false }
		}
	},
	{
		"Light Direction ", &Interface::PrintActiveLight,
		{
			{ "X ", &Interface::PrintLightDirX, &Interface::MoveLightDirX, true },
			{ "Y ", &Interface::PrintLightDirY, &Interface::MoveLightDirY, true },
			{ "Z ", &Interface::PrintLightDirZ, &Interface::MoveLightDirZ, true },
			{ "Back", NULL, &Interface::EditLights, false }
		}
	},
	{
		"Light Diffuse Color ", &Interface::PrintActiveLight,
		{
			{ "R ", &Interface::PrintDiffuseR, &Interface::EditDiffuseR, true },
			{ "G ", &Interface::PrintDiffuseG, &Interface::EditDiffuseG, true },
			{ "B ", &Interface::PrintDiffuseB, &Interface::EditDiffuseB, true },
			{ "Back", NULL, &Interface::EditLights, false }
		}
	},
	{
		"Light Specular Color ", &Interface::PrintActiveLight,
		{
			{ "R ", &Interface::PrintSpecularR, &Interface::EditSpecularR, true },
			{ "G ", &Interface::PrintSpecularG, &Interface::EditSpecularG, true },
			{ "B ", &Interface::PrintSpecularB, &Interface::EditSpecularB, true },
			{ "Back", NULL, &Interface::EditLights, false }
		}
	},
	{
		"Light Attenuation ", &Interface::PrintActiveLight,
		{
			{ "Constant ", &Interface::PrintConstantAttenuation,
				&Interface::EditConstantAttenuation, true },
			{ "Linear ", &Interface::PrintLinearAttenuation,
				&Interface::EditLinearAttenuation, true },
			{ "Quadratic ", &Interface::PrintQuadraticAttenuation,
				&Interface::EditQuadraticAttenuation, true },
			{ "Max Distance ", &Interface::PrintMaxDistance,
				&Interface::EditMaxDistance, true },
			{ "Back", NULL, &Interface::EditLights, false }
		}
	},
	{
		"Shadow Position ", &Interface::PrintActiveShadow,
		{
			{ "X ", &Interface::PrintShadowX, &Interface::MoveShadowX, true },
			{ "Y ", &Interface::PrintShadowY, &Interface::MoveShadowY, true },
			{ "Z ", &Interface::PrintShadowZ, &Interface::MoveShadowZ, true },
			{ "Back", NULL, &Interface::EditShadows, false }
		}
	},
	{
		"Tone Mapping", NULL,
		{
			{ "Mode: ", &Interface::PrintToneMappingMode,
				&Interface::EditToneMappingMode, false },
			{ "Image Key ", &Interface::PrintImageKey,
				&Interface::EditImageKey, true },
			{ "White Threshold ", &Interface::PrintWhiteThreshold,
				&Interface::EditWhiteThreshold, true },
			{ "Sigma: ", &Interface::PrintToneMappingSigma,
				&Interface::EditToneMappingSigma, true },
			{ "n: ", &Interface::PrintToneMappingN,
				&Interface::EditToneMappingN, true },
			{ "RGB Working Space: ", &Interface::PrintRGBWorkingSpace,
				&Interface::EditRGBWorkingSpace, false },
			{ "Back", NULL, &Interface::MainMenu, false }
		}
	},
	{
		"Glow", NULL,
		{
			{ "Blur size: ", &Interface::PrintGlowSize,
				&Interface::EditGlowSize, false },
			{ "Luminance Threshold: ", &Interface::PrintLuminanceThreshold,
				&Interface::EditLuminanceThreshold, true },
			{ "Back", NULL, &Interface::MainMenu, false }
		}
	}
};

Interface::Interface (Renderer *parent)
	: showInterface (false), menu (MAIN_MENU), submenu (0), renderer (parent),
		active_light (0), active_shadow (0), timefactor (0)
{
}

Interface::~Interface (void)
{
}

void Interface::MainMenu (int what)
{
	if (!what)
	{
		menu = MAIN_MENU;
		submenu = 0;
	}
}

void Interface::AddLight (int what)
{
	if (!what)
	{
		Light light;
		light.position = glm::vec4 (0, 10, 0, 0);
		light.color = glm::vec4 (1, 1, 1, 1);
		light.direction = glm::vec4 (0, -1, 0, 0);
		light.spot.exponent = 2.0f;
		light.spot.angle = DRE_PI/4.0f;
		light.spot.cosine = cosf (light.spot.angle);
		light.spot.tangent = tanf (light.spot.angle);
		light.spot.penumbra_angle = light.spot.angle * 0.8f;
		light.spot.penumbra_cosine = cosf (light.spot.penumbra_angle);
		light.specular.color = glm::vec3 (1, 1, 1);
		light.attenuation = glm::vec4 (0.0f, 0.0f, 0.007f, 50.0f);
		renderer->lights.push_back (light);

		renderer->lightmem = renderer->clctx.CreateBuffer
			 (CL_MEM_READ_ONLY,	sizeof (Light) * renderer->lights.size (), NULL);
		renderer->queue.EnqueueWriteBuffer
			 (renderer->lightmem, CL_TRUE, 0,
				sizeof (Light) * renderer->lights.size (),
				&renderer->lights[0], 0, NULL, NULL);
	}
}

void Interface::AddShadow (int what)
{
	if (!what)
	{
		Shadow shadow;
		shadow.position = glm::vec4 (0, 10, 0, 0);
		shadow.direction = glm::vec4 (0, -1, 0, 0);
		shadow.spot.angle = DRE_PI/4.0f;
		shadow.spot.cosine = cosf (shadow.spot.angle);
		renderer->shadows.push_back (shadow);
	}
}

void Interface::RemoveShadow (int what)
{
	if (!what)
	{
		if (renderer->shadows.size () < 2)
			 return;

		renderer->shadows.erase (renderer->shadows.begin () + active_shadow);
		if (active_shadow >= renderer->shadows.size ())
			 active_light = 0;
		if (!renderer->shadows.size ())
			 MainMenu (0);
	}
}

void Interface::RemoveLight (int what)
{
	if (!what)
	{
		if (renderer->lights.size () < 2)
			 return;

		renderer->lights.erase (renderer->lights.begin () + active_light);
		if (active_light >= renderer->lights.size ())
			 active_light = 0;
		renderer->queue.EnqueueWriteBuffer
			 (renderer->lightmem, CL_TRUE, 0,
				sizeof (Light) * renderer->lights.size (),
				&renderer->lights[0], 0, NULL, NULL);
	}
}

void Interface::EditGlowSize (int what)
{
	GLint size = renderer->composition.glowblur.GetSize ();

	size += what * 4;
	if (size >= 0)
	{
		renderer->composition.glowblur = renderer->filters.CreateBlur
			 (renderer->composition.glowmem_downsampled,
				renderer->gbuffer.width >> 2, renderer->gbuffer.height >> 2, size);
	}
}

void Interface::EditAntialiasing (int what)
{
	GLint size = renderer->antialiasing;

	size += what * 4;
	if (size < 0) return;
	renderer->antialiasing = size;
	if (size > 0)
	{
		renderer->composition.softmap = gl::Texture ();
		renderer->composition.softmap.Image2D (GL_TEXTURE_2D, 0, GL_RGBA16F,
																					 renderer->gbuffer.width,
																					 renderer->gbuffer.height,
																					 0, GL_RGBA, GL_FLOAT, NULL);
		renderer->composition.softmem = renderer->clctx.CreateFromGLTexture2D
				(CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, renderer->composition.softmap);

		renderer->composition.softmapblur = renderer->filters.CreateBlur
			 (renderer->composition.screenmem, renderer->composition.softmem,
				renderer->gbuffer.width, renderer->gbuffer.height, size);
		renderer->composition.freichen = renderer->filters.CreateFreiChen 
			 (renderer->composition.softmem, renderer->composition.edgemem,
				renderer->gbuffer.width, renderer->gbuffer.height);
	}
	else
	{
		renderer->composition.softmapblur = Blur ();
		renderer->composition.freichen = FreiChen ();
		renderer->composition.softmem = cl::Memory ();
		renderer->composition.softmap = gl::Texture ();
	}
}

void Interface::PrintAntialiasing (void)
{
	font.Print (renderer->antialiasing);
}

void Interface::PrintGlowSize (void)
{
	font.Print (renderer->composition.glowblur.GetSize ());
}

void Interface::RandomizeLights (int what)
{
	if (!what)
	{
		srand (time (NULL));
		for (Light &light : renderer->lights)
		{
			if (light.color == glm::vec4 (1, 1, 1, 1))
				 continue;
			light.position.x += ((float (rand ()) / float (RAND_MAX)) - 0.5f) * 2.0f;
			light.position.z += ((float (rand ()) / float (RAND_MAX)) - 0.5f) * 2.0f;
			if (light.position.x > 23.0f) light.position.x = 23.0f;
			if (light.position.x < -23.0f) light.position.x = -23.0f;
			if (light.position.z > 23.0f) light.position.z = 23.0f;
			if (light.position.z < -23.0f) light.position.z = -23.0f;

			const glm::vec4 colors[] = {
				glm::vec4 (0, 0, 1, 1),
				glm::vec4 (0, 1, 0, 1),
				glm::vec4 (1, 0, 0, 1),
				glm::vec4 (0, 1, 1, 1),
				glm::vec4 (1, 1, 0, 1),
				glm::vec4 (1, 0, 1, 1),
			};			
			light.color = colors[rand () % 6];
			light.specular.color = glm::vec3 (light.color);
		}
		renderer->queue.EnqueueWriteBuffer
			 (renderer->lightmem, CL_TRUE, 0,
				sizeof (Light) * renderer->lights.size (),
				&renderer->lights[0], 0, NULL, NULL);
	}
}

void Interface::EditLights (int what)
{
	if (!what)
	{
		if (!renderer->lights.size ())
		{
			AddLight (0);
		}
		menu = EDIT_LIGHTS;
		submenu = 0;
	}
}

void Interface::EditShadows (int what)
{
	if (!what)
	{
		if (!renderer->shadows.size ())
		{
			AddShadow (0);
		}
		menu = EDIT_SHADOWS;
		submenu = 0;
	}
}

void Interface::EditGlow (int what)
{
	if (!what)
	{
		menu = EDIT_GLOW;
		submenu = 0;
	}
}

#define NUM_RENDERMODES 20

void Interface::ToggleRendermode (int what)
{
	GLint rendermode = renderer->finalpass.GetRenderMode ();
	rendermode += what;
	if (rendermode < 0)
		 rendermode += NUM_RENDERMODES;
	if (rendermode >= NUM_RENDERMODES)
		 rendermode = 0;
	renderer->finalpass.SetRenderMode (rendermode);
}

void Interface::EditToneMapping (int what)
{
	if (!what)
	{
		menu = EDIT_TONE_MAPPING;
		submenu = 0;
	}
}

#define NUM_RGB_WORKING_SPACES 16

void Interface::EditRGBWorkingSpace (int what)
{
	GLint rgb_working_space;
	rgb_working_space = renderer->finalpass.tonemapping.rgb_working_space;
	rgb_working_space += what;
	if (rgb_working_space < 0)
		 rgb_working_space += NUM_RGB_WORKING_SPACES;
	if (rgb_working_space >= NUM_RGB_WORKING_SPACES)
		 rgb_working_space = 0;
	renderer->finalpass.tonemapping.rgb_working_space = rgb_working_space;
}

void Interface::PrintRGBWorkingSpace (void)
{
	const char *rgb_working_spaces[NUM_RGB_WORKING_SPACES] = {
		"Adobe RGB (1998)",
		"AppleRGB",
		"Best RGB",
		"Beta RGB",
		"Bruce RGB",
		"CIE RGB",
		"ColorMatch RGB",
		"Don RGB 4",
		"ECI RGB",
		"Ekta Space PS5",
		"NTSC RGB",
		"PAL/SECAM RGB",
		"ProPhoto RGB",
		"SMPTE-C RGB",
		"sRGB",
		"Wide Gamut RGB"
	};

	font.Print (rgb_working_spaces[renderer->finalpass.
																 tonemapping.rgb_working_space]);
}

extern bool running;

void Interface::Exit (int what)
{
	if (!what)
	{
		running = false;
	}
}

#define NUM_TONE_MAPPING_MODES 4

void Interface::PrintToneMappingMode (void)
{
	const char *tone_mapping_modes[NUM_TONE_MAPPING_MODES] = {
		"Reinhard",
		"Logarithmic",
		"URQ",
		"Exponential"
	};
	font.Print (tone_mapping_modes [renderer->finalpass.tonemapping.mode]);
}

void Interface::EditToneMappingMode (int what)
{
	GLint mode;
	mode = renderer->finalpass.tonemapping.mode;
	mode += what;
	if (mode < 0)
		 mode += NUM_TONE_MAPPING_MODES;
	if (mode >= NUM_TONE_MAPPING_MODES)
		 mode = 0;
	renderer->finalpass.tonemapping.mode = mode;
}

void Interface::PrintToneMappingN (void)
{
	font.Print (renderer->finalpass.tonemapping.n);
}

void Interface::EditToneMappingN (int what)
{
	renderer->finalpass.tonemapping.n += what * timefactor;
}

void Interface::PrintRendermode (void)
{
	const char *rendermodes[NUM_RENDERMODES] = {
		"compose",
		"color [0]",
		"color [1]",
		"color [2]",
		"color [3]",
		"normal buffer [0]",
		"normal buffer [1]",
		"normal buffer [2]",
		"normal buffer [3]",
		"specular buffer [0]",
		"specular buffer [1]",
		"specular buffer [2]",
		"specular buffer [3]",
		"depth buffer [0]",
		"depth buffer [1]",
		"depth buffer [2]",
		"depth buffer [3]",
		"shadow projection",
		"glow",
		"edges"
	};

	font.Print (rendermodes[renderer->finalpass.GetRenderMode ()]);
}

void Interface::SelectShadow (int what)
{
	active_shadow += what;
	if (active_shadow < 0)
		active_shadow += renderer->shadows.size ();
	if (active_shadow >= renderer->shadows.size ())
		 active_shadow = 0;
}

void Interface::SelectLight (int what)
{
	active_light += what;
	if (active_light < 0)
		active_light += renderer->lights.size ();
	if (active_light >= renderer->lights.size ())
		 active_light = 0;
}

void Interface::EditShadowPosition (int what)
{
	if (!what)
	{
		menu = EDIT_SHADOW_POSITION;
		submenu = 0;
	}
}

void Interface::EditLightPosition (int what)
{
	if (!what)
	{
		menu = EDIT_LIGHT_POSITION;
		submenu = 0;
	}
}

void Interface::EditLightDirection (int what)
{
	if (!what)
	{
		menu = EDIT_LIGHT_DIRECTION;
		submenu = 0;
	}
}

void Interface::EditDiffuseColor (int what)
{
	if (!what)
	{
		menu = EDIT_LIGHT_DIFFUSE;
		submenu = 0;
	}
}

void Interface::EditSpecularColor (int what)
{
	if (!what)
	{
		menu = EDIT_LIGHT_SPECULAR;
		submenu = 0;
	}
}

void Interface::EditAttenuation (int what)
{
	if (!what)
	{
		menu = EDIT_LIGHT_ATTENUATION;
		submenu = 0;
	}
}

void Interface::MoveShadowX (int what)
{
	renderer->shadows[active_shadow].position.x += what * timefactor;
}

void Interface::MoveShadowY (int what)
{
	renderer->shadows[active_shadow].position.y += what * timefactor;
}

void Interface::MoveShadowZ (int what)
{
	renderer->shadows[active_shadow].position.z += what * timefactor;
}

void Interface::ToggleSoftShadow (int what)
{
	bool value;
	value = renderer->shadowmap.GetSoftShadows ();
	renderer->shadowmap.SetSoftShadows (!value);
}

void Interface::MoveLightX (int what)
{
	renderer->lights[active_light].position.x += what * timefactor;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].position.x)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].position.x),
			&renderer->lights[active_light].position.x, 0, NULL, NULL);
}

void Interface::MoveLightY (int what)
{
	renderer->lights[active_light].position.y += what * timefactor;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].position.y)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].position.y),
			&renderer->lights[active_light].position.y,
			0, NULL, NULL);
}

void Interface::MoveLightZ (int what)
{
	renderer->lights[active_light].position.z += what * timefactor;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].position.z)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].position.z),
			&renderer->lights[active_light].position.z,
			0, NULL, NULL);

}

void Interface::MoveLightDirX (int what)
{
	renderer->lights[active_light].direction.x += what * timefactor;
	renderer->lights[active_light].direction
		 = glm::normalize (renderer->lights[active_light].direction);
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].direction)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].direction),
			&renderer->lights[active_light].direction,
			0, NULL, NULL);
}

void Interface::MoveLightDirY (int what)
{
	renderer->lights[active_light].direction.y += what * timefactor;
	renderer->lights[active_light].direction
		 = glm::normalize (renderer->lights[active_light].direction);
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].direction)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].direction),
			&renderer->lights[active_light].direction,
			0, NULL, NULL);
}

void Interface::MoveLightDirZ (int what)
{
	renderer->lights[active_light].direction.z += what * timefactor;
	renderer->lights[active_light].direction
		 = glm::normalize (renderer->lights[active_light].direction);
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].direction)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].direction),
			&renderer->lights[active_light].direction,
			0, NULL, NULL);
}

void Interface::EditDiffuseR (int what)
{
	renderer->lights[active_light].color.r += what * timefactor;
	if (renderer->lights[active_light].color.r < 0)
		 renderer->lights[active_light].color.r = 0;
	else if (renderer->lights[active_light].color.r > 1)
		 renderer->lights[active_light].color.r = 1;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].color.r)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].color.r),
			&renderer->lights[active_light].color.r,
			0, NULL, NULL);
}

void Interface::EditDiffuseG (int what)
{
	renderer->lights[active_light].color.g += what * timefactor;
	if (renderer->lights[active_light].color.g < 0)
		 renderer->lights[active_light].color.g = 0;
	else if (renderer->lights[active_light].color.g > 1)
		 renderer->lights[active_light].color.g = 1;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].color.g)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].color.g),
			&renderer->lights[active_light].color.g,
			0, NULL, NULL);
}

void Interface::EditDiffuseB (int what)
{
	renderer->lights[active_light].color.b += what * timefactor;
	if (renderer->lights[active_light].color.b < 0)
		 renderer->lights[active_light].color.b = 0;
	else if (renderer->lights[active_light].color.b > 1)
		 renderer->lights[active_light].color.b = 1;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].color.b)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].color.b),
			&renderer->lights[active_light].color.b,
			0, NULL, NULL);
}

void Interface::EditSpecularR (int what)
{
	renderer->lights[active_light].specular.color.r += what * timefactor;
	if (renderer->lights[active_light].specular.color.r < 0)
		 renderer->lights[active_light].specular.color.r = 0;
	else if (renderer->lights[active_light].specular.color.r > 1)
		 renderer->lights[active_light].specular.color.r = 1;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].specular.color.r)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].specular.color.r),
			&renderer->lights[active_light].specular.color.r,
			0, NULL, NULL);
}

void Interface::EditSpecularG (int what)
{
	renderer->lights[active_light].specular.color.g += what * timefactor;
	if (renderer->lights[active_light].specular.color.g < 0)
		 renderer->lights[active_light].specular.color.g = 0;
	else if (renderer->lights[active_light].specular.color.g > 1)
		 renderer->lights[active_light].specular.color.g = 1;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].specular.color.g)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].specular.color.g),
			&renderer->lights[active_light].specular.color.g,
			0, NULL, NULL);
}

void Interface::EditSpecularB (int what)
{
	renderer->lights[active_light].specular.color.b += what * timefactor;
	if (renderer->lights[active_light].specular.color.b < 0)
		 renderer->lights[active_light].specular.color.b = 0;
	else if (renderer->lights[active_light].specular.color.b > 1)
		 renderer->lights[active_light].specular.color.b = 1;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].specular.color.b)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].specular.color.b),
			&renderer->lights[active_light].specular.color.b,
			0, NULL, NULL);
}

void Interface::EditSpotAngle (int what)
{
	renderer->lights[active_light].spot.angle += what * timefactor;
	renderer->lights[active_light].spot.cosine
		 = cosf (renderer->lights[active_light].spot.angle);
	renderer->lights[active_light].spot.tangent
		 = tanf (renderer->lights[active_light].spot.angle);
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].spot)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].spot),
			&renderer->lights[active_light].spot,
			0, NULL, NULL);
}

void Interface::EditSpotPenumbraAngle (int what)
{
	renderer->lights[active_light].spot.penumbra_angle += what * timefactor;
	renderer->lights[active_light].spot.penumbra_cosine
		 = cosf (renderer->lights[active_light].spot.penumbra_angle);
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].spot)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].spot),
			&renderer->lights[active_light].spot,
			0, NULL, NULL);
}

void Interface::EditSpotExponent (int what)
{
	renderer->lights[active_light].spot.exponent += what * timefactor;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].spot.exponent)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].spot.exponent),
			&renderer->lights[active_light].spot.exponent,
			0, NULL, NULL);
}

void Interface::EditConstantAttenuation (int what)
{
	renderer->lights[active_light].attenuation.x += what * timefactor * 0.1;
	if (renderer->lights[active_light].attenuation.x < 0)
		 renderer->lights[active_light].attenuation.x = 0;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].attenuation.x)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].attenuation.x),
			&renderer->lights[active_light].attenuation.x,
			0, NULL, NULL);
}

void Interface::EditLinearAttenuation (int what)
{
	renderer->lights[active_light].attenuation.y += what * timefactor * 0.1;
	if (renderer->lights[active_light].attenuation.y < 0)
		 renderer->lights[active_light].attenuation.y = 0;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].attenuation.y)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].attenuation.y),
			&renderer->lights[active_light].attenuation.y,
			0, NULL, NULL);
}

void Interface::EditQuadraticAttenuation (int what)
{
	renderer->lights[active_light].attenuation.z += what * timefactor * 0.001;
	if (renderer->lights[active_light].attenuation.z < 0)
		 renderer->lights[active_light].attenuation.z = 0;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].attenuation.z)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].attenuation.z),
			&renderer->lights[active_light].attenuation.z,
			0, NULL, NULL);
}

void Interface::EditMaxDistance (int what)
{
	renderer->lights[active_light].attenuation.w += what * timefactor * 0.1;
	if (renderer->lights[active_light].attenuation.w < 0)
		 renderer->lights[active_light].attenuation.w = 0;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].attenuation.w)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].attenuation.w),
			&renderer->lights[active_light].attenuation.w,
			0, NULL, NULL);
}

void Interface::EditImageKey (int what)
{
	renderer->finalpass.tonemapping.image_key += what * timefactor;
}

void Interface::EditWhiteThreshold (int what)
{
	renderer->finalpass.tonemapping.white_threshold += what * timefactor;
}

void Interface::EditToneMappingSigma (int what)
{
	renderer->finalpass.tonemapping.sigma += what * timefactor;
}

void Interface::EditLuminanceThreshold (int what)
{
	renderer->composition.luminance_threshold += what * timefactor;
}

void Interface::EditShadowAlpha (int what)
{
	renderer->composition.shadow_alpha += what * timefactor;
	if (renderer->composition.shadow_alpha < 0)
		 renderer->composition.shadow_alpha = 0;
	if (renderer->composition.shadow_alpha > 1)
		 renderer->composition.shadow_alpha = 1;
}

void Interface::PrintToneMappingSigma (void)
{
	font.Print (renderer->finalpass.tonemapping.sigma);
}

void Interface::PrintActiveLight (void)
{
	font.Print (active_light + 1, " / ", renderer->lights.size ());
}

void Interface::PrintActiveShadow (void)
{
	font.Print (active_shadow + 1, " / ", renderer->shadows.size ());
}

void Interface::PrintShadowX (void)
{
	font.Print (renderer->shadows[active_shadow].position.x);
}

void Interface::PrintShadowY (void)
{
	font.Print (renderer->shadows[active_shadow].position.y);
}

void Interface::PrintShadowZ (void)
{
	font.Print (renderer->shadows[active_shadow].position.z);
}

void Interface::PrintSoftShadow (void)
{
	if (renderer->shadowmap.GetSoftShadows ())
		 font.Print ("yes");
	else
		 font.Print ("no");
}

void Interface::PrintLightX (void)
{
	font.Print (renderer->lights[active_light].position.x);
}

void Interface::PrintLightY (void)
{
	font.Print (renderer->lights[active_light].position.y);
}

void Interface::PrintLightZ (void)
{
	font.Print (renderer->lights[active_light].position.z);
}

void Interface::PrintLightDirX (void)
{
	font.Print (renderer->lights[active_light].direction.x);
}

void Interface::PrintLightDirY (void)
{
	font.Print (renderer->lights[active_light].direction.y);
}

void Interface::PrintLightDirZ (void)
{
	font.Print (renderer->lights[active_light].direction.z);
}

void Interface::PrintDiffuseR (void)
{
	font.Print (renderer->lights[active_light].color.r);
}

void Interface::PrintDiffuseG (void)
{
	font.Print (renderer->lights[active_light].color.g);
}

void Interface::PrintDiffuseB (void)
{
	font.Print (renderer->lights[active_light].color.b);
}

void Interface::PrintSpecularR (void)
{
	font.Print (renderer->lights[active_light].specular.color.r);
}

void Interface::PrintSpecularG (void)
{
	font.Print (renderer->lights[active_light].specular.color.g);
}

void Interface::PrintSpecularB (void)
{
	font.Print (renderer->lights[active_light].specular.color.b);
}

void Interface::PrintSpotAngle (void)
{
	font.Print (renderer->lights[active_light].spot.angle * 180.0f / DRE_PI);
}

void Interface::PrintSpotPenumbraAngle (void)
{
	font.Print (renderer->lights[active_light].spot.penumbra_angle
							* 180.0f / DRE_PI);
}

void Interface::PrintSpotExponent (void)
{
	font.Print (renderer->lights[active_light].spot.exponent);
}

void Interface::PrintConstantAttenuation (void)
{
	font.Print (renderer->lights[active_light].attenuation.x);
}

void Interface::PrintLinearAttenuation (void)
{
	font.Print (renderer->lights[active_light].attenuation.y);
}

void Interface::PrintQuadraticAttenuation (void)
{
	font.Print (renderer->lights[active_light].attenuation.z * 100);
}

void Interface::PrintMaxDistance (void)
{
	font.Print (renderer->lights[active_light].attenuation.w);
}

void Interface::PrintImageKey (void)
{
	font.Print (renderer->finalpass.tonemapping.image_key);
}

void Interface::PrintWhiteThreshold (void)
{
	font.Print (renderer->finalpass.tonemapping.white_threshold);
}

void Interface::PrintLuminanceThreshold (void)
{
	font.Print (renderer->composition.luminance_threshold);
}

void Interface::PrintShadowAlpha (void)
{
	font.Print (renderer->composition.shadow_alpha);
}

bool Interface::Init (void)
{
	if (!freetype.Init ())
		 return false;

	if (!freetype.Load (font, MakePath ("fonts", config["font"]["font"]
																			.as<std::string> () )))
		 return false;

	return true;
}

void Interface::OnKeyUp (int key)
{
	if (key == GLFW_KEY_TAB)
	{
		showInterface = !showInterface;
		return;
	}
	if (showInterface)
	{
		switch (key)
		{
		case GLFW_KEY_UP:
			if (!submenu)
				 submenu = menus[menu].entries.size ();
			submenu--;
			break;
		case GLFW_KEY_DOWN:
			submenu++;
			if (submenu >= menus[menu].entries.size ())
				 submenu = 0;
			break;
		case GLFW_KEY_ENTER:
			(this->*(menus[menu].entries[submenu].handler)) (0);
			break;
		case GLFW_KEY_LEFT:
			if (!menus[menu].entries[submenu].repeating)
				 (this->*(menus[menu].entries[submenu].handler)) (-1);
			break;
		case GLFW_KEY_RIGHT:
			if (!menus[menu].entries[submenu].repeating)
				 (this->*(menus[menu].entries[submenu].handler)) (1);
			break;
		case GLFW_KEY_ESC:
			if (menu == MAIN_MENU)
				 showInterface = false;
			else
				 MainMenu (0);
			break;
		}
	}
}

void Interface::OnKeyDown (int key)
{
}

void Interface::Frame (float tf)
{
	static unsigned int fps = 0, frames = 0;
	static double last_time = 0;

	timefactor = tf;

	if (glfwGetKey (GLFW_KEY_LSHIFT))
		 timefactor *= 5.0f;

	if (glfwGetKey (GLFW_KEY_LCTRL))
		 timefactor *= 10.0f;

	if (glfwGetKey (GLFW_KEY_LALT))
		 timefactor *= 20.0f;

	if (glfwGetTime () - last_time >= 1.0)
	{
		fps = frames;
		last_time = glfwGetTime ();
		frames = 0;
	}
	frames++;

	if (glfwGetKey (GLFW_KEY_RIGHT))
	{
		if (menus[menu].entries[submenu].repeating)
			 (this->*(menus[menu].entries[submenu].handler)) (1);
	}

	if (glfwGetKey (GLFW_KEY_LEFT))
	{
		if (menus[menu].entries[submenu].repeating)
			 (this->*(menus[menu].entries[submenu].handler)) (-1);
	}

	font.Frame ();
	if (showInterface)
	{
		font.SetColor (glm::vec3 (1, 1, 1));
		font.Print (menus[menu].title);
		if (menus[menu].info)
		{
			(this->*(menus[menu].info))();
		}
		font.Print ("\n\n");

		for (auto i = 0; i < menus[menu].entries.size (); i++)
		{
			if (i == submenu)
				 font.SetColor (glm::vec3 (0, 1, 0));
			else
				 font.SetColor (glm::vec3 (1, 1, 1));
			font.Print (menus[menu].entries[i].name);
			if (menus[menu].entries[i].info)
			{
				(this->*(menus[menu].entries[i].info)) ();
			}
			font.Print ('\n');
		}
	}
	else
	{
		font.SetColor (glm::vec3 (1, 1, 1));
		font.Print ("FPS: ", fps);
		font.Print ("\nOcclusion culled: ", Model::culled);
		font.Print ("\nFrustum culled: ", renderer->culling.culled);
	}
	glm::vec3 dir (sin (renderer->camera.horizontal_angle), 0,
								 cos (renderer->camera.horizontal_angle));
	if (glfwGetKey ('A'))
	{
		renderer->camera.horizontal_angle += timefactor;
	}
	if (glfwGetKey ('D'))
	{
		renderer->camera.horizontal_angle -= timefactor;
	}
	if (glfwGetKey ('W'))
	{
		renderer->camera.center += dir * timefactor * 10.0f;
	}
	if (glfwGetKey ('S'))
	{
		renderer->camera.center -= dir * timefactor * 10.0f;
	}
	if (glfwGetKey ('Q'))
	{
		renderer->camera.center -= glm::vec3 (0, 4, 0) * timefactor;
	}
	if (glfwGetKey ('E'))
	{
		renderer->camera.center += glm::vec3 (0, 4, 0) * timefactor;
	}
}
