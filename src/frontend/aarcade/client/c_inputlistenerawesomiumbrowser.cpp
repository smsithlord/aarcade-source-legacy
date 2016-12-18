#include "cbase.h"

#include "aa_globals.h"
#include "c_inputlistenerawesomiumbrowser.h"
#include "c_anarchymanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

std::map<ButtonCode_t, int> C_InputListenerAwesomiumBrowser::s_sourceToAwesomiumButtonMap;

C_InputListenerAwesomiumBrowser::C_InputListenerAwesomiumBrowser()
{
	using namespace Awesomium::KeyCodes;
	DevMsg("InputListenerAwesomiumBrowser: Constructor\n");

	if (s_sourceToAwesomiumButtonMap.find(KEY_0) == s_sourceToAwesomiumButtonMap.end())
	{
		s_sourceToAwesomiumButtonMap[KEY_0] = AK_0;
		s_sourceToAwesomiumButtonMap[KEY_1] = AK_1;
		s_sourceToAwesomiumButtonMap[KEY_2] = AK_2;
		s_sourceToAwesomiumButtonMap[KEY_3] = AK_3;
		s_sourceToAwesomiumButtonMap[KEY_4] = AK_4;
		s_sourceToAwesomiumButtonMap[KEY_5] = AK_5;
		s_sourceToAwesomiumButtonMap[KEY_6] = AK_6;
		s_sourceToAwesomiumButtonMap[KEY_7] = AK_7;
		s_sourceToAwesomiumButtonMap[KEY_8] = AK_8;
		s_sourceToAwesomiumButtonMap[KEY_9] = AK_9;
		s_sourceToAwesomiumButtonMap[KEY_A] = AK_A;
		s_sourceToAwesomiumButtonMap[KEY_B] = AK_B;
		s_sourceToAwesomiumButtonMap[KEY_C] = AK_C;
		s_sourceToAwesomiumButtonMap[KEY_D] = AK_D;
		s_sourceToAwesomiumButtonMap[KEY_E] = AK_E;
		s_sourceToAwesomiumButtonMap[KEY_F] = AK_F;
		s_sourceToAwesomiumButtonMap[KEY_G] = AK_G;
		s_sourceToAwesomiumButtonMap[KEY_H] = AK_H;
		s_sourceToAwesomiumButtonMap[KEY_I] = AK_I;
		s_sourceToAwesomiumButtonMap[KEY_J] = AK_J;
		s_sourceToAwesomiumButtonMap[KEY_K] = AK_K;
		s_sourceToAwesomiumButtonMap[KEY_L] = AK_L;
		s_sourceToAwesomiumButtonMap[KEY_M] = AK_M;
		s_sourceToAwesomiumButtonMap[KEY_N] = AK_N;
		s_sourceToAwesomiumButtonMap[KEY_O] = AK_O;
		s_sourceToAwesomiumButtonMap[KEY_P] = AK_P;
		s_sourceToAwesomiumButtonMap[KEY_Q] = AK_Q;
		s_sourceToAwesomiumButtonMap[KEY_R] = AK_R;
		s_sourceToAwesomiumButtonMap[KEY_S] = AK_S;
		s_sourceToAwesomiumButtonMap[KEY_T] = AK_T;
		s_sourceToAwesomiumButtonMap[KEY_U] = AK_U;
		s_sourceToAwesomiumButtonMap[KEY_V] = AK_V;
		s_sourceToAwesomiumButtonMap[KEY_W] = AK_W;
		s_sourceToAwesomiumButtonMap[KEY_X] = AK_X;
		s_sourceToAwesomiumButtonMap[KEY_Y] = AK_Y;
		s_sourceToAwesomiumButtonMap[KEY_Z] = AK_Z;
		s_sourceToAwesomiumButtonMap[KEY_PAD_0] = AK_NUMPAD0;
		s_sourceToAwesomiumButtonMap[KEY_PAD_1] = AK_NUMPAD1;
		s_sourceToAwesomiumButtonMap[KEY_PAD_2] = AK_NUMPAD2;
		s_sourceToAwesomiumButtonMap[KEY_PAD_3] = AK_NUMPAD3;
		s_sourceToAwesomiumButtonMap[KEY_PAD_4] = AK_NUMPAD4;
		s_sourceToAwesomiumButtonMap[KEY_PAD_5] = AK_NUMPAD5;
		s_sourceToAwesomiumButtonMap[KEY_PAD_6] = AK_NUMPAD6;
		s_sourceToAwesomiumButtonMap[KEY_PAD_7] = AK_NUMPAD7;
		s_sourceToAwesomiumButtonMap[KEY_PAD_8] = AK_NUMPAD8;
		s_sourceToAwesomiumButtonMap[KEY_PAD_9] = AK_NUMPAD9;
		s_sourceToAwesomiumButtonMap[KEY_PAD_DIVIDE] = AK_DIVIDE;
		s_sourceToAwesomiumButtonMap[KEY_PAD_MULTIPLY] = AK_MULTIPLY;
		s_sourceToAwesomiumButtonMap[KEY_PAD_MINUS] = AK_SUBTRACT;	// AK_OEM_MINUS;
		s_sourceToAwesomiumButtonMap[KEY_PAD_PLUS] = AK_ADD;	// AK_OEM_PLUS;
		s_sourceToAwesomiumButtonMap[KEY_PAD_ENTER] = AK_RETURN;
		s_sourceToAwesomiumButtonMap[KEY_PAD_DECIMAL] = AK_DECIMAL;	// AK_OEM_PERIOD;
		s_sourceToAwesomiumButtonMap[KEY_LBRACKET] = AK_OEM_4;	// AK_OEM_102;
		s_sourceToAwesomiumButtonMap[KEY_RBRACKET] = AK_OEM_6;	// AK_OEM_102;
		s_sourceToAwesomiumButtonMap[KEY_SEMICOLON] = AK_OEM_1;
		s_sourceToAwesomiumButtonMap[KEY_APOSTROPHE] = AK_OEM_7;
		s_sourceToAwesomiumButtonMap[KEY_BACKQUOTE] = AK_OEM_3;
		s_sourceToAwesomiumButtonMap[KEY_COMMA] = AK_OEM_COMMA;
		s_sourceToAwesomiumButtonMap[KEY_PERIOD] = AK_OEM_PERIOD;
		s_sourceToAwesomiumButtonMap[KEY_SLASH] = AK_OEM_2;
		s_sourceToAwesomiumButtonMap[KEY_BACKSLASH] = AK_OEM_5;
		s_sourceToAwesomiumButtonMap[KEY_MINUS] = AK_OEM_MINUS;
		s_sourceToAwesomiumButtonMap[KEY_EQUAL] = AK_OEM_PLUS;	// SHIFTED
		s_sourceToAwesomiumButtonMap[KEY_ENTER] = AK_RETURN;
		s_sourceToAwesomiumButtonMap[KEY_SPACE] = AK_SPACE;
		s_sourceToAwesomiumButtonMap[KEY_BACKSPACE] = AK_BACK;
		s_sourceToAwesomiumButtonMap[KEY_TAB] = AK_TAB;
		s_sourceToAwesomiumButtonMap[KEY_CAPSLOCK] = AK_CAPITAL;
		s_sourceToAwesomiumButtonMap[KEY_NUMLOCK] = AK_NUMLOCK;
		s_sourceToAwesomiumButtonMap[KEY_ESCAPE] = AK_ESCAPE;
		s_sourceToAwesomiumButtonMap[KEY_SCROLLLOCK] = AK_SCROLL;
		s_sourceToAwesomiumButtonMap[KEY_INSERT] = AK_INSERT;
		s_sourceToAwesomiumButtonMap[KEY_DELETE] = AK_DELETE;
		s_sourceToAwesomiumButtonMap[KEY_HOME] = AK_HOME;
		s_sourceToAwesomiumButtonMap[KEY_END] = AK_END;
		s_sourceToAwesomiumButtonMap[KEY_PAGEUP] = AK_PRIOR;
		s_sourceToAwesomiumButtonMap[KEY_PAGEDOWN] = AK_NEXT;
		s_sourceToAwesomiumButtonMap[KEY_BREAK] = AK_PAUSE;
		s_sourceToAwesomiumButtonMap[KEY_LSHIFT] = AK_LSHIFT;
		s_sourceToAwesomiumButtonMap[KEY_RSHIFT] = AK_RSHIFT;
		s_sourceToAwesomiumButtonMap[KEY_LALT] = AK_MENU;
		s_sourceToAwesomiumButtonMap[KEY_RALT] = AK_MENU;
		s_sourceToAwesomiumButtonMap[KEY_LCONTROL] = AK_CONTROL;
		s_sourceToAwesomiumButtonMap[KEY_RCONTROL] = AK_CONTROL;
		s_sourceToAwesomiumButtonMap[KEY_LWIN] = AK_LWIN;
		s_sourceToAwesomiumButtonMap[KEY_RWIN] = AK_RWIN;
		s_sourceToAwesomiumButtonMap[KEY_APP] = AK_APPS;
		s_sourceToAwesomiumButtonMap[KEY_UP] = AK_UP;
		s_sourceToAwesomiumButtonMap[KEY_LEFT] = AK_LEFT;
		s_sourceToAwesomiumButtonMap[KEY_DOWN] = AK_DOWN;
		s_sourceToAwesomiumButtonMap[KEY_RIGHT] = AK_RIGHT;
		s_sourceToAwesomiumButtonMap[KEY_F1] = AK_F1;
		s_sourceToAwesomiumButtonMap[KEY_F2] = AK_F2;
		s_sourceToAwesomiumButtonMap[KEY_F3] = AK_F3;
		s_sourceToAwesomiumButtonMap[KEY_F4] = AK_F4;
		s_sourceToAwesomiumButtonMap[KEY_F5] = AK_F5;
		s_sourceToAwesomiumButtonMap[KEY_F6] = AK_F6;
		s_sourceToAwesomiumButtonMap[KEY_F7] = AK_F7;
		s_sourceToAwesomiumButtonMap[KEY_F8] = AK_F8;
		s_sourceToAwesomiumButtonMap[KEY_F9] = AK_F9;
		s_sourceToAwesomiumButtonMap[KEY_CAPSLOCKTOGGLE] = AK_CAPITAL;
		s_sourceToAwesomiumButtonMap[KEY_NUMLOCKTOGGLE] = AK_NUMLOCK;
		s_sourceToAwesomiumButtonMap[KEY_SCROLLLOCKTOGGLE] = AK_SCROLL;
	}
}

C_InputListenerAwesomiumBrowser::~C_InputListenerAwesomiumBrowser()
{
	DevMsg("InputListenerAwesomiumBrowser: Destructor\n");
}

// this version accepts a target, instead of always using the "selected" instance
void C_InputListenerAwesomiumBrowser::OnMouseMove(float x, float y, C_AwesomiumBrowserInstance* pBrowserInstance)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	pBrowserInstance->OnMouseMove(x, y);
}

void C_InputListenerAwesomiumBrowser::OnMouseMove(float x, float y)
{
	if (g_pAnarchyManager->IsPaused())
		return;

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
	if (g_pAnarchyManager->IsPaused())
		return;

//	g_pAnarchyManager->GetAwesomiumBrowserManager()->GetSelectedAwesomiumBrowserInstance()->OnMousePressed(code);
	g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud")->OnMousePressed(code);
}

void C_InputListenerAwesomiumBrowser::OnMouseReleased(vgui::MouseCode code)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	//g_pAnarchyManager->GetSteamBrowserManager()->GetSelectedAwesomiumBrowserInstance()->OnMouseReleased(code);// OnMouseMove(x, y);
	g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud")->OnMouseReleased(code);
}

void C_InputListenerAwesomiumBrowser::OnMouseWheeled(int delta)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	if (pHudBrowserInstance->HasFocus())//&& g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() != pHudBrowserInstance)
		pHudBrowserInstance->OnMouseWheeled(delta);
	else if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() != pHudBrowserInstance)
	{
		C_AwesomiumBrowserInstance* pOtherAwesomiumBrowserInstance = dynamic_cast<C_AwesomiumBrowserInstance*>(g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance());
		if (pOtherAwesomiumBrowserInstance)
		{
			if (!pOtherAwesomiumBrowserInstance->HasFocus())
				pOtherAwesomiumBrowserInstance->Focus();	// make sure we are focused

			pOtherAwesomiumBrowserInstance->OnMouseWheeled(delta);
		}
	}
}

void C_InputListenerAwesomiumBrowser::OnKeyCodePressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	if (pHudBrowserInstance->HasFocus() )//&& g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() != pHudBrowserInstance)
		pHudBrowserInstance->OnKeyPressed(code, bShiftState, bCtrlState, bAltState, bWinState, bAutorepeatState);
	else if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() != pHudBrowserInstance)
	{
		C_AwesomiumBrowserInstance* pOtherAwesomiumBrowserInstance = dynamic_cast<C_AwesomiumBrowserInstance*>(g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance());
		if (pOtherAwesomiumBrowserInstance)
		{
			if (!pOtherAwesomiumBrowserInstance->HasFocus())
				pOtherAwesomiumBrowserInstance->Focus();	// make sure we are focused

			pOtherAwesomiumBrowserInstance->OnKeyPressed(code, bShiftState, bCtrlState, bAltState, bWinState, bAutorepeatState);
		}
	}
//	else
//	{
		//g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance()->OnKeyPressed(code, bShiftState, bCtrlState, bAltState);
	//}
	//g_pAnarchyManager->GetAwesomiumBrowserManager()->GetSelectedAwesomiumBrowserInstance()->OnKeyPressed(code);
}

void C_InputListenerAwesomiumBrowser::OnKeyCodeReleased(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	if (pHudBrowserInstance->HasFocus() )//&& g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() != pHudBrowserInstance)
		pHudBrowserInstance->OnKeyReleased(code, bShiftState, bCtrlState, bAltState, bWinState, bAutorepeatState);
	else if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() != pHudBrowserInstance)
	{
		C_AwesomiumBrowserInstance* pOtherAwesomiumBrowserInstance = dynamic_cast<C_AwesomiumBrowserInstance*>(g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance());
		if (pOtherAwesomiumBrowserInstance)
		{
			//if (!pOtherAwesomiumBrowserInstance->HasFocus())
				//pOtherAwesomiumBrowserInstance->Focus();	// make sure we are focused

			pOtherAwesomiumBrowserInstance->OnKeyReleased(code, bShiftState, bCtrlState, bAltState, bWinState, bAutorepeatState);
		}
	}
	//g_pAnarchyManager->GetAwesomiumBrowserManager()->GetSelectedAwesomiumBrowserInstance()->OnKeyReleased(code);

	//C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
//	pHudBrowserInstance->OnKeyReleased(code);
}

int C_InputListenerAwesomiumBrowser::ConvertSourceButtonToAwesomiumButton(ButtonCode_t sourceButton)
{
	auto it = s_sourceToAwesomiumButtonMap.find(sourceButton);
	if (it != s_sourceToAwesomiumButtonMap.end())
		return it->second;
	else
		return Awesomium::KeyCodes::AK_UNKNOWN;
}