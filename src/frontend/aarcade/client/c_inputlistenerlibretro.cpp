#include "cbase.h"

#include "aa_globals.h"
#include "c_inputlistenerlibretro.h"
#include "c_anarchymanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_InputListenerLibretro::C_InputListenerLibretro()
{
	DevMsg("InputListenerLibretro: Constructor\n");
}

C_InputListenerLibretro::~C_InputListenerLibretro()
{
	DevMsg("InputListenerLibretro: Destructor\n");
}

/*
void C_InputListenerLibretro::OnMouseMove(float x, float y)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	DevMsg("xxxOnMouseMove: %f %f\n", x, y);

	// end of the road for libretro.  gotta look into how libretro likes to recieve its input.

	//C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");

	//g_pAnarchyManager->GetLibretroManager()->GetInputListener()->OnMouseMove(x, y);
	//if (pHudBrowserInstance && pHud)
		//pHudBrowserInstance->OnMouseMove(x, y);
}
*/