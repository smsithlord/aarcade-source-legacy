#include "cbase.h"

#include "aa_globals.h"
#include "c_inputlistenerawesomiumbrowser.h"
#include "c_anarchymanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_InputListenerAwesomiumBrowser::C_InputListenerAwesomiumBrowser()
{
	DevMsg("InputListenerAwesomiumBrowser: Constructor\n");
}

C_InputListenerAwesomiumBrowser::~C_InputListenerAwesomiumBrowser()
{
	DevMsg("InputListenerAwesomiumBrowser: Destructor\n");
}

void C_InputListenerAwesomiumBrowser::OnMouseMove(float x, float y)
{
//	int iMouseX = x * 1280;
//	int iMouseY = y * 720;


	g_pAnarchyManager->GetAwesomiumBrowserManager()->GetSelectedAwesomiumBrowserInstance()->OnMouseMove(x, y);



	//bool bHudMode = false;
	//ITexture* pInputCanvasTexture = g_pAnarchyManager->GetInputManager()->GetInputCanvasTexture();
	//C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");

	//bool bHudMode = (pInputCanvasTexture == pHudBrowserInstance->GetTexture());
	/*
	// ( pInputCanvasTexture == 
	WebView* pWebView = ->GetWebView();//FindWebView(pWebTab);
	WebView* pHudWebView = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud")->GetWebView();//FindWebView(pWebTab);
	if (pWebView)
		pWebView->InjectMouseMove(iMouseX, iMouseY);

	// inject mouse movement into the HUD too, if its active.
	C_WebTab* pHudWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();
	if (pHudWebTab && pHudWebTab != pWebTab && g_pAnarchyManager->GetInputManager()->GetInputMode())
	{
		WebView* pHudWebView = FindWebView(pHudWebTab);
		if (pHudWebView)
			pHudWebView->InjectMouseMove(iMouseX, iMouseY);
	}
	//g_pAnarchyManager->GetSteamBrowserManager()->GetSelectedAwesomiumBrowserInstance()->OnMouseMove(x, y);
	*/
}

void C_InputListenerAwesomiumBrowser::OnMousePressed(vgui::MouseCode code)
{
	//g_pAnarchyManager->GetSteamBrowserManager()->GetSelectedAwesomiumBrowserInstance()->OnMousePressed(code);// OnMouseMove(x, y);
}

void C_InputListenerAwesomiumBrowser::OnMouseReleased(vgui::MouseCode code)
{
	//g_pAnarchyManager->GetSteamBrowserManager()->GetSelectedAwesomiumBrowserInstance()->OnMouseReleased(code);// OnMouseMove(x, y);
}