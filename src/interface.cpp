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
#include <ctime>

#define MAIN_MENU                 0
#define EDIT_COMPOSITION          1
#define EDIT_LIGHTS               2
#define EDIT_SHADOWS              3
#define EDIT_LIGHT_POSITION       4
#define EDIT_LIGHT_DIRECTION      5
#define EDIT_LIGHT_DIFFUSE        6
#define EDIT_LIGHT_SPECULAR       7
#define EDIT_LIGHT_ATTENUATION    8
#define EDIT_SHADOW_POSITION      9
#define EDIT_SHADOW_DIRECTION     10
#define EDIT_TONE_MAPPING         11
#define EDIT_GLOW                 12
#define EDIT_TONE_MAPPING_AVG_LUM 13
#define EDIT_ANTIALIASING         14
#define EDIT_PARAMS               15
#define EDIT_PARAMS_SPECULAR      16
#define EDIT_PARAMS_REFLECTION    17
#define EDIT_SKY                  18
#define EDIT_SKY_COEFFICIENTS_Y   19
#define EDIT_SKY_COEFFICIENTS_x   20
#define EDIT_SKY_COEFFICIENTS_y   12

extern bool running;

int ToOrdinalDate (int month, int day)
{
	if (month < 2)
		 return month * 31 + day;
	return int (30.6 * (month + 1) - 91.4) + day + 59;
}

void ToCalendarDate (int ordinal, int *month, int *day)
{
	time_t t = ordinal * 24 * 3600;
	struct tm *tm = gmtime (&t);
	if (month) *month = tm->tm_mon;
	if (day) *day = tm->tm_mday;
	return;
}

Interface::Interface (void)
	: showInterface (0), menu (MAIN_MENU), submenu (0),
		active_light (0), active_shadow (0), timefactor (0),
		active_parameter (0)
{
	menus = {
		{
			"Main Menu", NULL,
			{
				{ "Lights", NULL, [&] (int what) {
						if (!what)
						{
							if (r->GetNumLights () == 0)
								 AddLight ();
							menu = EDIT_LIGHTS;
							submenu = 0;
						}
					}, false },
				{ "Tile-based light culling: ", [&] (void) {
						if (r->composition.GetTileBased ())
							 font.Print ("yes");
						else
							 font.Print ("no");
					}, [&] (int what) {
						if (!what)
						{
							r->composition.SetTileBased (!r->composition.GetTileBased ());
						}
					}, false },
				{ "Shadows", NULL, [&] (int what) {
						if (!what)
						{
							if (!r->shadows.size ())
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
						font.Print (rendermodes[r->postprocess.GetRenderMode ()]);
					}, [&] (int what) {
						GLint rendermode = r->postprocess.GetRenderMode ();
						rendermode += what;
						if (rendermode < 0)
							 rendermode += NUM_RENDERMODES;
						if (rendermode >= NUM_RENDERMODES)
							 rendermode = 0;
						r->postprocess.SetRenderMode (rendermode);
					}, false },
				{ "Material Parameters", NULL,
					[&] (int what) {
						if (!what)
						{
							menu = EDIT_PARAMS;
							submenu = 0;
						}
					}, false },
				{ "Composition", NULL,
					[&] (int what) {
						if (!what)
						{
							menu = EDIT_COMPOSITION;
							submenu = 0;
						}
					}, false },
				{ "Sky", NULL,
					[&] (int what) {
						if (!what)
						{
							menu = EDIT_SKY;
							submenu = 0;
						}
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
			"Edit Composition", NULL,
			{
				{ "Screen limit: ", [&] (void) {
						font.Print (r->composition.GetScreenLimit ());
					}, [&] (int what) {
						float limit = r->composition.GetScreenLimit ();
						limit += what * timefactor;
						r->composition.SetScreenLimit (limit);
					}, true	},
				{ "Compositionmode: ", [&] (void) {
						font.Print (r->composition.GetMode ());
					}, [&] (int what) {
						GLint mode = r->composition.GetMode ();
						mode += what;
						if (mode < 0)
							 mode = 0;
						r->composition.SetMode (mode);
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
			"Edit Lights ", [&] (void) {
				font.Print (active_light + 1, " / ", r->GetNumLights ());
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
							 active_light += r->GetNumLights ();
						if (active_light >= r->GetNumLights ())
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
						font.Print (r->GetLight (active_light).spot.angle
												* 180.0f / DRE_PI);
					}, [&] (int what) {
						r->GetLight (active_light).spot.angle += what * timefactor;
						r->UpdateLight (active_light);
					}, true },
				{ "Spot Penumbra Angle ", [&] (void) {
						font.Print (r->GetLight (active_light).spot.penumbra_angle
												* 180.0f / DRE_PI);
					}, [&] (int what) {
						r->GetLight (active_light).spot.penumbra_angle
						+= what * timefactor;
						r->UpdateLight (active_light);
					}, true },
				{ "Spot Exponent ", [&] (void) {
						font.Print (r->GetLight (active_light).spot.exponent);
					}, [&] (int what) {
						r->GetLight (active_light).spot.exponent
						+= what * timefactor;
						r->UpdateLight (active_light);
					}, true },
				{ "Remove", NULL, [&] (int what) {
						if (!what)
						{
							r->RemoveLight (active_light);
							if (r->GetNumLights () == 0)
							{
								active_light = 0;
								menu = MAIN_MENU;
								submenu = 0;
							}
							if (active_light >= r->GetNumLights ())
								 active_light = 0;
						}
					}, false },
				{ "Randomize lights", NULL, [&] (int what) {
						if (!what)
						{
							srand (time (NULL));
							for (auto i = 0; i < r->GetNumLights (); i++)
							{
								Light &light = r->GetLight (i);
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
								r->UpdateLight (i);
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
				font.Print (active_shadow + 1, " / ", r->shadows.size ());
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
							 active_shadow += r->shadows.size ();
						if (active_shadow >= r->shadows.size ())
							 active_shadow = 0;
					}, false },
				{ "Edit Position", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_SHADOW_POSITION;
							submenu = 0;
						}
					}, false },
				{ "Edit Direction", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_SHADOW_DIRECTION;
							submenu = 0;
						}
					}, false },
				{ "Angle ", [&] (void) {
						font.Print (r->shadows[active_shadow].spot.angle
												* 180.0f / DRE_PI);
					}, [&] (int what) {
						r->shadows[active_shadow].spot.angle += what * timefactor;
						r->shadows[active_shadow].spot.cosine =
						cosf (r->shadows[active_shadow].spot.angle);
					}, true	},
				{ "Remove", NULL, [&] (int what) {
						if (!what)
						{
							if (r->shadows.size () < 2)
								 return;
							
							r->shadows.erase (r->shadows.begin ()
																			 + active_shadow);
							if (active_shadow >= r->shadows.size ())
								 active_light = 0;
							if (!r->shadows.size ())
							{
								menu = MAIN_MENU;
								submenu = 0;
							}
						}
					}, false },
				{ "Shadow Alpha: ", [&] (void) {
						font.Print (r->composition.GetShadowAlpha ());
				}, [&] (int what) {
						GLfloat alpha;
						alpha = r->composition.GetShadowAlpha ();
						alpha += timefactor * what;
						alpha = (alpha>=0?alpha:0);
						alpha = (alpha<=1?alpha:1);
						r->composition.SetShadowAlpha (alpha);
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
			"Light Position ", [&] (void) {
				font.Print (active_light + 1, " / ", r->GetNumLights ());
			},
			{
				{ "X ", [&] (void) {
						font.Print (r->GetLight (active_light).position.x);
					}, [&] (int what) {
						r->GetLight (active_light).position.x += what * timefactor;
						r->UpdateLight (active_light);
					}, true },
				{ "Y ", [&] (void) {
						font.Print (r->GetLight (active_light).position.y);
					}, [&] (int what) {
						r->GetLight (active_light).position.y += what * timefactor;
						r->UpdateLight (active_light);
					}, true },
				{ "Z ", [&] (void) {
						font.Print (r->GetLight (active_light).position.z);
					}, [&] (int what) {
						r->GetLight (active_light).position.z += what * timefactor;
						r->UpdateLight (active_light);
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
				font.Print (active_light + 1, " / ", r->GetNumLights ());
			},
			{
				{ "X ", [&] (void) {
						font.Print (r->GetLight (active_light).direction.x);
					}, [&] (int what) {
						r->GetLight (active_light).direction.x += what * timefactor;
						r->UpdateLight (active_light);
					}, true },
				{ "Y ", [&] (void) {
						font.Print (r->GetLight (active_light).direction.y);
					}, [&] (int what) {
						r->GetLight (active_light).direction.y += what * timefactor;
						r->UpdateLight (active_light);
					}, true },
				{ "Z ", [&] (void) {
						font.Print (r->GetLight (active_light).direction.z);
					}, [&] (int what) {
						r->GetLight (active_light).direction.z += what * timefactor;
						r->UpdateLight (active_light);
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
				font.Print (active_light + 1, " / ", r->GetNumLights ());
			},
			{
				{ "R ", [&] (void) {
						font.Print (r->GetLight (active_light).color.r);
					}, [&] (int what) {
						r->GetLight (active_light).color.r += what * timefactor;
						r->UpdateLight (active_light);
					}, true },
				{ "G ", [&] (void) {
						font.Print (r->GetLight (active_light).color.g);
					}, [&] (int what) {
						r->GetLight (active_light).color.g += what * timefactor;
						r->UpdateLight (active_light);
					}, true },
				{ "B ", [&] (void) {
						font.Print (r->GetLight (active_light).color.b);
					}, [&] (int what) {
						r->GetLight (active_light).color.b += what * timefactor;
						r->UpdateLight (active_light);
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
				font.Print (active_light + 1, " / ", r->GetNumLights ());
			},
			{
				{ "R ", [&] (void) {
						font.Print (r->GetLight (active_light).specular.color.r);
					}, [&] (int what) {
						r->GetLight (active_light).specular.color.r
						+= what * timefactor;
						r->UpdateLight (active_light);
					}, true },
				{ "G ", [&] (void) {
						font.Print (r->GetLight (active_light).specular.color.g);
					}, [&] (int what) {
						r->GetLight (active_light).specular.color.g
						+= what * timefactor;
						r->UpdateLight (active_light);
					}, true },
				{ "B ", [&] (void) {
						font.Print (r->GetLight (active_light).specular.color.b);
					}, [&] (int what) {
						r->GetLight (active_light).specular.color.b
						+= what * timefactor;
						r->UpdateLight (active_light);
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
				font.Print (active_light + 1, " / ", r->GetNumLights ());
			},
			{
				{ "Constant ", [&] (void) {
						font.Print (r->GetLight (active_light).attenuation.x);
					}, [&] (int what) {
						r->GetLight (active_light).attenuation.x
						+= what * timefactor * 0.1;
						r->UpdateLight (active_light);
					}, true },
				{ "Linear ", [&] (void) {
						font.Print (r->GetLight (active_light).attenuation.y);
					}, [&] (int what) {
						r->GetLight (active_light).attenuation.y
						+= what * timefactor * 0.1;
						r->UpdateLight (active_light);
					}, true },
				{ "Quadratic ", [&] (void) {
						font.Print (r->GetLight (active_light).attenuation.z * 100);
					}, [&] (int what) {
						r->GetLight (active_light).attenuation.z
						+= what * timefactor * 0.001;
						r->UpdateLight (active_light);
					}, true },
				{ "Max Distance ", [&] (void) {
						font.Print (r->GetLight (active_light).attenuation.w);
					}, [&] (int what) {
						r->GetLight (active_light).attenuation.w
						+= what * timefactor * 0.1;
						r->UpdateLight (active_light);
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
				font.Print (active_shadow + 1, " / ", r->shadows.size ());
			},
			{
				{ "X ", [&] (void) {
						font.Print (r->shadows[active_shadow].position.x);
					}, [&] (int what) {
						r->shadows[active_shadow].position.x += what * timefactor;
					}, true },
				{ "Y ", [&] (void) {
						font.Print (r->shadows[active_shadow].position.y);
					}, [&] (int what) {
						r->shadows[active_shadow].position.y += what * timefactor;
					}, true },
				{ "Z ", [&] (void) {
						font.Print (r->shadows[active_shadow].position.z);
					}, [&] (int what) {
						r->shadows[active_shadow].position.z += what * timefactor;
					}, true },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							if (!r->shadows.size ())
							{
								AddShadow ();
							}
							menu = EDIT_SHADOWS;
							submenu = 0;
						}
					}, false }
			}
		},
		{
			"Shadow Direction ", [&] (void) {
				font.Print (active_shadow + 1, " / ", r->shadows.size ());
			},
			{
				{ "X ", [&] (void) {
						font.Print (r->shadows[active_shadow].direction.x);
					}, [&] (int what) {
						r->shadows[active_shadow].direction.x += what * timefactor;
						r->shadows[active_shadow].direction =
						glm::normalize (r->shadows[active_shadow].direction);
					}, true },
				{ "Y ", [&] (void) {
						font.Print (r->shadows[active_shadow].direction.y);
					}, [&] (int what) {
						r->shadows[active_shadow].direction.y += what * timefactor;
						r->shadows[active_shadow].direction =
						glm::normalize (r->shadows[active_shadow].direction);
					}, true },
				{ "Z ", [&] (void) {
						font.Print (r->shadows[active_shadow].direction.z);
					}, [&] (int what) {
						r->shadows[active_shadow].direction.z += what * timefactor;
						r->shadows[active_shadow].direction =
						glm::normalize (r->shadows[active_shadow].direction);
					}, true },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							if (!r->shadows.size ())
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
						font.Print (tone_mapping_modes [r->postprocess.
																						GetTonemappingMode ()]);
					}, [&] (int what) {
						GLint mode;
						mode = r->postprocess.GetTonemappingMode ();
						mode += what;
						if (mode < 0)
							 mode += NUM_TONE_MAPPING_MODES;
						if (mode >= NUM_TONE_MAPPING_MODES)
							 mode = 0;
						r->postprocess.SetTonemappingMode (mode);
					}, false },
				{ "Image Key ", [&] (void) {
						font.Print (r->postprocess.GetImageKey ());
					}, [&] (int what) {
						float key;
						key = r->postprocess.GetImageKey ();
						key += what * timefactor;
						r->postprocess.SetImageKey (key);
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
						font.Print (r->postprocess.GetWhiteThreshold ());
					}, [&] (int what) {
						float threshold;
						threshold = r->postprocess.GetWhiteThreshold ();
						threshold += what * timefactor;
						r->postprocess.SetWhiteThreshold (threshold);
					}, true },
				{ "Sigma: ", [&] (void) {
						font.Print (r->postprocess.GetTonemappingSigma ());
					}, [&] (int what) {
						float sigma;
						sigma = r->postprocess.GetTonemappingSigma ();
						sigma += what * timefactor;
						r->postprocess.SetTonemappingSigma (sigma);
					}, true },
				{ "n: ", [&] (void) {
						font.Print (r->postprocess.GetTonemappingExponent ());
					}, [&] (int what) {
						float n;
						n = r->postprocess.GetTonemappingExponent ();
						n += what * timefactor;
						r->postprocess.SetTonemappingExponent (n);
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
						font.Print (rgb_working_spaces[r->postprocess.
																					 GetRGBWorkingSpace ()]);
					}, [&] (int what) {
						GLint rgb_working_space;
						rgb_working_space = r->postprocess.GetRGBWorkingSpace ();
						rgb_working_space += what;
						if (rgb_working_space < 0)
							 rgb_working_space += NUM_RGB_WORKING_SPACES;
						if (rgb_working_space >= NUM_RGB_WORKING_SPACES)
							 rgb_working_space = 0;
						r->postprocess.SetRGBWorkingSpace (rgb_working_space);
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
						font.Print (r->composition.GetGlow ().GetSize ());
					}, [&] (int what) {
						GLint size = r->composition.GetGlow ().GetSize ();
						size += what * 4;
						r->composition.GetGlow ().SetSize ((size>=0)?size:0);
					}, false },
				{ "Luminance Threshold: ", [&] (void) {
						font.Print (r->composition.GetLuminanceThreshold ());
					}, [&] (int what) {
						float threshold;
						threshold = r->composition.GetLuminanceThreshold ();
						threshold += what * timefactor;
						r->composition.SetLuminanceThreshold (threshold);
					}, true },
				{ "Limit: ", [&] (void) {
						font.Print (r->composition.GetGlow ().GetLimit ());
					}, [&] (int what) {
						GLfloat limit = r->composition.GetGlow ().GetLimit ();
						limit += what * timefactor;
						r->composition.GetGlow ().SetLimit (limit);
					}, true },
				{ "Exponent: ", [&] (void) {
						font.Print (r->composition.GetGlow ().GetExponent ());
					}, [&] (int what) {
						GLfloat exp = r->composition.GetGlow ().GetExponent ();
						exp += what * timefactor;
						r->composition.GetGlow ().SetExponent (exp);
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
						c = r->postprocess.GetAvgLumConst ();
						font.Print (c);
					}, [&] (int what) {
						float c;
						c = r->postprocess.GetAvgLumConst ();
						c += what * timefactor;
						r->postprocess.SetAvgLumConst (c);
					}, true },
				{ "Linear: ", [&] (void) {
						float c;
						c = r->postprocess.GetAvgLumLinear ();
						font.Print (c);
					}, [&] (int what) {
						float c;
						c = r->postprocess.GetAvgLumLinear ();
						c += what * timefactor;
						r->postprocess.SetAvgLumLinear (c);
					}, true },
				{ "Delta: ", [&] (void) {
						float c;
						c = r->postprocess.GetAvgLumDelta ();
						font.Print (c);
					}, [&] (int what) {
						float c;
						c = r->postprocess.GetAvgLumDelta ();
						c += what * timefactor;
						r->postprocess.SetAvgLumDelta (c);
					}, true },
				{ "Lod: ", [&] (void) {
						float c;
						c = r->postprocess.GetAvgLumLod ();
						font.Print (c);
					}, [&] (int what) {
						float c;
						c = r->postprocess.GetAvgLumLod ();
						c += what * 0.1;
						r->postprocess.SetAvgLumLod (c);
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
						font.Print (r->GetAntialiasing ());
					}, [&] (int what) {
						if (!what)
						{
							GLuint samples = r->GetAntialiasing ();
							samples += 4;
							if (samples > 16) samples = 0;
							r->SetAntialiasing (samples);
						}
					}, true },
				{ "Threshold: ", [&] (void) {
						font.Print (r->postprocess.
												GetAntialiasingThreshold () * 100.0f, " %");
					}, [&] (int what) {
						GLfloat threshold = r->postprocess.GetAntialiasingThreshold ();
						threshold += what * timefactor * 0.01;
						r->postprocess.SetAntialiasingThreshold (threshold);
					}, true },
				{ "Back", NULL, [&](int what) {
						if (!what)
						{
							menu = MAIN_MENU;
							submenu = 0;
						}
					}, false }
			}
		},
		{
			"Edit Material Parameters: \"", [&] (void) {
				font.Print (r->GetParameterName (active_parameter),
										"\" (", active_parameter + 1, " / ",
										r->GetNumParameters (), ")");
			},
			{
				{ "Select Material", NULL , [&] (int what) {
						active_parameter += what;
						if (active_parameter < 0)
							 active_parameter += r->GetNumParameters ();
						if (active_parameter >= r->GetNumParameters ())
							 active_parameter = 0;
					}, false },
				{ "Specular", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_PARAMS_SPECULAR;
							submenu = 0;
						}
					}, false },
				{ "Reflection", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_PARAMS_REFLECTION;
							submenu = 0;
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
			"Edit Specular Parameters: \"", [&] (void) {
				font.Print (r->GetParameterName (active_parameter),
										"\" (", active_parameter + 1, " / ",
										r->GetNumParameters (), ")");
			},
			{
				{ "Model: ", [&] (void) {
#define NUM_SPECULAR_MODELS 4
						const char *models[NUM_SPECULAR_MODELS] = {
							"None", "Gaussian", "Phong", "Beckmann"
						};
						font.Print (models[r->GetParameters
															 (active_parameter).specular.model]);
					}, [&] (int what) {
						int model;
						model = r->GetParameters (active_parameter).specular.model;
						model += what;
						if (model >= NUM_SPECULAR_MODELS)
							 model = 0;
						if (model < 0)
							 model = NUM_SPECULAR_MODELS - 1;
						r->GetParameters (active_parameter).specular.model = model;
						r->UpdateParameters (active_parameter);
					}, false },
				{ "", [&] (void) {
						float val;
						val = r->GetParameters (active_parameter).specular.param1;
						switch (r->GetParameters (active_parameter).specular.model)
						{
						case 0:
							font.Print ("Ignored: ", val);
							break;
						case 1:
						case 3:
							font.Print ("Smoothness: ", val);
							break;
						case 2:
							font.Print ("Shininess: ", val);
							break;
						}
					}, [&] (int what) {
						if (!what) {
							float defaults[NUM_SPECULAR_MODELS] = {
								0.25, 0.25, 2.0, 0.25
							};
							r->GetParameters (active_parameter).specular.param1
							= defaults [r->GetParameters (active_parameter).specular.model];
							r->UpdateParameters (active_parameter);
						} else {
							float val;
							val = r->GetParameters (active_parameter).specular.param1;
							val += what * timefactor;
							r->GetParameters (active_parameter).specular.param1 = val;
							r->UpdateParameters (active_parameter);
						}
					}, true },
				{ "", [&] (void) {
						float val;
						val = r->GetParameters (active_parameter).specular.param2;
						switch (r->GetParameters (active_parameter).specular.model)
						{
						case 0:
						case 2:
						case 3:
							font.Print ("Ignored: ", val);
							break;
						case 1:
							font.Print ("Gauss factor: ", val);
							break;
						}
					}, [&] (int what) {
						if (!what) {
							float defaults[NUM_SPECULAR_MODELS] = {
								1.0, 1.0, 1.0, 1.0
							};
							r->GetParameters (active_parameter).specular.param2
							= defaults [r->GetParameters (active_parameter).specular.model];
							r->UpdateParameters (active_parameter);
						} else {
							float val;
							val = r->GetParameters (active_parameter).specular.param2;
							val += what * timefactor;
							r->GetParameters (active_parameter).specular.param2 = val;
							r->UpdateParameters (active_parameter);
						}
					}, true },
				{ "Fresnel n: ", [&] (void) {
						font.Print (r->GetParameters (active_parameter).specular.fresnel.n);
					}, [&] (int what) {
						float val;
						val = r->GetParameters (active_parameter).specular.fresnel.n;
						val += what * timefactor;
						if (val < 0)
							 val = 0;
						r->GetParameters (active_parameter).specular.fresnel.n = val;
						r->UpdateParameters (active_parameter);
					}, true },
				{ "Fresnel k: ", [&] (void) {
						font.Print (r->GetParameters (active_parameter).specular.fresnel.k);
					}, [&] (int what) {
						float val;
						val = r->GetParameters (active_parameter).specular.fresnel.k;
						val += what * timefactor;
						if (val < 0)
							 val = 0;
						r->GetParameters (active_parameter).specular.fresnel.k = val;
						r->UpdateParameters (active_parameter);
					}, true },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_PARAMS;
							submenu = 0;
						}
					}, false }
			}
		},
		{
			"Edit Reflection Parameters: \"", [&] (void) {
				font.Print (r->GetParameterName (active_parameter),
										"\" (", active_parameter + 1, " / ",
										r->GetNumParameters (), ")");
			},
			{
				{ "Factor: ", [&] (void) {
						font.Print (r->GetParameters(active_parameter).reflection.factor
												* 100.0f,	" %");
					}, [&] (int what) {
						float ref = r->GetParameters(active_parameter).reflection.factor;
						ref += what * timefactor * 0.01f;
						if (ref < 0) ref = 0;
						if (ref > 1) ref = 1;
						r->GetParameters(active_parameter).reflection.factor = ref;
						r->UpdateParameters (active_parameter);
					}, true },
				{ "Fresnel n: ", [&] (void) {
						font.Print (r->GetParameters (active_parameter)
												.reflection.fresnel.n);
					}, [&] (int what) {
						float val;
						val = r->GetParameters (active_parameter).reflection.fresnel.n;
						val += what * timefactor;
						if (val < 0)
							 val = 0;
						r->GetParameters (active_parameter).reflection.fresnel.n = val;
						r->UpdateParameters (active_parameter);
					}, true },
				{ "Fresnel k: ", [&] (void) {
						font.Print (r->GetParameters (active_parameter)
												.reflection.fresnel.k);
					}, [&] (int what) {
						float val;
						val = r->GetParameters (active_parameter).reflection.fresnel.k;
						val += what * timefactor;
						if (val < 0)
							 val = 0;
						r->GetParameters (active_parameter).reflection.fresnel.k = val;
						r->UpdateParameters (active_parameter);
					}, true },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_PARAMS;
							submenu = 0;
						}
					}, false }
			}
		},
		{
			"Edit Sky Parameters", NULL,
			{
				{ "Turbidity ", [&] (void) {
						font.Print (r->composition.GetTurbidity ());
					}, [&] (int what) {
						float T = r->composition.GetTurbidity ();
						T += what * timefactor;
						r->composition.SetTurbidity (T);
					}, true },
				{ "Coefficients Y", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_SKY_COEFFICIENTS_Y;
							submenu = 0;
						}
					}, false },
				{ "Coefficients x", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_SKY_COEFFICIENTS_x;
							submenu = 0;
						}
					}, false },
				{ "Coefficients y", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_SKY_COEFFICIENTS_y;
							submenu = 0;
						}
					}, false },
				{ "Latitude ", [&] (void) {
						font.Print (r->composition.GetLatitude ());
					}, [&] (int what) {
						float L = r->composition.GetLatitude ();
						L += what * timefactor;
						r->composition.SetLatitude (L);
					}, true },
				{ "Month ", [&] (void) {
						const char *months[] = {
							"January", "February","March", "April", "May", "June",
							"July", "August", "September", "October", "November",
							"December"
						};
						int month;
						ToCalendarDate (r->composition.GetDate (), &month, NULL);
						font.Print (months[month]);
					}, [&] (int what) {
							int month, day;
							ToCalendarDate (r->composition.GetDate (), &month, &day);
							month += what;
							if (month < 0) month += 12;
							month %= 12;
							r->composition.SetDate (ToOrdinalDate (month, day));
					}, false },
				{ "Day ", [&] (void) {
						int day;
						ToCalendarDate (r->composition.GetDate (), NULL, &day);
						font.Print (day);
					}, [&] (int what) {
						int date;
						date = r->composition.GetDate ();
						date += what;
						if (date <= 0) date = 1;
						if (date > 365) date = 365;
						r->composition.SetDate (date);
					}, false },
				{ "Time ", [&] (void) {
						font.Print (r->composition.GetTimeOfDay ());
					}, [&] (int what) {
						float time;
						time = r->composition.GetTimeOfDay ();
						time += what * timefactor;
						if (time >= 24)
							 time = 24;
						if (time < 0)
							 time = 0;
						r->composition.SetTimeOfDay (time);
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
			"Edit Sky Coeficients Y", NULL,
			{
				{ "Darkening/Brightening of the horizon ", [&] (void) {
						font.Print (r->composition.GetPerezY (0));
					}, [&] (int what) {
						float v = r->composition.GetPerezY (0);
						v += what * timefactor * 0.1f;
						r->composition.SetPerezY (0, v);
					}, true },
				{ "Luminance gradient near the horizon ", [&] (void) {
						font.Print (r->composition.GetPerezY (1));
					}, [&] (int what) {
						float v = r->composition.GetPerezY (1);
						v += what * timefactor * 0.1f;
						r->composition.SetPerezY (1, v);
					}, true },
				{ "Relative intensity of circumsolar region ", [&] (void) {
						font.Print (r->composition.GetPerezY (2));
					}, [&] (int what) {
						float v = r->composition.GetPerezY (2);
						v += what * timefactor * 0.1f;
						r->composition.SetPerezY (2, v);
					}, true },
				{ "Width of the circumsolar region ", [&] (void) {
						font.Print (r->composition.GetPerezY (3));
					}, [&] (int what) {
						float v = r->composition.GetPerezY (3);
						v += what * timefactor * 0.1f;
						r->composition.SetPerezY (3, v);
					}, true },
				{ "Relative backscatered light at the earth surface ", [&] (void) {
						font.Print (r->composition.GetPerezY (4));
					}, [&] (int what) {
						float v = r->composition.GetPerezY (4);
						v += what * timefactor * 0.1f;
						r->composition.SetPerezY (4, v);
					}, true },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_SKY;
							submenu = 0;
						}
					}, false }
			}
		},
		{
			"Edit Sky Coeficients x", NULL,
			{
				{ "A ", [&] (void) {
						font.Print (r->composition.GetPerezx (0));
					}, [&] (int what) {
						float v = r->composition.GetPerezx (0);
						v += what * timefactor * 0.1f;
						r->composition.SetPerezx (0, v);
					}, true },
				{ "B ", [&] (void) {
						font.Print (r->composition.GetPerezx (1));
					}, [&] (int what) {
						float v = r->composition.GetPerezx (1);
						v += what * timefactor * 0.1f;
						r->composition.SetPerezx (1, v);
					}, true },
				{ "C ", [&] (void) {
						font.Print (r->composition.GetPerezx (2));
					}, [&] (int what) {
						float v = r->composition.GetPerezx (2);
						v += what * timefactor * 0.1f;
						r->composition.SetPerezx (2, v);
					}, true },
				{ "D ", [&] (void) {
						font.Print (r->composition.GetPerezx (3));
					}, [&] (int what) {
						float v = r->composition.GetPerezx (3);
						v += what * timefactor * 0.1f;
						r->composition.SetPerezx (3, v);
					}, true },
				{ "E ", [&] (void) {
						font.Print (r->composition.GetPerezx (4));
					}, [&] (int what) {
						float v = r->composition.GetPerezx (4);
						v += what * timefactor * 0.1f;
						r->composition.SetPerezx (4, v);
					}, true },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_SKY;
							submenu = 0;
						}
					}, false }
			}
		},
		{
			"Edit Sky Coeficients y", NULL,
			{
				{ "A ", [&] (void) {
						font.Print (r->composition.GetPerezy (0));
					}, [&] (int what) {
						float v = r->composition.GetPerezy (0);
						v += what * timefactor * 0.1f;
						r->composition.SetPerezy (0, v);
					}, true },
				{ "B ", [&] (void) {
						font.Print (r->composition.GetPerezy (1));
					}, [&] (int what) {
						float v = r->composition.GetPerezy (1);
						v += what * timefactor * 0.1f;
						r->composition.SetPerezy (1, v);
					}, true },
				{ "C ", [&] (void) {
						font.Print (r->composition.GetPerezy (2));
					}, [&] (int what) {
						float v = r->composition.GetPerezy (2);
						v += what * timefactor * 0.1f;
						r->composition.SetPerezy (2, v);
					}, true },
				{ "D ", [&] (void) {
						font.Print (r->composition.GetPerezy (3));
					}, [&] (int what) {
						float v = r->composition.GetPerezy (3);
						v += what * timefactor * 0.1f;
						r->composition.SetPerezy (3, v);
					}, true },
				{ "E ", [&] (void) {
						font.Print (r->composition.GetPerezy (4));
					}, [&] (int what) {
						float v = r->composition.GetPerezy (4);
						v += what * timefactor * 0.1f;
						r->composition.SetPerezy (4, v);
					}, true },
				{ "Back", NULL, [&] (int what) {
						if (!what)
						{
							menu = EDIT_SKY;
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
	r->AddLight (light);
}

void Interface::AddShadow (void)
{
	Shadow shadow;
	shadow.position = glm::vec4 (0, 10, 0, 1);
	shadow.direction = glm::vec4 (0, -1, 0, 1);
	shadow.spot.angle = DRE_PI/4.0f;
	shadow.spot.cosine = cosf (shadow.spot.angle);
	r->shadows.push_back (shadow);
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

	if (glfwGetKey (GLFW_KEY_RIGHT) || glfwGetKey ('+')
			|| glfwGetKey (GLFW_KEY_KP_ADD))
	{
		if (menus[menu].entries[submenu].repeating)
			 menus[menu].entries[submenu].handler (1);
	}

	if (glfwGetKey (GLFW_KEY_LEFT) || glfwGetKey ('-')
			|| glfwGetKey (GLFW_KEY_KP_SUBTRACT))
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
		font.Print ("\nGBuffer memory: ", r->memory >> 20, " MB");
#endif
	}
	else
	{
		font.SetColor (glm::vec3 (1, 1, 1));
		font.Print ("FPS: ", fps);
		font.Print ("\nOcclusion culled: ", Model::culled);
		font.Print ("\nFrustum culled: ", r->culling.culled);
	}
	if (glfwGetKey ('A'))
	{
		r->camera.RotateY (timefactor);
	}
	if (glfwGetKey ('D'))
	{
		r->camera.RotateY (-timefactor);
	}
	if (glfwGetKey ('W'))
	{
		r->camera.MoveForward (timefactor * 10.0f);
	}
	if (glfwGetKey ('S'))
	{
		r->camera.MoveForward (-timefactor * 10.0f);
	}
	if (glfwGetKey ('Q'))
	{
		r->camera.MoveUp (-4.0f * timefactor);
	}
	if (glfwGetKey ('E'))
	{
		r->camera.MoveUp (4.0f * timefactor);
	}
	if (glfwGetKey ('R'))
	{
		r->camera.RotateUp (timefactor);
	}
	if (glfwGetKey ('F'))
	{
		r->camera.RotateUp (-timefactor);
	}
}
