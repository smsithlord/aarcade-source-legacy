#ifndef C_INPUT_LISTENER_STEAM_BROWSER_H
#define C_INPUT_LISTENER_STEAM_BROWSER_H

#include "c_inputlistener.h"
//#include "c_canvas.h"
//#include <vector>

#include "vgui_controls/Controls.h"

class C_InputListenerSteamBrowser : public C_InputListener
{
public:
	C_InputListenerSteamBrowser();
	~C_InputListenerSteamBrowser();

	void OnMouseMove(float x, float y);
	void OnMousePressed(vgui::MouseCode code);
	void OnMouseReleased(vgui::MouseCode code);
	void OnMouseWheeled(int delta);

	void OnKeyCodePressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState);
	void OnKeyCodeReleased(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState);

	// accessors
	// mutators
	
private:

};

#endif