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
#ifndef INTERFACE_H
#define INTERFACE_H

#include <common.h>
#include "font/all.h"

class Renderer;

class Interface
{
public:
	 Interface (Renderer *parent);
	 ~Interface (void);
	 bool Init (void);
	 void Frame (float timefactor);
	 void OnKeyUp (int key);
	 void OnKeyDown (int key);

	 void MainMenu (int what);
	 void AddLight (int what);
	 void AddShadow (int what);
	 void EditLights (int what);
	 void EditShadows (int what);
	 void EditGlow (int what);
	 void ToggleRendermode (int what);
	 void ToggleCompositionmode (int what);
	 void ToggleSoftShadow (int what);
	 void EditToneMapping (int what);
	 void EditToneMappingAvgLum (int what);
	 void EditGlowSize (int what);
	 void EditAntialiasing (int what);
	 void Exit (int what);

	 void SelectShadow (int what);
	 void EditShadowPosition (int what);
	 void RemoveShadow (int what);

	 void RemoveLight (int what);
	 void RandomizeLights (int what);
	 void SelectLight (int what);
	 void EditLightPosition (int what);
	 void MoveShadowX (int what);
	 void MoveShadowY (int what);
	 void MoveShadowZ (int what);
	 void EditShadowAlpha (int what);
	 void MoveLightX (int what);
	 void MoveLightY (int what);
	 void MoveLightZ (int what);
	 void EditLightDirection (int what);
	 void MoveLightDirX (int what);
	 void MoveLightDirY (int what);
	 void MoveLightDirZ (int what);
	 void EditDiffuseColor (int what);
	 void EditAttenuation (int what);
	 void EditSpecularColor (int what);
	 void EditDiffuseR (int what);
	 void EditDiffuseG (int what);
	 void EditDiffuseB (int what);
	 void EditSpecularR (int what);
	 void EditSpecularG (int what);
	 void EditSpecularB (int what);
	 void EditSpotAngle (int what);
	 void EditSpotPenumbraAngle (int what);
	 void EditSpotExponent (int what);
	 void EditConstantAttenuation (int what);
	 void EditLinearAttenuation (int what);
	 void EditQuadraticAttenuation (int what);
	 void EditMaxDistance (int what);
	 void EditShininess (int what);
	 void EditImageKey (int what);
	 void EditWhiteThreshold (int what);
	 void EditRGBWorkingSpace (int what);
	 void EditToneMappingMode (int what);
	 void EditToneMappingSigma (int what);
	 void EditToneMappingN (int what);
	 void EditToneMappingAvgLumConstant (int what);
	 void EditToneMappingAvgLumLinear (int what);
	 void EditToneMappingAvgLumDelta (int what);
	 void EditToneMappingAvgLumLod (int what);
	 void EditLuminanceThreshold (int what);

	 void PrintRendermode (void);
	 void PrintCompositionmode (void);
	 void PrintActiveLight (void);
	 void PrintActiveShadow (void);
	 void PrintSoftShadow (void);
	 void PrintShadowX (void);
	 void PrintShadowY (void);
	 void PrintShadowZ (void);
	 void PrintShadowAlpha (void);
	 void PrintLightX (void);
	 void PrintLightY (void);
	 void PrintLightZ (void);
	 void PrintLightDirX (void);
	 void PrintLightDirY (void);
	 void PrintLightDirZ (void);
	 void PrintDiffuseR (void);
	 void PrintDiffuseG (void);
	 void PrintDiffuseB (void);
	 void PrintSpecularR (void);
	 void PrintSpecularG (void);
	 void PrintSpecularB (void);
	 void PrintSpotAngle (void);
	 void PrintSpotPenumbraAngle (void);
	 void PrintSpotExponent (void);
	 void PrintConstantAttenuation (void);
	 void PrintLinearAttenuation (void);
	 void PrintQuadraticAttenuation (void);
	 void PrintMaxDistance (void);
	 void PrintShininess (void);
	 void PrintImageKey (void);
	 void PrintWhiteThreshold (void);
	 void PrintRGBWorkingSpace (void);
	 void PrintToneMappingMode (void);
	 void PrintToneMappingSigma (void);
	 void PrintToneMappingN (void);
	 void PrintToneMappingAvgLumConstant (void);
	 void PrintToneMappingAvgLumLinear (void);
	 void PrintToneMappingAvgLumDelta (void);
	 void PrintToneMappingAvgLumLod (void);
	 void PrintGlowSize (void);
	 void PrintLuminanceThreshold (void);
	 void PrintAntialiasing (void);
private:
	 gl::Freetype freetype;
	 gl::Font font;

	 bool showInterface;
	 unsigned int menu, submenu;

	 int active_light, active_shadow;

	 float timefactor;

	 Renderer *renderer;
};

#endif /* !defined INTERFACE_H */
