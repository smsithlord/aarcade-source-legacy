#ifndef C_INPUT_MANAGER_H
#define C_INPUT_MANAGER_H

#include "vgui/MouseCode.h"
#include "vgui/KeyCode.h"
#include <map>

#include "c_inputlistener.h"
#include "c_embeddedinstance.h"
/*
enum listener_t
{
	LISTENER_NONE = 0,
	LISTENER_WEB_MANAGER = 1,
	LISTENER_LIBRETRO_MANAGER = 2
};
*/

class C_InputManager
{
public:
	C_InputManager();
	~C_InputManager();

	vgui::KeyCode StringToSteamKeyEnum(std::string text);

	// accessors
	C_EmbeddedInstance* GetEmbeddedInstance() { return m_pEmbeddedInstance; }
	//ITexture* GetInputCanvasTexture() { return m_pInputCanvasTexture; }
	bool GetInputMode() { return m_bInputMode; }
	bool GetFullscreenMode() { return m_bFullscreenMode; }
	bool GetOverlayMode() { return m_bOverlayMode; }
	bool GetMainMenuMode() { return m_bMainMenuMode; }
	bool GetForceInputMode() { return m_bForcedInputMode; }
	bool GetWasForceInputMode() { return m_bWasForcedInputMode; }

	void SetFullscreenMode(bool value) { m_bFullscreenMode = value; }
	void SetOverlayMode(bool value) { m_bOverlayMode = value; }
	void ActivateInputMode(bool bFullscreen = false, bool bMainMenu = false, C_EmbeddedInstance* pEmbeddedInstance = null, bool bOverlay = false);
	void ForceInputMode();
	void DeactivateInputMode(bool bForce = false);
	void MouseMove(float x, float y);
	void MousePress(vgui::MouseCode code);
	void MouseRelease(vgui::MouseCode code);
	void OnMouseWheeled(int delta);
	void KeyCodePressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState);
	void KeyCodeReleased(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState);

	void MouseWheelDown();
	void MouseWheelUp();

	// mutators
	void SetEmbeddedInstance(C_EmbeddedInstance* pEmbeddedInstance) { m_pEmbeddedInstance = pEmbeddedInstance; }
	//void SetInputListener(C_InputListener* pListener) { m_pInputListener = pListener; }
	//void SetInputCanvasTexture(ITexture* pTexture) { m_pInputCanvasTexture = pTexture; }
	
private:
	std::map<std::string, vgui::KeyCode> m_sourceKeyEnumMap;
	C_EmbeddedInstance* m_pEmbeddedInstance;
	bool m_bWasForcedInputMode;
	bool m_bForcedInputMode;
	bool m_bInputMode;
	bool m_bFullscreenMode;
	bool m_bOverlayMode;
	bool m_bMainMenuMode;
	//float m_fMouseX;
	//float m_fMouseY;
	//C_InputListener* m_pInputListener;
	//ITexture* m_pInputCanvasTexture;
	//listener_t m_inputListenerType;
};

#endif