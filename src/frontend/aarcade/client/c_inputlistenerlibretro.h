#ifndef C_INPUT_LISTENER_LIBRETRO_H
#define C_INPUT_LISTENER_LIBRETRO_H

#include "c_inputlistener.h"
//#include "c_canvas.h"
//#include <vector>
#include "vgui_controls/Controls.h"

class C_InputListenerLibretro : public C_InputListener
{
public:
	C_InputListenerLibretro();
	~C_InputListenerLibretro();

	void OnMouseMove(float x, float y) {};
	void OnMousePressed(vgui::MouseCode code) {};
	void OnMouseReleased(vgui::MouseCode code) {};

	//void OnKeyCodePressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState);
	//void OnKeyCodeReleased(vgui::KeyCode code);

	// accessors
	// mutators
	
private:

};

#endif