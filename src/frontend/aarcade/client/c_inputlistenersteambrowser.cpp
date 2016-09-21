#include "cbase.h"

#include "aa_globals.h"
#include "c_inputlistenersteambrowser.h"
#include "c_anarchymanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_InputListenerSteamBrowser::C_InputListenerSteamBrowser()
{
	DevMsg("InputListenerSteamBrowser: Constructor\n");
}

C_InputListenerSteamBrowser::~C_InputListenerSteamBrowser()
{
	DevMsg("InputListenerSteamBrowser: Destructor\n");
}

void C_InputListenerSteamBrowser::OnMouseMove(float x, float y)
{
//	g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance()->GetInputListener()->OnMouseMove(x, y);
	//DevMsg("yyyOnMouseMove: %f %f\n", x, y);
	g_pAnarchyManager->GetSteamBrowserManager()->GetSelectedSteamBrowserInstance()->OnMouseMove(x, y);
}

void C_InputListenerSteamBrowser::OnMousePressed(vgui::MouseCode code)
{
	g_pAnarchyManager->GetSteamBrowserManager()->GetSelectedSteamBrowserInstance()->OnMousePressed(code);// OnMouseMove(x, y);
}

void C_InputListenerSteamBrowser::OnMouseReleased(vgui::MouseCode code)
{
	g_pAnarchyManager->GetSteamBrowserManager()->GetSelectedSteamBrowserInstance()->OnMouseReleased(code);// OnMouseMove(x, y);
}

void C_InputListenerSteamBrowser::OnKeyCodePressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState)
{
	g_pAnarchyManager->GetSteamBrowserManager()->GetFocusedSteamBrowserInstance()->OnKeyCodePressed(code, bShiftState, bCtrlState, bAltState);
}

void C_InputListenerSteamBrowser::OnKeyCodeReleased(vgui::KeyCode code)
{
	g_pAnarchyManager->GetSteamBrowserManager()->GetFocusedSteamBrowserInstance()->OnKeyCodeReleased(code);
}