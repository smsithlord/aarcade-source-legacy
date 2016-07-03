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
	m_pInputListener = null;
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

void C_InputManager::SetInputListener(void* pInputListener, listener_t type)
{
	m_pInputListener = pInputListener;
	m_inputListenerType = type;
}

void C_InputManager::ForceInputMode()
{
	if (!m_bInputMode)
		return;

	m_bForcedInputMode = true;
	m_bWasForcedInputMode = true;
}

void C_InputManager::ActivateInputMode(bool bFullscreen, bool bMainMenu)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	if (m_bInputMode || (!bFullscreen && !g_pAnarchyManager->GetSelectedEntity()))
		return;

	m_bMainMenuMode = bMainMenu;

		//engine->ClientCmd("pause");

	m_bInputMode = true;
	m_bFullscreenMode = (bFullscreen || !g_pAnarchyManager->GetSelectedEntity());	// !g_pAnarchyManager->GetSelectedEntity();// (bFullscreen || !g_pAnarchyManager->GetSelectedEntity());	// Only allow non-fullscreen mode if there is an entity selected

//	if (bFullscreen)
//		m_bWasForcedInputMode = true;

	// if no web tab is selected, then select the hud web tab.
	C_WebManager* pWebManager = g_pAnarchyManager->GetWebManager();
	if (!pWebManager->GetSelectedWebTab())
		pWebManager->SelectWebTab(pWebManager->GetHudWebTab());

	//ShowCursor(true);
	InputSlate->Create(enginevgui->GetPanel(PANEL_ROOT));

	g_pAnarchyManager->GetWebManager()->OnActivateInputMode();

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

	//if (m_bMainMenuMode)
	//	engine->ClientCmd("toggleconsole;\n");
		//engine->ClientCmd_Unrestricted("unpause");
	//	engine->ServerCmd("unpause");
		//engine->ClientCmd("unpause;\n");
		//engine->ExecuteClientCmd("unpause\n");
		//engine->ClientCmd("unpause");

	if (g_pAnarchyManager->GetInputManager()->GetMainMenuMode())
		g_pAnarchyManager->GetWebManager()->GetHudWebTab()->SetUrl("asset://ui/blank.html");

	if (m_bFullscreenMode)	// TODO: Add more checks here, like if the selected entity's web tab is also the selected entity.
	{
		C_WebManager* pWebManager = g_pAnarchyManager->GetWebManager();
		if (pWebManager->GetSelectedWebTab() && !g_pAnarchyManager->GetSelectedEntity() )
			pWebManager->DeselectWebTab(pWebManager->GetSelectedWebTab());
	}

	m_bInputMode = false;
	m_bForcedInputMode = false;
	m_bWasForcedInputMode = false;
	m_bFullscreenMode = false;
	m_bMainMenuMode = false;
	//ShowCursor(false);
	InputSlate->Destroy();
}

void C_InputManager::MouseMove(float x, float y)
{
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
}

void C_InputManager::MousePress(vgui::MouseCode code)
{
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
}

void C_InputManager::MouseRelease(vgui::MouseCode code)
{
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
}

void C_InputManager::KeyCodePressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState)
{
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
}


void C_InputManager::KeyCodeReleased(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState)
{
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
}