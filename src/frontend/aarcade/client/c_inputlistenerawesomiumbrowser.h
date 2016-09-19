#ifndef C_INPUT_LISTENER_AWESOMIUM_BROWSER_H
#define C_INPUT_LISTENER_AWESOMIUM_BROWSER_H

#include "c_inputlistener.h"
#include "c_awesomiumbrowserinstance.h"
//#include "c_canvas.h"
//#include <vector>

#include "vgui_controls/Controls.h"

class C_InputListenerAwesomiumBrowser : public C_InputListener
{
public:
	C_InputListenerAwesomiumBrowser();
	~C_InputListenerAwesomiumBrowser();

	void OnMouseMove(float x, float y);
	void OnMouseMove(float x, float y, C_AwesomiumBrowserInstance* pBrowserInstance);
	void OnMousePressed(vgui::MouseCode code);
	void OnMouseReleased(vgui::MouseCode code);

	// accessors
	// mutators
	
private:

};

#endif