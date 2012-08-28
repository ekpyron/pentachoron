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
#include "interface.h"
#include "renderer.h"
#include <ctime>
#include <AntTweakBar.h>

unsigned int active_light = 0;
unsigned int active_parameter = 0;
extern unsigned int fps;

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

void AddLight (void)
{
	Light light;
	light.position = glm::vec4 (0, 10, 0, 0);
	light.color = glm::vec4 (1, 1, 1, 1);
	light.direction = glm::vec4 (0, -1, 0, 0);
	light.spot.exponent = 2.0f;
	light.spot.angle = PCH_PI/4.0f;
	light.spot.cosine = cosf (light.spot.angle);
	light.spot.tangent = tanf (light.spot.angle);
	light.spot.penumbra_angle = light.spot.angle * 0.8f;
	light.spot.penumbra_cosine = cosf (light.spot.penumbra_angle);
	light.specular.color = glm::vec3 (1, 1, 1);
	light.attenuation = glm::vec4 (0.0f, 0.0f, 0.007f, 50.0f);
	light.CalculateFrustum ();
	r->AddLight (light);

	std::ostringstream stream;
	stream << "lights/active visible=true min=0 max=" << r->GetNumLights () - 1;
	TwDefine (stream.str ().c_str ());
	if (r->GetNumLights () == 1)
	{
		TwDefine ("lights/position visible=true");
		TwDefine ("lights/direction visible=true");
		TwDefine ("lights/color visible=true");
		TwDefine ("lights/spot visible=true");
		TwDefine ("lights/attenuation visible=true");
		TwDefine ("lights/specular visible=true");
		TwDefine ("lights/remove visible=true");
	}
}

void RemoveLight (void)
{
	r->RemoveLight (active_light);
	if (active_light >= r->GetNumLights ())
		 active_light = 0;
	if (r->GetNumLights () == 0)
	{
		TwDefine ("lights/active visible=false");
		TwDefine ("lights/position visible=false");
		TwDefine ("lights/direction visible=false");
		TwDefine ("lights/color visible=false");
		TwDefine ("lights/spot visible=false");
		TwDefine ("lights/attenuation visible=false");
		TwDefine ("lights/specular visible=false");
		TwDefine ("lights/remove visible=false");
	}
	else
	{
		std::ostringstream stream;
		stream << "lights/active visible=true min=0 max="
					 << r->GetNumLights () - 1;
		TwDefine (stream.str ().c_str ());
	}
}

void CreateMenus (void)
{
	{
		TwBar *bar = TwNewBar ("pentachoron");
		TwDefine ("pentachoron label='Pentachoron'");

		TwType rendermodeType;
		TwEnumVal rendermodeEV[] = {
			{ 0, "compose" },
			{ 1, "color" },
			{ 2, "normal" },
			{ 3, "specular" },
			{ 4, "depth" },
			{ 5, "glow" },
			{ 6, "shadow" }
		};
		rendermodeType = TwDefineEnum ("rendermode", rendermodeEV, 7);

		TwAddVarCB (bar, "FPS", TW_TYPE_UINT32,
								NULL, [&] (void *v, void*) {
									*(unsigned int*)v = fps;
								}, NULL, NULL);
		TwAddVarCB (bar, "rendermode", rendermodeType,
								[&] (const void *v, void*) {
									r->postprocess.SetRenderMode (*(unsigned int*)v);
								}, [&] (void *v, void*) {
									*(unsigned int*)v = r->postprocess.GetRenderMode ();
								}, NULL, NULL);
		TwAddVarCB (bar, "tile-based light culling", TW_TYPE_BOOLCPP,
								[&] (const void *v, void*) {
									r->composition.SetTileBased (*(bool*)v);
								}, [&] (void *v, void*){
									*(bool*)v = r->composition.GetTileBased ();
								}, NULL, NULL);
		TwAddVarCB (bar, "wireframe", TW_TYPE_BOOLCPP,
								[&] (const void *v, void*) {
									r->gbuffer.SetWireframe (*(bool*)v);
								}, [&] (void *v, void*) {
									*(bool*)v = r->gbuffer.GetWireframe ();
								}, NULL, NULL);
		TwAddVarCB (bar, "tesslevel", TW_TYPE_UINT32,
								[&] (const void *v, void*) {
									r->geometry.SetTessLevel (*(uint32_t*)v);
								}, [&] (void *v, void*) {
									*(uint32_t*)v = r->geometry.GetTessLevel ();
								}, NULL, "label='tessellation level' min=1 max=64");
		TwAddVarCB (bar, "displacement", TW_TYPE_FLOAT,
								[&] (const void *v, void*) {
									r->geometry.SetDisplacement (*(float*)v);
								}, [&] (void *v, void*) {
									*(float*)v = r->geometry.GetDisplacement ();
								}, NULL, "label='displacement' min=0 step=0.01");
	}
	{
		TwBar *bar = TwNewBar ("lights");
		TwDefine ("lights label='Lights' iconified=true");

		TwAddButton (bar, "add light", [&] (void*) {
				AddLight ();
			}, NULL, NULL);
		TwAddButton (bar, "remove", [&] (void*) {
				RemoveLight ();
			}, NULL, "label='remove light' visible=false");

		TwAddVarRW (bar, "active", TW_TYPE_UINT32,
								&active_light, "label='active light' visible=false"); 
		TwAddSeparator (bar, NULL, NULL);

		TwStructMember lightposDesc[] = {
			{ "x", TW_TYPE_FLOAT, offsetof (glm::vec4, x), "step=0.1" },
			{ "y", TW_TYPE_FLOAT, offsetof (glm::vec4, y), "step=0.1" },
			{ "z", TW_TYPE_FLOAT, offsetof (glm::vec4, z), "step=0.1" }
		};
		TwStructMember attenuationDesc[] = {
			{ "constant", TW_TYPE_FLOAT, offsetof (glm::vec4, x), "step=0.01" },
			{ "linear", TW_TYPE_FLOAT, offsetof (glm::vec4, y), "step=0.01" },
			{ "quadratic", TW_TYPE_FLOAT, offsetof (glm::vec4, z), "step=0.01" },
			{ "cull distance", TW_TYPE_FLOAT, offsetof (glm::vec4, w), "step=0.1" }
		};
		typedef struct spotparam {
			 float angle;
			 float penumbra_angle;
			 float exponent;
		} spotparam_t;
		TwStructMember spotDesc[] = {
			{ "angle", TW_TYPE_FLOAT,
				offsetof (spotparam_t, angle), "step=1 min=0 max=360" },
			{ "penumbra angle", TW_TYPE_FLOAT,
				offsetof (spotparam_t, penumbra_angle), "step=1 min=0 max=360" },
			{ "exponent", TW_TYPE_FLOAT,
				offsetof (spotparam_t, exponent), "step=0.1" }
		};
		
		TwType lightposType = TwDefineStruct ("lightpos", lightposDesc, 3,
																					sizeof (glm::vec4),
																					NULL, NULL);
		TwType spotdescType = TwDefineStruct ("spotdesc", spotDesc, 3,
																					sizeof (spotparam_t),
																					NULL, NULL);
		TwType attenuationType = TwDefineStruct ("attenuation", attenuationDesc, 4,
																						 sizeof (glm::vec4),
																						 NULL, NULL);
		TwAddVarCB (bar, "position", lightposType,
								[&] (const void *v, void*) {
									r->GetLight (active_light).position
										 = *(glm::vec4*)v;
									r->UpdateLight (active_light);
								}, [&] (void *v, void*) {
									*(glm::vec4*)v = r->GetLight (active_light).position;
								}, NULL, "visible=false");
		TwAddVarCB (bar, "direction", TW_TYPE_DIR3F,
								[&] (const void *v, void*) {
									r->GetLight (active_light).direction
										 = glm::vec4 (*(const glm::vec3*)v, 0.0f);
									r->UpdateLight (active_light);
								}, [&] (void *v, void*) {
									*(glm::vec3*)v
										 = glm::vec3 (r->GetLight (active_light).direction);
								}, NULL, "visible=false");
		TwAddVarCB (bar, "color", TW_TYPE_COLOR3F,
								[&] (const void *v, void*) {
									r->GetLight (active_light).color
										 = glm::vec4 (*(const glm::vec3*)v, 1.0f);
									r->UpdateLight (active_light);
								}, [&] (void *v, void*) {
									*(glm::vec3*)v
										 = glm::vec3 (r->GetLight (active_light).color);
								}, NULL, "visible=false");
		TwAddVarCB (bar, "spot", spotdescType,
								[&] (const void *v, void*) {
									Light &light = r->GetLight (active_light);
									light.spot.exponent = ((spotparam_t*)v)->exponent;
									light.spot.angle = (((spotparam_t*)v)->angle)
										 * PCH_PI / 180.0f;
									light.spot.cosine = cosf (light.spot.angle);
									light.spot.tangent = tanf (light.spot.angle);
									light.spot.penumbra_angle
										 = ((spotparam_t*)v)->penumbra_angle * PCH_PI / 180.0f;
									light.spot.penumbra_cosine
										 = cosf (light.spot.penumbra_angle);
									r->UpdateLight (active_light);
								}, [&] (void *v, void*){
									const Light &light = r->GetLight (active_light);
									((spotparam_t*)v)->exponent = light.spot.exponent;
									((spotparam_t*)v)->angle = light.spot.angle
										 * 180.0f / PCH_PI;
									((spotparam_t*)v)->penumbra_angle
										 = light.spot.penumbra_angle * 180.0f / PCH_PI;
								}, NULL, "visible=false");
		TwAddVarCB (bar, "attenuation", attenuationType,
								[&] (const void *v, void*) {
									r->GetLight (active_light).attenuation
										 = *(glm::vec4*)v;
									r->GetLight (active_light).attenuation.z /= 100.0f;
									r->UpdateLight (active_light);
								}, [&] (void *v, void*) {
									*(glm::vec4*)v = r->GetLight (active_light).attenuation;
									((glm::vec4*)v)->z *= 100.0f;
								}, NULL, "visible=false");
		TwAddVarCB (bar, "specular", TW_TYPE_COLOR3F,
								[&] (const void *v, void*) {
									r->GetLight (active_light).specular.color
										 = *(const glm::vec3*)v;
									r->UpdateLight (active_light);
								}, [&] (void *v, void*) {
									*(glm::vec3*)v
										 = r->GetLight (active_light).specular.color;
								}, NULL, "label='specular color' visible=false");

		if (r->GetNumLights () >= 1)
		{
			TwDefine ("lights/position visible=true");
			TwDefine ("lights/direction visible=true");
			TwDefine ("lights/color visible=true");
			TwDefine ("lights/spot visible=true");
			TwDefine ("lights/attenuation visible=true");
			TwDefine ("lights/specular visible=true");
			TwDefine ("lights/remove visible=true");
		}
	}
	{
		TwBar *bar = TwNewBar ("sky");
		TwDefine ("sky label='Sky' iconified=true");

		typedef struct perezcoefficients {
			 union {
					struct {
						 float A, B, C, D, E;
					};
					float p[5];
			 };
		} perezcoefficients_t;
		TwStructMember perezDesc[] = {
			{ "Darkening/Brightening to the horizon",
				TW_TYPE_FLOAT, offsetof (perezcoefficients_t, A),
				"step=0.01" },
			{ "Luminance gradient near the horizon",
				TW_TYPE_FLOAT, offsetof (perezcoefficients_t, B),
				"step=0.01" },
			{ "Relative intensity of circumsolar region",
				TW_TYPE_FLOAT, offsetof (perezcoefficients_t, C),
				"step=0.01" },
			{ "Width of the circumsolar region",
				TW_TYPE_FLOAT, offsetof (perezcoefficients_t, D),
				"step=0.01" },
			{ "Relative backscatered light at the earth surface",
				TW_TYPE_FLOAT, offsetof (perezcoefficients_t, E),
				"step=0.01" }

		};
		TwType perezType = TwDefineStruct ("perez", perezDesc, 5,
																			 sizeof (perezcoefficients_t),
																			 NULL, NULL);

		TwAddVarCB (bar, "turbidity", TW_TYPE_FLOAT,
								[&] (const void *v, void*) {
									r->composition.SetTurbidity (*(const float*)v);
								}, [&] (void *v, void*) {
									*(float*)v = r->composition.GetTurbidity ();
								}, NULL, "step=0.1 min=1.0 max=10.0");
		TwAddVarCB (bar, "time", TW_TYPE_FLOAT,
								[&] (const void *v, void*) {
									r->composition.SetTimeOfDay (*(const float*)v);
								}, [&] (void *v, void*) {
									*(float*)v = r->composition.GetTimeOfDay ();
								}, NULL, "step=0.01 min=0.0 max=24.0");
		TwType monthType;
		TwEnumVal monthEV[] = {
			{  0, "January" },
			{  1, "February" },
			{  2, "March" },
			{  3, "April" },
			{  4, "May" },
			{  5, "June" },
			{  6, "July" },
			{  7, "August" },
			{  8, "September" },
			{  9, "October" },
			{ 10, "November" },
			{ 11, "December" }
		};
		monthType = TwDefineEnum ("month", monthEV, 12);
		TwAddVarCB (bar, "month", monthType,
								[&] (const void *v, void*) {
									int month, day;
									ToCalendarDate (r->composition.GetDate (), &month, &day);
									month = *(const unsigned int*)v;
									r->composition.SetDate (ToOrdinalDate (month, day));
								}, [&] (void *v, void*) {
									int month, day;
									ToCalendarDate (r->composition.GetDate (), &month, &day);
									*(unsigned int*)v = month;
								}, NULL, NULL);
		TwAddVarCB (bar, "day", TW_TYPE_UINT8,
								[&] (const void *v, void*) {
									int month, day;
									ToCalendarDate (r->composition.GetDate (), &month, &day);
									day = *(const unsigned char*)v;
									r->composition.SetDate (ToOrdinalDate (month, day));
								}, [&] (void *v, void*) {
									int month, day;
									ToCalendarDate (r->composition.GetDate (), &month, &day);
									*(unsigned char*)v = day;
								}, NULL, "min=1 max=31");
		TwAddVarCB (bar, "luminosity", TW_TYPE_FLOAT,
								[&] (const void *v, void*) {
									r->composition.SetSkyLuminosity (0.01f * *(const float*)v);
								}, [&] (void *v, void*) {
									*(float*)v = r->composition.GetSkyLuminosity () * 100.0f;
								}, NULL, "step=0.01");
		TwAddVarCB (bar, "latitude", TW_TYPE_FLOAT,
								[&] (const void *v, void*) {
									r->composition.SetLatitude (*(const float*)v);
								}, [&] (void *v, void*) {
									*(float*)v = r->composition.GetLatitude ();
								}, NULL, "step=0.1 min=0.0 max=90.0");
		TwAddVarCB (bar, "Y", perezType,
								[&] (const void *v, void*) {
									for (auto i = 0; i < 5; i++)
										 r->composition.SetPerezY
												(i, ((perezcoefficients_t*)v)->p[i]);
								}, [&] (void *v, void *) {
									for (auto i = 0; i < 5; i++)
										 ((perezcoefficients_t*)v)->p[i]
												= r->composition.GetPerezY (i);
								}, NULL, "label='Coefficients Y'");
		TwAddVarCB (bar, "x", perezType,
								[&] (const void *v, void*) {
									for (auto i = 0; i < 5; i++)
										 r->composition.SetPerezx
												(i, ((perezcoefficients_t*)v)->p[i]);
								}, [&] (void *v, void *) {
									for (auto i = 0; i < 5; i++)
										 ((perezcoefficients_t*)v)->p[i]
												= r->composition.GetPerezx (i);
								}, NULL, "label='Coefficients x'");
		TwAddVarCB (bar, "y", perezType,
								[&] (const void *v, void*) {
									for (auto i = 0; i < 5; i++)
										 r->composition.SetPerezy
												(i, ((perezcoefficients_t*)v)->p[i]);
								}, [&] (void *v, void *) {
									for (auto i = 0; i < 5; i++)
										 ((perezcoefficients_t*)v)->p[i]
												= r->composition.GetPerezy (i);
								}, NULL, "label='Coefficients y'");
	}
	{
		TwBar *bar = TwNewBar ("tonemapping");
		TwDefine ("tonemapping label='Tonemapping' iconified=true");

		TwEnumVal tonemappingmodeEV[] = {
			{  0, "Default" },
			{  1, "Reinhard" },
			{  2, "Logarithmic" },
			{  3, "URQ" },
			{  4, "Exponential" },
			{  5, "Drago" },
		};
		TwType tonemappingmodeType;
		tonemappingmodeType = TwDefineEnum ("tonemappingmode",
																				tonemappingmodeEV, 6);
		TwAddVarCB (bar, "Mode", tonemappingmodeType,
								[&] (const void *v, void*) {
									r->postprocess.SetTonemappingMode (*(const unsigned int*)v);
								}, [&] (void *v, void*) {
									*(unsigned int*)v
										 = r->postprocess.GetTonemappingMode ();
								}, NULL, NULL);
		TwAddVarCB (bar, "Image Key", TW_TYPE_FLOAT,
								[&] (const void *v, void*) {
									r->postprocess.SetImageKey (*(const float*)v);
								}, [&] (void *v, void*) {
									*(float*)v
										 = r->postprocess.GetImageKey ();
								}, NULL, "step=0.001 min=0.001 max=5.0");
		TwAddVarCB (bar, "White threshold", TW_TYPE_FLOAT,
								[&] (const void *v, void*) {
									r->postprocess.SetWhiteThreshold (*(const float*)v);
								}, [&] (void *v, void*) {
									*(float*)v = r->postprocess.GetWhiteThreshold ();
								}, NULL, "step=0.01 min=0.0 max=5.0");
		TwAddVarCB (bar, "Sigma", TW_TYPE_FLOAT,
								[&] (const void *v, void*) {
									r->postprocess.SetTonemappingSigma (*(const float*)v);
								}, [&] (void *v, void*) {
									*(float*)v = r->postprocess.GetTonemappingSigma ();
								}, NULL, "step=0.01 min=0.0 max=5.0");
		TwAddVarCB (bar, "Exponent", TW_TYPE_FLOAT,
								[&] (const void *v, void*) {
									r->postprocess.SetTonemappingExponent (*(const float*)v);
								}, [&] (void *v, void*) {
									*(float*)v = r->postprocess.GetTonemappingExponent ();
								}, NULL, "step=0.01 min=0.0");

		TwStructMember avglumDesc[] = {
			{ "constant", TW_TYPE_FLOAT, offsetof (glm::vec4, x), "step=0.01" },
			{ "linear", TW_TYPE_FLOAT, offsetof (glm::vec4, y), "step=0.01" },
			{ "delta", TW_TYPE_FLOAT, offsetof (glm::vec4, z), "step=0.01" },
			{ "lod", TW_TYPE_FLOAT, offsetof (glm::vec4, w), "step=0.1" }
		};
		TwType avglumType = TwDefineStruct ("avglum", avglumDesc, 4,
																				sizeof (glm::vec4),
																				NULL, NULL);
		TwAddVarCB (bar, "avgLum", avglumType,
								[&] (const void *v, void*) {
									r->postprocess.SetAvgLumConst (((const glm::vec4*)v)->x);
									r->postprocess.SetAvgLumLinear (((const glm::vec4*)v)->y);
									r->postprocess.SetAvgLumDelta (((const glm::vec4*)v)->z);
									r->postprocess.SetAvgLumLod (((const glm::vec4*)v)->w);
								}, [&] (void *v, void*) {
									((glm::vec4*)v)->x = r->postprocess.GetAvgLumConst ();
									((glm::vec4*)v)->y = r->postprocess.GetAvgLumLinear ();
									((glm::vec4*)v)->z = r->postprocess.GetAvgLumDelta ();
									((glm::vec4*)v)->w = r->postprocess.GetAvgLumLod ();
								}, NULL, "label='Average Luminance'");
		TwEnumVal rgbworkingspaceEV[] = {
			{ 0, "Adobe RGB (1998)" },
			{  1, "AppleRGB" },
			{  2, "Best RGB" },
			{  3, "Beta RGB" },
			{  4, "Bruce RGB" },
			{  5, "CIE RGB" },
			{  6, "ColorMatch RGB" },
			{  7, "Don RGB 4" },
			{  8, "ECI RGB" },
			{  9, "Ekta Space PS5" },
			{ 10, "NTSC RGB" },
			{ 11, "PAL/SECAM RGB" },
			{ 12, "ProPhoto RGB" },
			{ 13, "SMPTE-C RGB" },
			{ 14, "sRGB" },
			{ 15, "Wide Gamut RGB" }
		};
		TwType rgbworkingspaceType;
		rgbworkingspaceType = TwDefineEnum ("rgbworkingspace",
																				rgbworkingspaceEV, 16);
		TwAddVarCB (bar, "RGB Working Space", rgbworkingspaceType,
								[&] (const void *v, void*) {
									r->postprocess.SetRGBWorkingSpace (*(const unsigned int*)v);
								}, [&] (void *v, void*) {
									*(unsigned int*)v
										 = r->postprocess.GetRGBWorkingSpace ();
								}, NULL, NULL);
	}
	{
		TwBar *bar = TwNewBar ("antialiasing");
		TwDefine ("antialiasing label='Antialiasing' iconified=true");
		TwEnumVal samplesEV[] = {
			{  0, "disabled" },
			{  4, "4x" },
			{  8, "8x" },
			{  12, "12x" },
			{  16, "16x" },
		};
		TwType samplesType;
		samplesType = TwDefineEnum ("samples",
																samplesEV, 5);
		TwAddVarCB (bar, "Mode", samplesType,
								[&] (const void *v, void*) {
									r->SetAntialiasing (*(const unsigned int*)v);
								}, [&] (void *v, void*) {
									*(unsigned int*)v
										 = r->GetAntialiasing ();
								}, NULL, NULL);
		TwAddVarCB (bar, "Threshold", TW_TYPE_FLOAT,
								[&] (const void *v, void*) {
									r->postprocess.SetAntialiasingThreshold
										 (0.01f * *(const float*)v);
								}, [&] (void *v, void*) {
									*(float*)v
										 = 100.0f * r->postprocess.GetAntialiasingThreshold ();
								}, NULL, "step=0.1 min=0.0");
	}
	{
		TwBar *bar = TwNewBar ("glow");
		TwDefine ("glow label='Glow' iconified=true");

		TwAddVarCB (bar, "blur size", TW_TYPE_UINT32,
								[&] (const void *v, void*) {
									r->composition.GetGlow ().SetSize (*(const unsigned int*)v);
								}, [&] (void *v, void*) {
									*(unsigned int*)v = r->composition.GetGlow ().GetSize ();
								}, NULL, "min=0 max=256");
		TwAddVarCB (bar, "luminance threshold", TW_TYPE_FLOAT,
								[&] (const void *v, void*) {
									r->composition.SetLuminanceThreshold (*(const float*)v);
								}, [&] (void *v, void*) {
									*(float*)v = r->composition.GetLuminanceThreshold ();
								}, NULL, "step=0.01 min=0");
// TODO: NOT YET REIMPLEMENTED!
		TwAddVarCB (bar, "limit", TW_TYPE_FLOAT,
								[&] (const void *v, void*) {
									r->composition.GetGlow ().SetLimit (*(const float*)v);
								}, [&] (void *v, void*) {
									*(float*)v = r->composition.GetGlow ().GetLimit ();
								}, NULL, "step=0.01 min=0 visible=false");
		TwAddVarCB (bar, "exponent", TW_TYPE_FLOAT,
								[&] (const void *v, void*) {
									r->composition.GetGlow ().SetExponent (*(const float*)v);
								}, [&] (void *v, void*) {
									*(float*)v = r->composition.GetGlow ().GetExponent ();
								}, NULL, "step=0.01 min=0 visible=false");
	}
	{
		TwBar *bar = TwNewBar ("parameter");
		TwDefine ("parameter label='Material Parameters' iconified=true");

		{
			std::vector<TwEnumVal> paramNames;
			for (auto i = 0; i < r->GetNumParameters (); i++)
			{
				paramNames.push_back (TwEnumVal ());
				paramNames.back ().Value = i;
				paramNames.back ().Label = r->GetParameterName (i).c_str ();
			}
			TwType paramnameType;
			paramnameType = TwDefineEnum ("paramname", &paramNames[0],
																		paramNames.size ());
			TwAddVarRW (bar, "active", paramnameType,
									&active_parameter, "label='active parameter set'"); 
		}
		TwAddSeparator (bar, NULL, NULL);

		TwEnumVal specmodelEV[] = {
			{ 0, "None" },
			{  1, "Gaussian" },
			{  2, "Phong" },
			{  3, "Beckmann" }
		};
		TwType specmodelType;
		specmodelType = TwDefineEnum ("specmodel",
																	specmodelEV, 4);

		typedef struct matspec {
			 unsigned int model;
			 float param1;
			 float param2;
			 float fresnel_n;
			 float fresnel_k;
		} matspec_t;
		TwStructMember matspecDesc[] = {
			{ "Specular Model",
				specmodelType, offsetof (matspec_t, model), NULL},
			{ "Smoothness/Shininess",
				TW_TYPE_FLOAT, offsetof (matspec_t, param1),
				"step=0.01" },
			{ "Gauss Factor/Ignored",
				TW_TYPE_FLOAT, offsetof (matspec_t, param2),
				"step=0.01" },
			{ "Fresnel n",
				TW_TYPE_FLOAT, offsetof (matspec_t, fresnel_n),
				"step=0.01" },
			{ "Fresnel k",
				TW_TYPE_FLOAT, offsetof (matspec_t, fresnel_k),
				"step=0.01" }

		};
		TwType matspecType = TwDefineStruct ("matspectype", matspecDesc, 5,
																				 sizeof (matspec_t),
																				 NULL, NULL);
		TwAddVarCB (bar, "matspec", matspecType,
								[&] (const void *v, void*) {
									Parameter &p = r->GetParameters (active_parameter);
									const matspec_t *m = (const matspec_t*) v;
									p.specular.model = m->model;
									p.specular.param1 = m->param1;
									p.specular.param2 = m->param2;
									p.specular.fresnel.n = m->fresnel_n;
									p.specular.fresnel.k = m->fresnel_k;
									r->UpdateParameters (active_parameter);
								}, [&] (void *v, void*) {
									Parameter &p = r->GetParameters (active_parameter);
									matspec_t *m = (matspec_t*) v;
									m->model = p.specular.model;
									m->param1 = p.specular.param1;
									m->param2 = p.specular.param2;
									m->fresnel_n = p.specular.fresnel.n;
									m->fresnel_k = p.specular.fresnel.k;
								}, NULL, "label='Specular Parameters'");

		typedef struct matrefl {
			 float factor;
			 float fresnel_n;
			 float fresnel_k;
		} matrefl_t;
		TwStructMember matreflDesc[] = {
			{ "Factor",
				TW_TYPE_FLOAT, offsetof (matrefl_t, factor),
				"step=0.01 min=0.0 max=1.0" },
			{ "Fresnel n",
				TW_TYPE_FLOAT, offsetof (matrefl_t, fresnel_n),
				"step=0.01" },
			{ "Fresnel k",
				TW_TYPE_FLOAT, offsetof (matrefl_t, fresnel_k),
				"step=0.01" }

		};
		TwType matreflType = TwDefineStruct ("matrefltype", matreflDesc, 3,
																				 sizeof (matrefl_t),
																				 NULL, NULL);
		TwAddVarCB (bar, "matrefl", matreflType,
								[&] (const void *v, void*) {
									Parameter &p = r->GetParameters (active_parameter);
									const matrefl_t *m = (const matrefl_t*) v;
									p.reflection.factor = m->factor;
									p.reflection.fresnel.n = m->fresnel_n;
									p.reflection.fresnel.k = m->fresnel_k;
									r->UpdateParameters (active_parameter);
								}, [&] (void *v, void*) {
									Parameter &p = r->GetParameters (active_parameter);
									matrefl_t *m = (matrefl_t*) v;
									m->factor = p.reflection.factor;
									m->fresnel_n = p.reflection.fresnel.n;
									m->fresnel_k = p.reflection.fresnel.k;
								}, NULL, "label='Reflection'");
	}
}
