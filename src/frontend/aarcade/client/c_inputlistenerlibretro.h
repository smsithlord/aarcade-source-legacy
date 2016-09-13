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

	void OnMouseMove(float x, float y);
	void OnMousePressed(vgui::MouseCode code) {};
	void OnMouseReleased(vgui::MouseCode code) {};

	// accessors
	// mutators
	
private:

};

#endif