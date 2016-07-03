#ifndef C_INPUT_MANAGER_H
#define C_INPUT_MANAGER_H

#include "vgui/MouseCode.h"
#include "vgui/KeyCode.h"

enum listener_t
{
	LISTENER_NONE = 0,
	LISTENER_WEB_MANAGER = 1,
	LISTENER_LIBRETRO_MANAGER = 2
};

class C_InputManager
{
public:
	C_InputManager();
	~C_InputManager();

	// accessors
	bool GetInputMode() { return m_bInputMode; }
	bool GetFullscreenMode() { return m_bFullscreenMode; }
	bool GetMainMenuMode() { return m_bMainMenuMode; }
	bool GetForceInputMode() { return m_bForcedInputMode; }
	bool GetWasForceInputMode() { return m_bWasForcedInputMode; }

	void SetFullscreenMode(bool value);
	void SetInputListener(void* pInputListener, listener_t type);
	void ActivateInputMode(bool bFullscreen = false, bool bMainMenu = false);
	void ForceInputMode();
	void DeactivateInputMode(bool bForce = false);
	void MouseMove(float x, float y);
	void MousePress(vgui::MouseCode code);
	void MouseRelease(vgui::MouseCode code);
	void KeyCodePressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState);
	void KeyCodeReleased(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState);
	
private:
	bool m_bWasForcedInputMode;
	bool m_bForcedInputMode;
	bool m_bInputMode;
	bool m_bFullscreenMode;
	bool m_bMainMenuMode;
	//float m_fMouseX;
	//float m_fMouseY;
	void* m_pInputListener;
	listener_t m_inputListenerType;
};

#endif