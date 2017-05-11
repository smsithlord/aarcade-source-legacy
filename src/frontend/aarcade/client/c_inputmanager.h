#ifndef C_INPUT_MANAGER_H
#define C_INPUT_MANAGER_H

#include "vgui/MouseCode.h"
#include "vgui/KeyCode.h"
#include <map>

//#include "c_inputslate.h"
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

	void Update();

	vgui::KeyCode StringToSteamKeyEnum(std::string text);

	ITexture* GetInputSlateCanvasTexture();

	// accessors
	bool GetInputCapture() { return m_bInputCapture; }
	C_EmbeddedInstance* GetEmbeddedInstance() { return m_pEmbeddedInstance; }
	bool GetInputMode() { return m_bInputMode; }
	bool GetFullscreenMode() { return m_bFullscreenMode; }
	bool GetMainMenuMode() { return m_bMainMenuMode; }
	bool GetForceInputMode() { return m_bForcedInputMode; }
	bool GetWasForceInputMode() { return m_bWasForcedInputMode; }

	
	void SetFullscreenMode(bool value) { m_bFullscreenMode = value; }
	//void SetOverlayMode(bool value) { m_bOverlayMode = value; }
	void ActivateInputMode(bool bFullscreen = false, bool bMainMenu = false, C_EmbeddedInstance* pEmbeddedInstance = null, bool bInputCapture = true);
	void ForceInputMode();
	void DeactivateInputMode(bool bForce = false);
	//void ShutdownInputMode();
	void MouseMove(float x, float y);
	void MousePress(vgui::MouseCode code);
	void MouseRelease(vgui::MouseCode code);
	void OnMouseWheeled(int delta);
	void KeyCodePressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState);
	void KeyCodeReleased(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState);

	void MouseWheelDown();
	void MouseWheelUp();

	// mutators
	void SetInputCapture(bool value) { m_bInputCapture = value; }
	void SetEmbeddedInstance(C_EmbeddedInstance* pEmbeddedInstance) { m_pEmbeddedInstance = pEmbeddedInstance; }
	
private:
	std::map<std::string, vgui::KeyCode> m_sourceKeyEnumMap;
	C_EmbeddedInstance* m_pEmbeddedInstance;
	bool m_bWasForcedInputMode;
	bool m_bForcedInputMode;
	bool m_bInputMode;
	bool m_bInputCapture;
	bool m_bFullscreenMode;
	bool m_bMainMenuMode;
};

#endif