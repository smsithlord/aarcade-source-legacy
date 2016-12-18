#include "cbase.h"

#include "c_inputmanager.h"
#include "c_anarchymanager.h"
#include "ienginevgui.h"
#include "c_inputslate.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_InputManager::C_InputManager()
{
	DevMsg("InputManager: Constructor\n");
	m_bInputMode = false;
	m_bForcedInputMode = false;
	m_bWasForcedInputMode = false;
	m_bFullscreenMode = false;
	m_bOverlayMode = false;
	m_bMainMenuMode = false;
	//m_pInputListener = null;
	//m_pInputCanvasTexture = null;
	m_pEmbeddedInstance = null;

	m_sourceKeyEnumMap["KEY_0"] = KEY_0;
	m_sourceKeyEnumMap["KEY_1"] = KEY_1;
	m_sourceKeyEnumMap["KEY_2"] = KEY_2;
	m_sourceKeyEnumMap["KEY_3"] = KEY_3;
	m_sourceKeyEnumMap["KEY_4"] = KEY_4;
	m_sourceKeyEnumMap["KEY_5"] = KEY_5;
	m_sourceKeyEnumMap["KEY_6"] = KEY_6;
	m_sourceKeyEnumMap["KEY_7"] = KEY_7;
	m_sourceKeyEnumMap["KEY_8"] = KEY_8;
	m_sourceKeyEnumMap["KEY_9"] = KEY_9;
	m_sourceKeyEnumMap["KEY_A"] = KEY_A;
	m_sourceKeyEnumMap["KEY_B"] = KEY_B;
	m_sourceKeyEnumMap["KEY_C"] = KEY_C;
	m_sourceKeyEnumMap["KEY_D"] = KEY_D;
	m_sourceKeyEnumMap["KEY_E"] = KEY_E;
	m_sourceKeyEnumMap["KEY_F"] = KEY_F;
	m_sourceKeyEnumMap["KEY_G"] = KEY_G;
	m_sourceKeyEnumMap["KEY_H"] = KEY_H;
	m_sourceKeyEnumMap["KEY_I"] = KEY_I;
	m_sourceKeyEnumMap["KEY_J"] = KEY_J;
	m_sourceKeyEnumMap["KEY_K"] = KEY_K;
	m_sourceKeyEnumMap["KEY_L"] = KEY_L;
	m_sourceKeyEnumMap["KEY_M"] = KEY_M;
	m_sourceKeyEnumMap["KEY_N"] = KEY_N;
	m_sourceKeyEnumMap["KEY_O"] = KEY_O;
	m_sourceKeyEnumMap["KEY_P"] = KEY_P;
	m_sourceKeyEnumMap["KEY_Q"] = KEY_Q;
	m_sourceKeyEnumMap["KEY_R"] = KEY_R;
	m_sourceKeyEnumMap["KEY_S"] = KEY_S;
	m_sourceKeyEnumMap["KEY_T"] = KEY_T;
	m_sourceKeyEnumMap["KEY_U"] = KEY_U;
	m_sourceKeyEnumMap["KEY_V"] = KEY_V;
	m_sourceKeyEnumMap["KEY_W"] = KEY_W;
	m_sourceKeyEnumMap["KEY_X"] = KEY_X;
	m_sourceKeyEnumMap["KEY_Y"] = KEY_Y;
	m_sourceKeyEnumMap["KEY_Z"] = KEY_Z;
	m_sourceKeyEnumMap["KEY_PAD_0"] = KEY_PAD_0;
	m_sourceKeyEnumMap["KEY_PAD_1"] = KEY_PAD_1;
	m_sourceKeyEnumMap["KEY_PAD_2"] = KEY_PAD_2;
	m_sourceKeyEnumMap["KEY_PAD_3"] = KEY_PAD_3;
	m_sourceKeyEnumMap["KEY_PAD_4"] = KEY_PAD_4;
	m_sourceKeyEnumMap["KEY_PAD_5"] = KEY_PAD_5;
	m_sourceKeyEnumMap["KEY_PAD_6"] = KEY_PAD_6;
	m_sourceKeyEnumMap["KEY_PAD_7"] = KEY_PAD_7;
	m_sourceKeyEnumMap["KEY_PAD_8"] = KEY_PAD_8;
	m_sourceKeyEnumMap["KEY_PAD_9"] = KEY_PAD_9;
	m_sourceKeyEnumMap["KEY_PAD_DIVIDE"] = KEY_PAD_DIVIDE;
	m_sourceKeyEnumMap["KEY_PAD_MULTIPLY"] = KEY_PAD_MULTIPLY;
	m_sourceKeyEnumMap["KEY_PAD_MINUS"] = KEY_PAD_MINUS;
	m_sourceKeyEnumMap["KEY_PAD_PLUS"] = KEY_PAD_PLUS;
	m_sourceKeyEnumMap["KEY_PAD_ENTER"] = KEY_PAD_ENTER;
	m_sourceKeyEnumMap["KEY_PAD_DECIMAL"] = KEY_PAD_DECIMAL;
	m_sourceKeyEnumMap["KEY_LBRACKET"] = KEY_LBRACKET;
	m_sourceKeyEnumMap["KEY_RBRACKET"] = KEY_RBRACKET;
	m_sourceKeyEnumMap["KEY_SEMICOLON"] = KEY_SEMICOLON;
	m_sourceKeyEnumMap["KEY_APOSTROPHE"] = KEY_APOSTROPHE;
	m_sourceKeyEnumMap["KEY_BACKQUOTE"] = KEY_BACKQUOTE;
	m_sourceKeyEnumMap["KEY_COMMA"] = KEY_COMMA;
	m_sourceKeyEnumMap["KEY_PERIOD"] = KEY_PERIOD;
	m_sourceKeyEnumMap["KEY_SLASH"] = KEY_SLASH;
	m_sourceKeyEnumMap["KEY_BACKSLASH"] = KEY_BACKSLASH;
	m_sourceKeyEnumMap["KEY_MINUS"] = KEY_MINUS;
	m_sourceKeyEnumMap["KEY_EQUAL"] = KEY_EQUAL;
	m_sourceKeyEnumMap["KEY_ENTER"] = KEY_ENTER;
	m_sourceKeyEnumMap["KEY_SPACE"] = KEY_SPACE;
	m_sourceKeyEnumMap["KEY_BACKSPACE"] = KEY_BACKSPACE;
	m_sourceKeyEnumMap["KEY_TAB"] = KEY_TAB;
	m_sourceKeyEnumMap["KEY_CAPSLOCK"] = KEY_CAPSLOCK;
	m_sourceKeyEnumMap["KEY_NUMLOCK"] = KEY_NUMLOCK;
	m_sourceKeyEnumMap["KEY_ESCAPE"] = KEY_ESCAPE;
	m_sourceKeyEnumMap["KEY_SCROLLLOCK"] = KEY_SCROLLLOCK;
	m_sourceKeyEnumMap["KEY_INSERT"] = KEY_INSERT;
	m_sourceKeyEnumMap["KEY_DELETE"] = KEY_DELETE;
	m_sourceKeyEnumMap["KEY_HOME"] = KEY_HOME;
	m_sourceKeyEnumMap["KEY_END"] = KEY_END;
	m_sourceKeyEnumMap["KEY_PAGEUP"] = KEY_PAGEUP;
	m_sourceKeyEnumMap["KEY_PAGEDOWN"] = KEY_PAGEDOWN;
	m_sourceKeyEnumMap["KEY_BREAK"] = KEY_BREAK;
	m_sourceKeyEnumMap["KEY_LSHIFT"] = KEY_LSHIFT;
	m_sourceKeyEnumMap["KEY_RSHIFT"] = KEY_RSHIFT;
	m_sourceKeyEnumMap["KEY_LALT"] = KEY_LALT;
	m_sourceKeyEnumMap["KEY_RALT"] = KEY_RALT;
	m_sourceKeyEnumMap["KEY_LCONTROL"] = KEY_LCONTROL;
	m_sourceKeyEnumMap["KEY_RCONTROL"] = KEY_RCONTROL;
	m_sourceKeyEnumMap["KEY_LWIN"] = KEY_LWIN;
	m_sourceKeyEnumMap["KEY_RWIN"] = KEY_RWIN;
	m_sourceKeyEnumMap["KEY_APP"] = KEY_APP;
	m_sourceKeyEnumMap["KEY_UP"] = KEY_UP;
	m_sourceKeyEnumMap["KEY_LEFT"] = KEY_LEFT;
	m_sourceKeyEnumMap["KEY_DOWN"] = KEY_DOWN;
	m_sourceKeyEnumMap["KEY_RIGHT"] = KEY_RIGHT;
	m_sourceKeyEnumMap["KEY_F1"] = KEY_F1;
	m_sourceKeyEnumMap["KEY_F2"] = KEY_F2;
	m_sourceKeyEnumMap["KEY_F3"] = KEY_F3;
	m_sourceKeyEnumMap["KEY_F4"] = KEY_F4;
	m_sourceKeyEnumMap["KEY_F5"] = KEY_F5;
	m_sourceKeyEnumMap["KEY_F6"] = KEY_F6;
	m_sourceKeyEnumMap["KEY_F7"] = KEY_F7;
	m_sourceKeyEnumMap["KEY_F8"] = KEY_F8;
	m_sourceKeyEnumMap["KEY_F9"] = KEY_F9;
	m_sourceKeyEnumMap["KEY_F10"] = KEY_F10;
	m_sourceKeyEnumMap["KEY_F11"] = KEY_F11;
	m_sourceKeyEnumMap["KEY_F12"] = KEY_F12;
	m_sourceKeyEnumMap["KEY_CAPSLOCKTOGGLE"] = KEY_CAPSLOCKTOGGLE;
	m_sourceKeyEnumMap["KEY_NUMLOCKTOGGLE"] = KEY_NUMLOCKTOGGLE;
	m_sourceKeyEnumMap["KEY_SCROLLLOCKTOGGLE"] = KEY_SCROLLLOCKTOGGLE;
	m_sourceKeyEnumMap["MOUSE_LEFT"] = MOUSE_LEFT;
	m_sourceKeyEnumMap["MOUSE_RIGHT"] = MOUSE_RIGHT;
	m_sourceKeyEnumMap["MOUSE_MIDDLE"] = MOUSE_MIDDLE;
	m_sourceKeyEnumMap["MOUSE_4"] = MOUSE_4;
	m_sourceKeyEnumMap["MOUSE_5"] = MOUSE_5;
	m_sourceKeyEnumMap["MOUSE_WHEEL_UP"] = MOUSE_WHEEL_UP;
	m_sourceKeyEnumMap["MOUSE_WHEEL_DOWN"] = MOUSE_WHEEL_DOWN;
	m_sourceKeyEnumMap["KEY_XBUTTON_UP"] = KEY_XBUTTON_UP;
	m_sourceKeyEnumMap["KEY_XBUTTON_RIGHT"] = KEY_XBUTTON_RIGHT;
	m_sourceKeyEnumMap["KEY_XBUTTON_DOWN"] = KEY_XBUTTON_DOWN;
	m_sourceKeyEnumMap["KEY_XBUTTON_LEFT"] = KEY_XBUTTON_LEFT;
	m_sourceKeyEnumMap["KEY_XBUTTON_A"] = KEY_XBUTTON_A;
	m_sourceKeyEnumMap["KEY_XBUTTON_B"] = KEY_XBUTTON_B;
	m_sourceKeyEnumMap["KEY_XBUTTON_X"] = KEY_XBUTTON_X;
	m_sourceKeyEnumMap["KEY_XBUTTON_Y"] = KEY_XBUTTON_Y;
	m_sourceKeyEnumMap["KEY_XBUTTON_LEFT_SHOULDER"] = KEY_XBUTTON_LEFT_SHOULDER;
	m_sourceKeyEnumMap["KEY_XBUTTON_RIGHT_SHOULDER"] = KEY_XBUTTON_RIGHT_SHOULDER;
	m_sourceKeyEnumMap["KEY_XBUTTON_BACK"] = KEY_XBUTTON_BACK;
	m_sourceKeyEnumMap["KEY_XBUTTON_START"] = KEY_XBUTTON_START;
	m_sourceKeyEnumMap["KEY_XBUTTON_STICK1"] = KEY_XBUTTON_STICK1;
	m_sourceKeyEnumMap["KEY_XBUTTON_STICK2"] = KEY_XBUTTON_STICK2;
	m_sourceKeyEnumMap["KEY_XSTICK1_RIGHT"] = KEY_XSTICK1_RIGHT;
	m_sourceKeyEnumMap["KEY_XSTICK1_LEFT"] = KEY_XSTICK1_LEFT;
	m_sourceKeyEnumMap["KEY_XSTICK1_DOWN"] = KEY_XSTICK1_DOWN;
	m_sourceKeyEnumMap["KEY_XSTICK1_UP"] = KEY_XSTICK1_UP;
	m_sourceKeyEnumMap["KEY_XBUTTON_LTRIGGER"] = KEY_XBUTTON_LTRIGGER;
	m_sourceKeyEnumMap["KEY_XBUTTON_RTRIGGER"] = KEY_XBUTTON_RTRIGGER;
	m_sourceKeyEnumMap["KEY_XSTICK2_RIGHT"] = KEY_XSTICK2_RIGHT;
	m_sourceKeyEnumMap["KEY_XSTICK2_LEFT"] = KEY_XSTICK2_LEFT;
	m_sourceKeyEnumMap["KEY_XSTICK2_DOWN"] = KEY_XSTICK2_DOWN;
	m_sourceKeyEnumMap["KEY_XSTICK2_UP"] = KEY_XSTICK2_UP;
}

C_InputManager::~C_InputManager()
{
	DevMsg("InputManager: Destructor\n");
}

/*
void C_InputManager::SetInputListener(void* pInputListener, listener_t type)
{
	m_pInputListener = pInputListener;
	m_inputListenerType = type;
}
*/

vgui::KeyCode C_InputManager::StringToSteamKeyEnum(std::string text)
{
	auto it = m_sourceKeyEnumMap.find(text);
	if (it != m_sourceKeyEnumMap.end())
		return it->second;

	return KEY_NONE;
}

void C_InputManager::ForceInputMode()
{
	if (!m_bInputMode)
		return;

	m_bForcedInputMode = true;
	m_bWasForcedInputMode = true;
}

void C_InputManager::ActivateInputMode(bool bFullscreen, bool bMainMenu, C_EmbeddedInstance* pEmbeddedInstance, bool bOverlay)//C_InputListener* pListener)
{
	if (g_pAnarchyManager->IsPaused())
		return;

//	/*
	if (m_bInputMode || (!bFullscreen && !g_pAnarchyManager->GetSelectedEntity() && !bOverlay))
		return;
//	*/

	if (m_bInputMode)
		return;

	m_bMainMenuMode = bMainMenu;

		//engine->ClientCmd("pause");

	m_bInputMode = true;

//	/*
	m_bFullscreenMode = (bFullscreen || !g_pAnarchyManager->GetSelectedEntity());	// !g_pAnarchyManager->GetSelectedEntity();// (bFullscreen || !g_pAnarchyManager->GetSelectedEntity());	// Only allow non-fullscreen mode if there is an entity selected
//	*/

	//m_bFullscreenMode = bFullscreen;	// this is probably not supposed to be here.  delete it & this comment.
	m_bOverlayMode = bOverlay;

	//m_pInputListener = pListener;
	m_pEmbeddedInstance = pEmbeddedInstance;

//	if (bFullscreen)
//		m_bWasForcedInputMode = true;

	// if no web tab is selected, then select the hud web tab.
//	/*
	//C_WebManager* pWebManager = g_pAnarchyManager->GetWebManager();

//	g_pAnarchyManager->GetSelectedEntity()->webt
	//C_EmbeddedInstance* pSelectedEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
//	if (pSelectedEmbeddedInstance)
		//pS

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	if (!m_pEmbeddedInstance)
	{
		pHudBrowserInstance->Select();
		pHudBrowserInstance->Focus();
		this->SetEmbeddedInstance(pHudBrowserInstance);
	}
	else if (m_pEmbeddedInstance == pHudBrowserInstance)
	{
		pHudBrowserInstance->Select();
		pHudBrowserInstance->Focus();
		//this->SetEmbeddedInstance(pHudBrowserInstance);
	}

//	C_EmbeddedInstance* pSelectedEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
//	if (pSelectedEmbeddedInstance && )

		//		g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(pHudBrowserInstance);
//	if (!g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance())
//		g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(pHudBrowserInstance);
		//pWebManager->SelectWebTab(pWebManager->GetHudWebTab());
//	*/

	//ShowCursor(true);
	InputSlate->Create(enginevgui->GetPanel(PANEL_ROOT));

	if (m_bFullscreenMode && bMainMenu && engine->IsInGame())
		engine->ServerCmd("pause");

	//g_pAnarchyManager->GetWebManager()->OnActivateInputMode();

	//if (m_bMainMenuMode)
	//{
		//e
		//engine->ClientCmd("toggleconsole;\n");
		//engine->ExecuteClientCmd("toggleconsole; toggleconsole;");
		//engine->ClientCmd_Unrestricted("toggleconsole; toggleconsole;");
		//engine->ClientCmd_Unrestricted("setpause");
		//engine->ServerCmd("pause");
		//		engine->ClientCmd("pause;\n");
		//engine->ExecuteClientCmd("unpause\n");
		//engine->ExecuteClientCmd("pause\n");
	//}

	//InputSlate->Create(enginevgui->GetPanel(PANEL_GAMEDLL));
	//InputSlate->Create(enginevgui->GetPanel(PANEL_TOOLS));
	//InputSlate->Create(enginevgui->GetPanel(PANEL_INGAMESCREENS));
	//InputSlate->Create(enginevgui->GetPanel(PANEL_CLIENTDLL));
}

void C_InputManager::DeactivateInputMode(bool bForce)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	if (!bForce && m_bForcedInputMode)
	{
		m_bForcedInputMode = false;
		return;
	}
	
	if (!m_bInputMode)
		return;

	C_EmbeddedInstance* pOldInstance = m_pEmbeddedInstance;

	//if (m_bMainMenuMode)
	//	engine->ClientCmd("toggleconsole;\n");
		//engine->ClientCmd_Unrestricted("unpause");
	//	engine->ServerCmd("unpause");
		//engine->ClientCmd("unpause;\n");
		//engine->ExecuteClientCmd("unpause\n");
		//engine->ClientCmd("unpause");

//	/*
	if (g_pAnarchyManager->GetInputManager()->GetMainMenuMode())
		g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud")->SetUrl("asset://ui/blank.html");
//		g_pAnarchyManager->GetWebManager()->GetHudWebTab()->SetUrl("asset://ui/blank.html");

	if (m_bFullscreenMode && !m_bOverlayMode)	// TODO: Add more checks here, like if the selected entity's web tab is also the selected entity.
	{
	//	C_WebManager* pWebManager = g_pAnarchyManager->GetWebManager();
		//if (pWebManager->GetSelectedWebTab() && !g_pAnarchyManager->GetSelectedEntity())
		if (m_pEmbeddedInstance && !g_pAnarchyManager->GetSelectedEntity())
		{
			m_pEmbeddedInstance->Deselect();
			m_pEmbeddedInstance->Blur();
		}
			//pWebManager->DeselectWebTab(pWebManager->GetSelectedWebTab());
		
		if (g_pAnarchyManager->GetInputManager()->GetMainMenuMode())
			engine->ClientCmd("gamemenucommand ResumeGame");
	}
//	*/

	m_bInputMode = false;
	m_bForcedInputMode = false;
	m_bWasForcedInputMode = false;
	m_bFullscreenMode = false;
	m_bOverlayMode = false;
	m_bMainMenuMode = false;
	//m_pEmbeddedInstance = null;
	//ShowCursor(false);
	InputSlate->Destroy();
}

void C_InputManager::MouseMove(float x, float y)
{
	if (m_pEmbeddedInstance)
	{
		C_InputListener* pInputListener = m_pEmbeddedInstance->GetInputListener();
		if (pInputListener)
			pInputListener->OnMouseMove(x, y);
	}

	C_AwesomiumBrowserInstance* pHudEmbeddedInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	C_InputListenerAwesomiumBrowser* pInputListenerAwesomiumBrowser = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetInputListener();
	if (pInputListenerAwesomiumBrowser)
		pInputListenerAwesomiumBrowser->OnMouseMove(x, y, pHudEmbeddedInstance);
	/*
	C_InputListenerAwesomiumBrowser* pInputListenerAwesomiumBrowser = dynamic_cast<C_InputListenerAwesomiumBrowser*>(pHudEmbeddedInstance->GetInputListener());	// this actually returns the 1 input listener that is used for ALL awesomium browser instances.
	if (pInputListenerAwesomiumBrowser)
		pInputListenerAwesomiumBrowser->OnMouseMove(x, y, pInputListenerAwesomiumBrowser);
	*/

	/*
	// forward this info to any listeners
	if (m_pInputListener)
	{
		if (m_inputListenerType == LISTENER_WEB_MANAGER)
		{
			C_WebManager* pWebManager = static_cast<C_WebManager*>(m_pInputListener);
			if (pWebManager)
				pWebManager->OnMouseMove(x, y);
		}
	}
	*/
}

void C_InputManager::MousePress(vgui::MouseCode code)
{
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	//pHudBrowserInstance->Select();
	pHudBrowserInstance->Focus();
	if (pHudBrowserInstance)
		pHudBrowserInstance->GetInputListener()->OnMousePressed(code);
//	if (m_pInputListener)
	//	m_pInputListener->OnMousePressed(code);
	/*
	// forward this info to any listeners
	if (m_pInputListener)
	{
		if (m_inputListenerType == LISTENER_WEB_MANAGER)
		{
			C_WebManager* pWebManager = static_cast<C_WebManager*>(m_pInputListener);
			if (pWebManager)
				pWebManager->OnMousePress(code);
		}
	}
	*/
}

void C_InputManager::MouseRelease(vgui::MouseCode code)
{
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	if (pHudBrowserInstance)
		pHudBrowserInstance->GetInputListener()->OnMouseReleased(code);

	//if (m_pInputListener)
	//	m_pInputListener->OnMouseReleased(code);
	/*
	// forward this info to any listeners
	if (m_pInputListener)
	{
		if (m_inputListenerType == LISTENER_WEB_MANAGER)
		{
			C_WebManager* pWebManager = static_cast<C_WebManager*>(m_pInputListener);
			if (pWebManager)
				pWebManager->OnMouseRelease(code);
		}
	}
	*/
}

void C_InputManager::KeyCodePressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState)
{
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	if (pHudBrowserInstance->HasFocus())
	{
		C_InputListener* pInputListener = pHudBrowserInstance->GetInputListener();
		pInputListener->OnKeyCodePressed(code, bShiftState, bCtrlState, bAltState, bWinState, bAutorepeatState);
	}
	else if (m_pEmbeddedInstance)
	{
		C_InputListener* pInputListener = m_pEmbeddedInstance->GetInputListener();
		pInputListener->OnKeyCodePressed(code, bShiftState, bCtrlState, bAltState, bWinState, bAutorepeatState);
	}
	/*
	// forward this info to any listeners
	if (m_pInputListener)
	{
		if (m_inputListenerType == LISTENER_WEB_MANAGER)
		{
			C_WebManager* pWebManager = static_cast<C_WebManager*>(m_pInputListener);
			if (pWebManager)
			{
				pWebManager->OnKeyCodePressed(code, bShiftState, bCtrlState, bAltState);
			}
		}
	}
	*/
}

void C_InputManager::OnMouseWheeled(int delta)
{
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	if (pHudBrowserInstance->HasFocus())
	{
		C_InputListener* pInputListener = pHudBrowserInstance->GetInputListener();
		pInputListener->OnMouseWheeled(delta);
	}
	else if (m_pEmbeddedInstance)
	{
		C_InputListener* pInputListener = m_pEmbeddedInstance->GetInputListener();
		pInputListener->OnMouseWheeled(delta);
	}
}

void C_InputManager::MouseWheelDown()
{
	if (!this->GetInputMode())
		return; // DO DEFAULT MAPPED MOUSE WHEEL DOWN ACTION!!

	this->OnMouseWheeled(-10);
}

void C_InputManager::MouseWheelUp()
{
	if (!this->GetInputMode())
		return; // DO DEFAULT MAPPED MOUSE WHEEL DOWN ACTION!!

	this->OnMouseWheeled(10);
}

void C_InputManager::KeyCodeReleased(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState)
{
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	if (pHudBrowserInstance->HasFocus())
	{
		C_InputListener* pInputListener = pHudBrowserInstance->GetInputListener();
		pInputListener->OnKeyCodeReleased(code, bShiftState, bCtrlState, bAltState, bWinState, bAutorepeatState);
		//C_InputListenerAwesomiumBrowser* listener = dynamic_cast<C_InputListenerAwesomiumBrowser*>(pInputListener);
		//listener->OnKeyCodeReleased(code, bShiftState, bCtrlState, bAltState, bWinState, bAutorepeatState);
	}
	else if (m_pEmbeddedInstance)
	{
		C_InputListener* pInputListener = m_pEmbeddedInstance->GetInputListener();
		pInputListener->OnKeyCodeReleased(code, bShiftState, bCtrlState, bAltState, bWinState, bAutorepeatState);
	}
	/*
	// forward this info to any listeners
	if (m_pInputListener)
	{
		if (m_inputListenerType == LISTENER_WEB_MANAGER)
		{
			C_WebManager* pWebManager = static_cast<C_WebManager*>(m_pInputListener);
			if (pWebManager)
			{
				pWebManager->OnKeyCodeReleased(code, bShiftState, bCtrlState, bAltState);
			}
		}
	}
	*/
}