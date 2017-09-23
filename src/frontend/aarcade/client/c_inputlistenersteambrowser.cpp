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
	//DevMsg("InputListenerSteamBrowser: Destructor\n");
}

void C_InputListenerSteamBrowser::OnMouseWheeled(int delta)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	//C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	//if (pHudBrowserInstance->HasFocus())//&& g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() != pHudBrowserInstance)
	//	pHudBrowserInstance->OnMouseWheeled(delta);
	//else if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() != pHudBrowserInstance)
	//{
	C_SteamBrowserInstance* pBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance());
	if ( pBrowserInstance )
	{
		//C_AwesomiumBrowserInstance* pOtherAwesomiumBrowserInstance = dynamic_cast<C_AwesomiumBrowserInstance*>(g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance());
		//if (pOtherAwesomiumBrowserInstance)
		//{
		//if (!pBrowserInstance->HasFocus())
		//	pBrowserInstance->Focus();	// make sure we are focused

			pBrowserInstance->OnMouseWheeled(delta);
		//}
	}
}

void C_InputListenerSteamBrowser::OnMouseMove(float x, float y)
{
	if (g_pAnarchyManager->IsPaused())
		return;

//	g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance()->GetInputListener()->OnMouseMove(x, y);
	//DevMsg("yyyOnMouseMove: %f %f\n", x, y);

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->GetSelectedSteamBrowserInstance();
	if (pBrowserInstance)
		pBrowserInstance->OnMouseMove(x, y);
}

void C_InputListenerSteamBrowser::OnMousePressed(vgui::MouseCode code)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->GetSelectedSteamBrowserInstance();
	if (pBrowserInstance)
		pBrowserInstance->OnMousePressed(code);// OnMouseMove(x, y);
}

void C_InputListenerSteamBrowser::OnMouseReleased(vgui::MouseCode code)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->GetSelectedSteamBrowserInstance();
	if (pBrowserInstance)
		pBrowserInstance->OnMouseReleased(code);// OnMouseMove(x, y);
}

void C_InputListenerSteamBrowser::OnKeyCodePressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->GetFocusedSteamBrowserInstance();
	if (pBrowserInstance)
		pBrowserInstance->OnKeyCodePressed(code, bShiftState, bCtrlState, bAltState, bWinState, bAutorepeatState);
}

void C_InputListenerSteamBrowser::OnKeyCodeReleased(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->GetFocusedSteamBrowserInstance();
	if (pBrowserInstance)
		pBrowserInstance->OnKeyCodeReleased(code, bShiftState, bCtrlState, bAltState, bWinState, bAutorepeatState);
}