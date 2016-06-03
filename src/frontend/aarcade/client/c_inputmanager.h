#ifndef C_INPUT_MANAGER_H
#define C_INPUT_MANAGER_H

#include "vgui/MouseCode.h"

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

	void SetInputListener(void* pInputListener, listener_t type);
	void ActivateInputMode(bool bFullscreen = false);
	void DeactivateInputMode();
	void MouseMove(float x, float y);
	void MousePress(vgui::MouseCode code);
	void MouseRelease(vgui::MouseCode code);
	
private:
	bool m_bInputMode;
	bool m_bFullscreenMode;
	//float m_fMouseX;
	//float m_fMouseY;
	void* m_pInputListener;
	listener_t m_inputListenerType;
};

#endif