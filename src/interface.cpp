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

#define MAIN_MENU                 0
#define EDIT_LIGHTS               1
#define EDIT_SHADOWS              2
#define EDIT_LIGHT_POSITION       3
#define EDIT_LIGHT_DIRECTION      4
#define EDIT_LIGHT_DIFFUSE        5
#define EDIT_LIGHT_SPECULAR       6
#define EDIT_LIGHT_ATTENUATION    7
#define EDIT_SHADOW_POSITION      8
#define EDIT_TONE_MAPPING         9
#define EDIT_GLOW                 10
#define EDIT_TONE_MAPPING_AVG_LUM 11
#define EDIT_ANTIALIASING         12

extern bool running;

Interface::Interface (Renderer *parent)
	: showInterface (0), menu (MAIN_MENU), submenu (0), renderer (parent),
		active_light (0), active_shadow (0), timefactor (0)
{
	menus = {
		{
			"Main Menu", NULL,
			{
				{ "Lights", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_LIGHTS;
							submenu = 0;
						}
					}, false },
				{ "Shadows", NULL, [&] (int what) {
						if (!what)
						{
							if (!renderer->shadows.size ())
							{
								AddShadow ();
							}
							menu = EDIT_SHADOWS;
							submenu = 0;
						}
					}, false },
				{ "Tone Mapping", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_TONE_MAPPING;
							submenu = 0;
						}
					}, false },
				{ "Glow", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_GLOW;
							submenu = 0;
						}
					}, false },
				{ "Antialiasing", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_ANTIALIASING;
							submenu = 0;
						}
					}, false },
#define NUM_RENDERMODES 7
				{ "Rendermode: ", [&] (void) {
						const char *rendermodes[] = {
							"compose",
							"color",
							"normal buffer",
							"specular buffer",
							"depth buffer",
							"glow",
							"shadow mask"
						};
						font.Print (rendermodes[renderer->finalpass.GetRenderMode ()]);
					}, [&] (int what) {
						GLint rendermode = renderer->finalpass.GetRenderMode ();
						rendermode += what;
						if (rendermode < 0)
							 rendermode += NUM_RENDERMODES;
						if (rendermode >= NUM_RENDERMODES)
							 rendermode = 0;
						renderer->finalpass.SetRenderMode (rendermode);
					}, false },
				{ "Compositionmode: ", [&] (void) {
						font.Print (renderer->composition.GetMode ());
					}, [&] (int what) {
						GLint mode = renderer->composition.GetMode ();
						mode += what;
						if (mode < 0)
							 mode = 0;
						renderer->composition.SetMode (mode);
					}, false },
				{ "Exit", NULL, [&] (int what) {
						if (!what)
						{
							running = false;
						}
					}, false }
			}
		},
		{
			"Edit Lights ", [&] (void) {
				font.Print (active_light + 1, " / ", renderer->GetNumLights ());
			},
			{
				{ "Add Light", NULL, [&] (int what) {
						if (!what)
						{
							AddLight ();
						}
					}, false },
				{ "Select Light ", NULL , [&] (int what) {
						active_light += what;
						if (active_light < 0)
							 active_light += renderer->GetNumLights ();
						if (active_light >= renderer->GetNumLights ())
							 active_light = 0;
					}, false },
				{ "Edit Position", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_LIGHT_POSITION;
							submenu = 0;
						}
					}, false },
				{ "Edit Direction", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_LIGHT_DIRECTION;
							submenu = 0;
						}
					}, false },
				{ "Edit Diffuse Color", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_LIGHT_DIFFUSE;
							submenu = 0;
						}
					}, false },
				{ "Edit Specular Color", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_LIGHT_SPECULAR;
							submenu = 0;
						}
					}, false },
				{ "Edit Attenuation", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_LIGHT_ATTENUATION;
							submenu = 0;
						}
					}, false },
				{ "Spot Angle ", [&] (void) {
						font.Print (renderer->GetLight (active_light).spot.angle
												* 180.0f / DRE_PI);
					}, [&] (int what) {
						renderer->GetLight (active_light).spot.angle += what * timefactor;
						renderer->UpdateLight (active_light);
					}, true },
				{ "Spot Penumbra Angle ", [&] (void) {
						font.Print (renderer->GetLight (active_light).spot.penumbra_angle
												* 180.0f / DRE_PI);
					}, [&] (int what) {
						renderer->GetLight (active_light).spot.penumbra_angle
						+= what * timefactor;
						renderer->UpdateLight (active_light);
					}, true },
				{ "Spot Exponent ", [&] (void) {
						font.Print (renderer->GetLight (active_light).spot.exponent);
					}, [&] (int what) {
						renderer->GetLight (active_light).spot.exponent
						+= what * timefactor;
						renderer->UpdateLight (active_light);
					}, true },
				{ "Remove", NULL, [&] (int what) {
						if (!what)
						{
							renderer->RemoveLight (active_light);
						}
					}, false },
				{ "Randomize lights", NULL, [&] (int what) {
						if (!what)
						{
							srand (time (NULL));
							for (auto i = 0; i < renderer->GetNumLights (); i++)
							{
								Light &light = renderer->GetLight (i);
								if (light.color == glm::vec4 (1, 1, 1, 1))
									 continue;
								light.position.x += ((float (rand ())
																			/ float (RAND_MAX)) - 0.5f) * 2.0f;
								light.position.z += ((float (rand ())
																			/ float (RAND_MAX)) - 0.5f) * 2.0f;
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
								renderer->UpdateLight (i);
							}
						}
					}, false },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							menu = MAIN_MENU;
							submenu = 0;
						}
					}, false }
			}
		},
		{
			"Edit Shadows ", [&] (void) {
				font.Print (active_shadow + 1, " / ", renderer->shadows.size ());
			},
			{
				{ "Add Shadow", NULL, [&] (int what) {
						if (!what)
						{
							AddShadow ();
						}
					}, false },
				{ "Select Shadow ", NULL , [&] (int what) {
						active_shadow += what;
						if (active_shadow < 0)
							 active_shadow += renderer->shadows.size ();
						if (active_shadow >= renderer->shadows.size ())
							 active_shadow = 0;
					}, false },
				{ "Edit Position", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_SHADOW_POSITION;
							submenu = 0;
						}
					}, false },
				{ "Remove", NULL, [&] (int what) {
						if (!what)
						{
							if (renderer->shadows.size () < 2)
								 return;
							
							renderer->shadows.erase (renderer->shadows.begin ()
																			 + active_shadow);
							if (active_shadow >= renderer->shadows.size ())
								 active_light = 0;
							if (!renderer->shadows.size ())
							{
								menu = MAIN_MENU;
								submenu = 0;
							}
						}
					}, false },
				{ "Shadow Alpha: ", [&] (void) {
						font.Print (renderer->composition.GetShadowAlpha ());
				}, [&] (int what) {
						GLfloat alpha;
						alpha = renderer->composition.GetShadowAlpha ();
						alpha += timefactor * what;
						alpha = (alpha>=0?alpha:0);
						alpha = (alpha<=1?alpha:1);
						renderer->composition.SetShadowAlpha (alpha);
					}, true },
				{ "Soft Shadows: ", [&] (void) {
						if (renderer->shadowpass.GetSoftShadows ())
							 font.Print ("yes");
						else
							 font.Print ("no");
					}, [&] (int what) {
						bool value;
						value = renderer->shadowpass.GetSoftShadows ();
						renderer->shadowpass.SetSoftShadows (!value);
				}, false },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							menu = MAIN_MENU;
							submenu = 0;
						}
					}, false }
			}
		},
		{
			"Light Position ", [&] (void) {
				font.Print (active_light + 1, " / ", renderer->GetNumLights ());
			},
			{
				{ "X ", [&] (void) {
						font.Print (renderer->GetLight (active_light).position.x);
					}, [&] (int what) {
						renderer->GetLight (active_light).position.x += what * timefactor;
						renderer->UpdateLight (active_light);
					}, true },
				{ "Y ", [&] (void) {
						font.Print (renderer->GetLight (active_light).position.y);
					}, [&] (int what) {
						renderer->GetLight (active_light).position.y += what * timefactor;
						renderer->UpdateLight (active_light);
					}, true },
				{ "Z ", [&] (void) {
						font.Print (renderer->GetLight (active_light).position.z);
					}, [&] (int what) {
						renderer->GetLight (active_light).position.z += what * timefactor;
						renderer->UpdateLight (active_light);
					}, true },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_LIGHTS;
							submenu = 0;
						}
					}, false }
			}
		},
		{
			"Light Direction ", [&] (void) {
				font.Print (active_light + 1, " / ", renderer->GetNumLights ());
			},
			{
				{ "X ", [&] (void) {
						font.Print (renderer->GetLight (active_light).direction.x);
					}, [&] (int what) {
						renderer->GetLight (active_light).direction.x += what * timefactor;
						renderer->UpdateLight (active_light);
					}, true },
				{ "Y ", [&] (void) {
						font.Print (renderer->GetLight (active_light).direction.y);
					}, [&] (int what) {
						renderer->GetLight (active_light).direction.y += what * timefactor;
						renderer->UpdateLight (active_light);
					}, true },
				{ "Z ", [&] (void) {
						font.Print (renderer->GetLight (active_light).direction.z);
					}, [&] (int what) {
						renderer->GetLight (active_light).direction.z += what * timefactor;
						renderer->UpdateLight (active_light);
					}, true },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_LIGHTS;
							submenu = 0;
						}
					}, false }
			}
		},
		{
			"Light Diffuse Color ", [&] (void) {
				font.Print (active_light + 1, " / ", renderer->GetNumLights ());
			},
			{
				{ "R ", [&] (void) {
						font.Print (renderer->GetLight (active_light).color.r);
					}, [&] (int what) {
						renderer->GetLight (active_light).color.r += what * timefactor;
						renderer->UpdateLight (active_light);
					}, true },
				{ "G ", [&] (void) {
						font.Print (renderer->GetLight (active_light).color.g);
					}, [&] (int what) {
						renderer->GetLight (active_light).color.g += what * timefactor;
						renderer->UpdateLight (active_light);
					}, true },
				{ "B ", [&] (void) {
						font.Print (renderer->GetLight (active_light).color.b);
					}, [&] (int what) {
						renderer->GetLight (active_light).color.b += what * timefactor;
						renderer->UpdateLight (active_light);
					}, true },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_LIGHTS;
							submenu = 0;
						}
					}, false }
			}
		},
		{
			"Light Specular Color ", [&] (void) {
				font.Print (active_light + 1, " / ", renderer->GetNumLights ());
			},
			{
				{ "R ", [&] (void) {
						font.Print (renderer->GetLight (active_light).specular.color.r);
					}, [&] (int what) {
						renderer->GetLight (active_light).specular.color.r
						+= what * timefactor;
						renderer->UpdateLight (active_light);
					}, true },
				{ "G ", [&] (void) {
						font.Print (renderer->GetLight (active_light).specular.color.g);
					}, [&] (int what) {
						renderer->GetLight (active_light).specular.color.g
						+= what * timefactor;
						renderer->UpdateLight (active_light);
					}, true },
				{ "B ", [&] (void) {
						font.Print (renderer->GetLight (active_light).specular.color.b);
					}, [&] (int what) {
						renderer->GetLight (active_light).specular.color.b
						+= what * timefactor;
						renderer->UpdateLight (active_light);
					}, true },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_LIGHTS;
							submenu = 0;
						}
					}, false }
			}
		},
		{
			"Light Attenuation ", [&] (void) {
				font.Print (active_light + 1, " / ", renderer->GetNumLights ());
			},
			{
				{ "Constant ", [&] (void) {
						font.Print (renderer->GetLight (active_light).attenuation.x);
					}, [&] (int what) {
						renderer->GetLight (active_light).attenuation.x
						+= what * timefactor * 0.1;
						renderer->UpdateLight (active_light);
					}, true },
				{ "Linear ", [&] (void) {
						font.Print (renderer->GetLight (active_light).attenuation.y);
					}, [&] (int what) {
						renderer->GetLight (active_light).attenuation.y
						+= what * timefactor * 0.1;
						renderer->UpdateLight (active_light);
					}, true },
				{ "Quadratic ", [&] (void) {
						font.Print (renderer->GetLight (active_light).attenuation.z * 100);
					}, [&] (int what) {
						renderer->GetLight (active_light).attenuation.z
						+= what * timefactor * 0.001;
						renderer->UpdateLight (active_light);
					}, true },
				{ "Max Distance ", [&] (void) {
						font.Print (renderer->GetLight (active_light).attenuation.w);
					}, [&] (int what) {
						renderer->GetLight (active_light).attenuation.w
						+= what * timefactor * 0.1;
						renderer->UpdateLight (active_light);
					}, true },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_LIGHTS;
							submenu = 0;
						}
					}, false }
			}
		},
		{
			"Shadow Position ", [&] (void) {
				font.Print (active_shadow + 1, " / ", renderer->shadows.size ());
			},
			{
				{ "X ", [&] (void) {
						font.Print (renderer->shadows[active_shadow].position.x);
					}, [&] (int what) {
						renderer->shadows[active_shadow].position.x += what * timefactor;
					}, true },
				{ "Y ", [&] (void) {
						font.Print (renderer->shadows[active_shadow].position.y);
					}, [&] (int what) {
						renderer->shadows[active_shadow].position.y += what * timefactor;
					}, true },
				{ "Z ", [&] (void) {
						font.Print (renderer->shadows[active_shadow].position.z);
					}, [&] (int what) {
						renderer->shadows[active_shadow].position.z += what * timefactor;
					}, true },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							if (!renderer->shadows.size ())
							{
								AddShadow ();
							}
							menu = EDIT_SHADOWS;
							submenu = 0;
						}
					}, false }
			}
		},
#define NUM_TONE_MAPPING_MODES 6
		{
			"Tone Mapping", NULL,
			{
				{ "Mode: ", [&] (void) {
						const char *tone_mapping_modes[NUM_TONE_MAPPING_MODES] = {
							"Default",
							"Reinhard",
							"Logarithmic",
							"URQ",
							"Exponential",
							"Drago"
						};
						font.Print (tone_mapping_modes [renderer->finalpass.
																						GetTonemappingMode ()]);
					}, [&] (int what) {
						GLint mode;
						mode = renderer->finalpass.GetTonemappingMode ();
						mode += what;
						if (mode < 0)
							 mode += NUM_TONE_MAPPING_MODES;
						if (mode >= NUM_TONE_MAPPING_MODES)
							 mode = 0;
						renderer->finalpass.SetTonemappingMode (mode);
					}, false },
				{ "Image Key ", [&] (void) {
						font.Print (renderer->finalpass.GetImageKey ());
					}, [&] (int what) {
						float key;
						key = renderer->finalpass.GetImageKey ();
						key += what * timefactor;
						renderer->finalpass.SetImageKey (key);
					}, true },
				{ "Average Luminance", NULL,
					[&] (int what) {
						if (!what)
						{
							menu = EDIT_TONE_MAPPING_AVG_LUM;
							submenu = 0;
						}
					}, false },
				{ "White Threshold ", [&] (void) {
						font.Print (renderer->finalpass.GetWhiteThreshold ());
					}, [&] (int what) {
						float threshold;
						threshold = renderer->finalpass.GetWhiteThreshold ();
						threshold += what * timefactor;
						renderer->finalpass.SetWhiteThreshold (threshold);
					}, true },
				{ "Sigma: ", [&] (void) {
						font.Print (renderer->finalpass.GetTonemappingSigma ());
					}, [&] (int what) {
						float sigma;
						sigma = renderer->finalpass.GetTonemappingSigma ();
						sigma += what * timefactor;
						renderer->finalpass.SetTonemappingSigma (sigma);
					}, true },
				{ "n: ", [&] (void) {
						font.Print (renderer->finalpass.GetTonemappingExponent ());
					}, [&] (int what) {
						float n;
						n = renderer->finalpass.GetTonemappingExponent ();
						n += what * timefactor;
						renderer->finalpass.SetTonemappingExponent (n);
					}, true },
#define NUM_RGB_WORKING_SPACES 16
				{ "RGB Working Space: ", [&] (void) {
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
																					 GetRGBWorkingSpace ()]);
					}, [&] (int what) {
						GLint rgb_working_space;
						rgb_working_space = renderer->finalpass.GetRGBWorkingSpace ();
						rgb_working_space += what;
						if (rgb_working_space < 0)
							 rgb_working_space += NUM_RGB_WORKING_SPACES;
						if (rgb_working_space >= NUM_RGB_WORKING_SPACES)
							 rgb_working_space = 0;
						renderer->finalpass.SetRGBWorkingSpace (rgb_working_space);
					}, false },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							menu = MAIN_MENU;
							submenu = 0;
						}
					}, false }
			}
		},
		{
			"Glow", NULL,
			{
				{ "Blur size: ", [&] (void) {
						font.Print (renderer->composition.GetGlow ().GetSize ());
					}, [&] (int what) {
						GLint size = renderer->composition.GetGlow ().GetSize ();
						size += what * 4;
						if (size >= 0)
						{
							renderer->composition.GetGlow ().SetSize (size);
						}
					}, false },
				{ "Luminance Threshold: ", [&] (void) {
						font.Print (renderer->composition.GetLuminanceThreshold ());
					}, [&] (int what) {
						float threshold;
						threshold = renderer->composition.GetLuminanceThreshold ();
						threshold += what * timefactor;
						renderer->composition.SetLuminanceThreshold (threshold);
					}, true },
				{ "Limit: ", [&] (void) {
						font.Print (renderer->composition.GetGlow ().GetLimit ());
					}, [&] (int what) {
						GLfloat limit = renderer->composition.GetGlow ().GetLimit ();
						limit += what * timefactor;
						renderer->composition.GetGlow ().SetLimit (limit);
					}, true },
				{ "Exponent: ", [&] (void) {
						font.Print (renderer->composition.GetGlow ().GetExponent ());
					}, [&] (int what) {
						GLfloat exp = renderer->composition.GetGlow ().GetExponent ();
						exp += what * timefactor;
						renderer->composition.GetGlow ().SetExponent (exp);
					}, true },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							menu = MAIN_MENU;
							submenu = 0;
						}
					}, false }
			}
		},
		{
			"Tonemapping Average Luminance", NULL,
			{
				{ "Constant: ", [&] (void) {
						float c;
						c = renderer->finalpass.GetAvgLumConst ();
						font.Print (c);
					}, [&] (int what) {
						float c;
						c = renderer->finalpass.GetAvgLumConst ();
						c += what * timefactor;
						renderer->finalpass.SetAvgLumConst (c);
					}, true },
				{ "Linear: ", [&] (void) {
						float c;
						c = renderer->finalpass.GetAvgLumLinear ();
						font.Print (c);
					}, [&] (int what) {
						float c;
						c = renderer->finalpass.GetAvgLumLinear ();
						c += what * timefactor;
						renderer->finalpass.SetAvgLumLinear (c);
					}, true },
				{ "Delta: ", [&] (void) {
						float c;
						c = renderer->finalpass.GetAvgLumDelta ();
						font.Print (c);
					}, [&] (int what) {
						float c;
						c = renderer->finalpass.GetAvgLumDelta ();
						c += what * timefactor;
						renderer->finalpass.SetAvgLumDelta (c);
					}, true },
				{ "Lod: ", [&] (void) {
						float c;
						c = renderer->finalpass.GetAvgLumLod ();
						font.Print (c);
					}, [&] (int what) {
						float c;
						c = renderer->finalpass.GetAvgLumLod ();
						c += what * 0.1;
						renderer->finalpass.SetAvgLumLod (c);
					}, true },
				{ "Back", NULL, [&](int what) {
						if (!what)
						{
							menu = EDIT_TONE_MAPPING;
							submenu = 0;
						}
					}, false }
			}
		},
		{
			"Antialiasing", NULL,
			{
				{ "Samples: ", [&] (void) {
						font.Print (renderer->GetAntialiasing ());
					}, [&] (int what) {
						if (!what)
						{
							GLuint samples = renderer->GetAntialiasing ();
							samples += 4;
							if (samples > 16) samples = 0;
							renderer->SetAntialiasing (samples);
						}
					}, true },
				{ "Threshold: ", [&] (void) {
						font.Print (renderer->finalpass.
												GetAntialiasingThreshold () * 100.0f, " %");
					}, [&] (int what) {
						GLfloat threshold = renderer->finalpass.GetAntialiasingThreshold ();
						threshold += what * timefactor * 0.01;
						renderer->finalpass.SetAntialiasingThreshold (threshold);
					}, true },
				{ "Back", NULL, [&](int what) {
						if (!what)
						{
							menu = MAIN_MENU;
							submenu = 0;
						}
					}, false }
			}
		}
	};
}

Interface::~Interface (void)
{
}

void Interface::AddLight (void)
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
	light.CalculateFrustum ();
	renderer->AddLight (light);
}

void Interface::AddShadow (void)
{
	Shadow shadow;
	shadow.position = glm::vec4 (0, 10, 0, 0);
	shadow.direction = glm::vec4 (0, -1, 0, 0);
	shadow.spot.angle = DRE_PI/4.0f;
	shadow.spot.cosine = cosf (shadow.spot.angle);
	renderer->shadows.push_back (shadow);
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
		if (showInterface == 1)
			 showInterface = 0;
		else
			 showInterface = 1;
		return;
	}
	if (key == 'I')
	{
		if (showInterface == 2)
			 showInterface = 0;
		else
			 showInterface = 2;
	}
	if (showInterface == 1)
	{
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
				menus[menu].entries[submenu].handler (0);
				break;
			case GLFW_KEY_LEFT:
				if (!menus[menu].entries[submenu].repeating)
					 menus[menu].entries[submenu].handler (-1);
				break;
			case GLFW_KEY_RIGHT:
				if (!menus[menu].entries[submenu].repeating)
					 menus[menu].entries[submenu].handler (1);
				break;
			case GLFW_KEY_ESC:
				if (menu == MAIN_MENU)
				{
					showInterface = 0;
				}
				else
				{
					menu = MAIN_MENU;
					submenu = 0;
				}
				break;
			}
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
			 menus[menu].entries[submenu].handler (1);
	}

	if (glfwGetKey (GLFW_KEY_LEFT))
	{
		if (menus[menu].entries[submenu].repeating)
			 menus[menu].entries[submenu].handler (-1);
	}

	font.Frame ();
	if (showInterface == 1)
	{
		font.SetColor (glm::vec3 (1, 1, 1));
		font.Print (menus[menu].title);
		if (menus[menu].info)
		{
			menus[menu].info ();
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
				menus[menu].entries[i].info ();
			}
			font.Print ('\n');
		}
	}
	else if (showInterface == 2)
	{
		font.SetColor (glm::vec3 (1, 1, 1));
		font.Print ("FPS: ", fps);
		if (gl::IsExtensionSupported ("GL_NVX_gpu_memory_info"))
		{
			GLint mem;
			GLint usedmem;
			gl::GetIntegerv (GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &mem);
			font.Print ("\nDedicated memory: ", mem >> 10, " MB");
			gl::GetIntegerv (GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &mem);
			usedmem = mem;
			font.Print ("\nTotal available memory: ", mem >> 10, " MB");
			gl::GetIntegerv (GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &mem);
			usedmem -= mem;
			font.Print ("\nCurrently available memory: ", mem >> 10, " MB");
			font.Print ("\nUsed memory: ", usedmem >> 10, " MB");
			gl::GetIntegerv (GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &mem);
			font.Print ("\nEviction count: ", mem);
			gl::GetIntegerv (GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, &mem);
			font.Print ("\nEvicted memory: ", mem >> 10, " MB");
		}
#ifdef DEBUG
		font.Print ("\nGBuffer memory: ", renderer->memory >> 20, " MB");
#endif
	}
	else
	{
		font.SetColor (glm::vec3 (1, 1, 1));
		font.Print ("FPS: ", fps);
		font.Print ("\nOcclusion culled: ", Model::culled);
		font.Print ("\nFrustum culled: ", renderer->culling.culled);
	}
	if (glfwGetKey ('A'))
	{
		renderer->camera.RotateY (timefactor);
	}
	if (glfwGetKey ('D'))
	{
		renderer->camera.RotateY (-timefactor);
	}
	if (glfwGetKey ('W'))
	{
		renderer->camera.MoveForward (timefactor * 10.0f);
	}
	if (glfwGetKey ('S'))
	{
		renderer->camera.MoveForward (-timefactor * 10.0f);
	}
	if (glfwGetKey ('Q'))
	{
		renderer->camera.MoveUp (-4.0f * timefactor);
	}
	if (glfwGetKey ('E'))
	{
		renderer->camera.MoveUp (4.0f * timefactor);
	}
	if (glfwGetKey ('R'))
	{
		renderer->camera.RotateUp (timefactor);
	}
	if (glfwGetKey ('F'))
	{
		renderer->camera.RotateUp (-timefactor);
	}
}
