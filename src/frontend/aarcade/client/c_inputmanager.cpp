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
	m_bMainMenuMode = false;
	//m_pInputListener = null;
	//m_pInputCanvasTexture = null;
	m_pEmbeddedInstance = null;
}

C_InputManager::~C_InputManager()
{
	DevMsg("InputManager: Destructor\n");
}

void C_InputManager::SetFullscreenMode(bool value)
{
	m_bFullscreenMode = value;
//	m_pInputSlate
}

/*
void C_InputManager::SetInputListener(void* pInputListener, listener_t type)
{
	m_pInputListener = pInputListener;
	m_inputListenerType = type;
}
*/

void C_InputManager::ForceInputMode()
{
	if (!m_bInputMode)
		return;

	m_bForcedInputMode = true;
	m_bWasForcedInputMode = true;
}

void C_InputManager::ActivateInputMode(bool bFullscreen, bool bMainMenu, C_EmbeddedInstance* pEmbeddedInstance)//C_InputListener* pListener)
{
	if (g_pAnarchyManager->IsPaused())
		return;

//	/*
	if (m_bInputMode || (!bFullscreen && !g_pAnarchyManager->GetSelectedEntity()))
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

	m_bFullscreenMode = bFullscreen;

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

	if (m_bFullscreenMode)	// TODO: Add more checks here, like if the selected entity's web tab is also the selected entity.
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

void C_InputManager::KeyCodePressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState)
{
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	if (pHudBrowserInstance->HasFocus())
	{
		C_InputListener* pInputListener = pHudBrowserInstance->GetInputListener();
		pInputListener->OnKeyCodePressed(code, bShiftState, bCtrlState, bAltState);
	}
	else if (m_pEmbeddedInstance)
	{
		C_InputListener* pInputListener = m_pEmbeddedInstance->GetInputListener();
		pInputListener->OnKeyCodePressed(code, bShiftState, bCtrlState, bAltState);
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


void C_InputManager::KeyCodeReleased(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState)
{
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	if (pHudBrowserInstance->HasFocus())
	{
		C_InputListener* pInputListener = pHudBrowserInstance->GetInputListener();
		pInputListener->OnKeyCodeReleased(code);
	}
	else if (m_pEmbeddedInstance)
	{
		C_InputListener* pInputListener = m_pEmbeddedInstance->GetInputListener();
		pInputListener->OnKeyCodeReleased(code);
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