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

const std::vector<Menu> menus = {
	{
		"Main Menu", NULL,
		{
			{ "Edit Lights", NULL, &Interface::EditLights },
			{ "Edit Shadows", NULL, &Interface::EditShadows },
			{ "Soft Shadows: ", &Interface::PrintSoftShadow,
				&Interface::ToggleSoftShadow },
			{ "Toggle Rendermode: ", &Interface::PrintRendermode,
				&Interface::ToggleRendermode },
			{ "Exit", NULL, &Interface::Exit }
		}
	},
	{
		"Edit Lights ", &Interface::PrintActiveLight,
		{
			{ "Add Light", NULL, &Interface::AddLight },
			{ "Next Light ", NULL , &Interface::NextLight },
			{ "Previous Light ", NULL , &Interface::PreviousLight },
			{ "Edit Position", NULL, &Interface::EditLightPosition },
			{ "Edit Direction", NULL, &Interface::EditLightDirection },
			{ "Edit Diffuse Color", NULL, &Interface::EditDiffuseColor },
			{ "Edit Specular Color", NULL, &Interface::EditSpecularColor },
			{ "Edit Shininess ", &Interface::PrintShininess,
				&Interface::EditShininess },
			{ "Edit Attenuation", NULL, &Interface::EditAttenuation },
			{ "Spot Angle ", &Interface::PrintSpotAngle,
				&Interface::EditSpotAngle },
			{ "Spot Exponent ", &Interface::PrintSpotExponent,
				&Interface::EditSpotExponent },
			{ "Remove", NULL, &Interface::RemoveLight },
			{ "Back", NULL, &Interface::MainMenu }
		}
	},
	{
		"Edit Shadows ", &Interface::PrintActiveShadow,
		{
			{ "Add Shadow", NULL, &Interface::AddShadow },
			{ "Next Shadow ", NULL , &Interface::NextShadow },
			{ "Previous Shadow ", NULL , &Interface::PreviousShadow },
			{ "Edit Position", NULL, &Interface::EditShadowPosition },
			{ "Remove", NULL, &Interface::RemoveShadow },
			{ "Back", NULL, &Interface::MainMenu }
		}
	},
	{
		"Light Position ", &Interface::PrintActiveLight,
		{
			{ "X ", &Interface::PrintLightX, &Interface::MoveLightX },
			{ "Y ", &Interface::PrintLightY, &Interface::MoveLightY },
			{ "Z ", &Interface::PrintLightZ, &Interface::MoveLightZ },
			{ "Back", NULL, &Interface::EditLights }
		}
	},
	{
		"Light Direction ", &Interface::PrintActiveLight,
		{
			{ "X ", &Interface::PrintLightDirX, &Interface::MoveLightDirX },
			{ "Y ", &Interface::PrintLightDirY, &Interface::MoveLightDirY },
			{ "Z ", &Interface::PrintLightDirZ, &Interface::MoveLightDirZ },
			{ "Back", NULL, &Interface::EditLights }
		}
	},
	{
		"Light Diffuse Color ", &Interface::PrintActiveLight,
		{
			{ "R ", &Interface::PrintDiffuseR, &Interface::EditDiffuseR },
			{ "G ", &Interface::PrintDiffuseG, &Interface::EditDiffuseG },
			{ "B ", &Interface::PrintDiffuseB, &Interface::EditDiffuseB },
			{ "Back", NULL, &Interface::EditLights }
		}
	},
	{
		"Light Specular Color ", &Interface::PrintActiveLight,
		{
			{ "R ", &Interface::PrintSpecularR, &Interface::EditSpecularR },
			{ "G ", &Interface::PrintSpecularG, &Interface::EditSpecularG },
			{ "B ", &Interface::PrintSpecularB, &Interface::EditSpecularB },
			{ "Back", NULL, &Interface::EditLights }
		}
	},
	{
		"Light Attenuation ", &Interface::PrintActiveLight,
		{
			{ "Constant ", &Interface::PrintConstantAttenuation,
				&Interface::EditConstantAttenuation },
			{ "Linear ", &Interface::PrintLinearAttenuation,
				&Interface::EditLinearAttenuation },
			{ "Quadratic ", &Interface::PrintQuadraticAttenuation,
				&Interface::EditQuadraticAttenuation },
			{ "Back", NULL, &Interface::EditLights }
		}
	},
	{
		"Shadow Position ", &Interface::PrintActiveShadow,
		{
			{ "X ", &Interface::PrintShadowX, &Interface::MoveShadowX },
			{ "Y ", &Interface::PrintShadowY, &Interface::MoveShadowY },
			{ "Z ", &Interface::PrintShadowZ, &Interface::MoveShadowZ },
			{ "Back", NULL, &Interface::EditShadows }
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
		light.color = glm::vec4 (1, 1, 1, 0);
		light.direction = glm::vec4 (0, -1, 0, 0);
		light.spot.cosine = cosf (M_PI/4.0f);
		light.spot.exponent = 2.0f;
		light.spot.angle = M_PI/4.0f;
		light.specular.color = glm::vec3 (1, 1, 1);
		light.specular.shininess = 8.0f;
		light.attenuation = glm::vec4 (0.0f, 0.0f, 0.007f, 0.0f);
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
		shadow.spot.cosine = cosf (M_PI/4.0f);
		shadow.spot.angle = M_PI/4.0f;
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

#define NUM_RENDERMODES 19

void Interface::ToggleRendermode (int what)
{
	if (!what)
	{
		GLuint rendermode = renderer->finalpass.GetRenderMode ();
		rendermode++;
		if (rendermode >= NUM_RENDERMODES)
			 rendermode = 0;
		renderer->finalpass.SetRenderMode (rendermode);
	}
}

extern bool running;

void Interface::Exit (int what)
{
	if (!what)
	{
		running = false;
	}
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
		"shadow mask",
		"shadow projection"
	};

	font.Print (rendermodes[renderer->finalpass.GetRenderMode ()]);
}

void Interface::NextShadow (int what)
{
	if (!what)
	{
		active_shadow++;
		if (active_shadow >= renderer->shadows.size ())
			 active_shadow = 0;
	}
}

void Interface::PreviousShadow (int what)
{
	if (!what)
	{
		if (!active_shadow)
			 active_shadow = renderer->shadows.size ();
		if (active_shadow)
			 active_shadow--;
	}
}

void Interface::NextLight (int what)
{
	if (!what)
	{
		active_light++;
		if (active_light >= renderer->lights.size ())
			 active_light = 0;
	}
}

void Interface::PreviousLight (int what)
{
	if (!what)
	{
		if (!active_light)
			 active_light = renderer->lights.size ();
		if (active_light)
			 active_light--;
	}
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
	value = renderer->shadowpass.GetSoftShadows ();
	renderer->shadowpass.SetSoftShadows (!value);
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
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].direction.x)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].direction.x),
			&renderer->lights[active_light].direction.x,
			0, NULL, NULL);
}

void Interface::MoveLightDirY (int what)
{
	renderer->lights[active_light].direction.y += what * timefactor;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].direction.y)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].direction.y),
			&renderer->lights[active_light].direction.y,
			0, NULL, NULL);
}

void Interface::MoveLightDirZ (int what)
{
	renderer->lights[active_light].direction.z += what * timefactor;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].direction.z)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].direction.z),
			&renderer->lights[active_light].direction.z,
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
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].spot.cosine)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].spot.cosine),
			&renderer->lights[active_light].spot.cosine,
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
	renderer->lights[active_light].attenuation.z += what * timefactor * 0.1;
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

void Interface::EditShininess (int what)
{
	renderer->lights[active_light].specular.shininess
		 += what * timefactor * 10.0;
	renderer->queue.EnqueueWriteBuffer
		 (renderer->lightmem, CL_TRUE,
			intptr_t (&renderer->lights[active_light].specular.shininess)
			- intptr_t (&renderer->lights[0]),
			sizeof (renderer->lights[active_light].specular.shininess),
			&renderer->lights[active_light].specular.shininess,
			0, NULL, NULL);
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
	if (renderer->shadowpass.GetSoftShadows ())
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
	font.Print (renderer->lights[active_light].spot.angle * 180.0f / M_PI);
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
	font.Print (renderer->lights[active_light].attenuation.z);
}

void Interface::PrintShininess (void)
{
	font.Print (renderer->lights[active_light].specular.shininess);
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
		 timefactor *= 10.0f;

	if (glfwGetKey (GLFW_KEY_LCTRL))
		 timefactor *= 50.0f;

	if (glfwGetTime () - last_time >= 1.0)
	{
		fps = frames;
		last_time = glfwGetTime ();
		frames = 0;
	}
	frames++;

	if (glfwGetKey (GLFW_KEY_RIGHT))
	{
		(this->*(menus[menu].entries[submenu].handler)) (1);
	}

	if (glfwGetKey (GLFW_KEY_LEFT))
	{
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
	}
	glm::vec3 dir (sin (renderer->camera.angle), 0,
								 cos (renderer->camera.angle));
	if (glfwGetKey ('A'))
	{
		renderer->camera.angle += timefactor;
	}
	if (glfwGetKey ('D'))
	{
		renderer->camera.angle -= timefactor;
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
