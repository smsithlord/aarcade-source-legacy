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

void C_InputManager::ActivateInputMode(bool bFullscreen)
{
	m_bInputMode = true;
	m_bFullscreenMode = bFullscreen;

	//ShowCursor(true);
	InputSlate->Create(enginevgui->GetPanel(PANEL_ROOT));
	//InputSlate->Create(enginevgui->GetPanel(PANEL_GAMEDLL));
	//InputSlate->Create(enginevgui->GetPanel(PANEL_TOOLS));
	//InputSlate->Create(enginevgui->GetPanel(PANEL_INGAMESCREENS));
	//InputSlate->Create(enginevgui->GetPanel(PANEL_CLIENTDLL));
}

void C_InputManager::DeactivateInputMode()
{
	m_bInputMode = false;
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