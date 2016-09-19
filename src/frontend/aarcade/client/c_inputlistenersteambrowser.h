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

	void OnKeyCodePressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState);
	void OnKeyCodeReleased(vgui::KeyCode code);

	// accessors
	// mutators
	
private:

};

#endif