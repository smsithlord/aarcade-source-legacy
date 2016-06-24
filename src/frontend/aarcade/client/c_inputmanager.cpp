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
	m_pInputListener = null;
}

C_InputManager::~C_InputManager()
{
	DevMsg("InputManager: Destructor\n");
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

void C_InputManager::ActivateInputMode(bool bFullscreen)
{
	if (m_bInputMode)
		return;

	m_bInputMode = true;
	m_bFullscreenMode = !g_pAnarchyManager->GetSelectedEntity();// (bFullscreen || !g_pAnarchyManager->GetSelectedEntity());	// Only allow non-fullscreen mode if there is an entity selected

	// if no web tab is selected, then select the hud web tab.
	C_WebManager* pWebManager = g_pAnarchyManager->GetWebManager();
	if (!pWebManager->GetSelectedWebTab())
		pWebManager->SelectWebTab(pWebManager->GetHudWebTab());

	//ShowCursor(true);
	InputSlate->Create(enginevgui->GetPanel(PANEL_ROOT));

	g_pAnarchyManager->GetWebManager()->OnActivateInputMode(m_bFullscreenMode);

	//InputSlate->Create(enginevgui->GetPanel(PANEL_GAMEDLL));
	//InputSlate->Create(enginevgui->GetPanel(PANEL_TOOLS));
	//InputSlate->Create(enginevgui->GetPanel(PANEL_INGAMESCREENS));
	//InputSlate->Create(enginevgui->GetPanel(PANEL_CLIENTDLL));
}

void C_InputManager::DeactivateInputMode(bool bForce)
{
	if (!bForce && m_bForcedInputMode)
	{
		m_bForcedInputMode = false;
		return;
	}
	
	if (!m_bInputMode)
		return;

	if (m_bFullscreenMode)	// TODO: Add more checks here, like if the selected entity's web tab is also the selected entity.
	{
		C_WebManager* pWebManager = g_pAnarchyManager->GetWebManager();
		if (pWebManager->GetSelectedWebTab())
			pWebManager->DeselectWebTab(pWebManager->GetSelectedWebTab());
	}

	m_bInputMode = false;
	m_bForcedInputMode = false;
	m_bWasForcedInputMode = false;
	m_bFullscreenMode = false;
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