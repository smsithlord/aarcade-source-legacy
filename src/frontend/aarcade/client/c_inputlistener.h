#ifndef C_INPUT_LISTENER_H
#define C_INPUT_LISTENER_H

//#include "c_canvas.h"
//#include <vector>

#include "vgui_controls/Controls.h"

class C_InputListener
{
public:
	C_InputListener();
	~C_InputListener();

	virtual void OnMouseMove(float x, float y) {};
	virtual void OnMousePressed(vgui::MouseCode code) {};
	virtual void OnMouseReleased(vgui::MouseCode code) {};
	virtual void OnKeyCodePressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState) {};
	virtual void OnKeyCodeReleased(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState) {};
	virtual void OnMouseWheeled(int delta) {};

	// accessors
	// mutators
	
private:

};

#endif