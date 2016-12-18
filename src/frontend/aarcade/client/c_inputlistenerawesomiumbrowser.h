#ifndef C_INPUT_LISTENER_AWESOMIUM_BROWSER_H
#define C_INPUT_LISTENER_AWESOMIUM_BROWSER_H

#include "c_inputlistener.h"
#include "c_awesomiumbrowserinstance.h"
//#include "c_canvas.h"
#include <map>

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
	void OnMouseWheeled(int delta);

	void OnKeyCodePressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState);
	void OnKeyCodeReleased(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState);
//	void OnKeyCodeReleased(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState) {};

	int ConvertSourceButtonToAwesomiumButton(ButtonCode_t sourceButton);

	// accessors
	// mutators
	
private:
	static std::map<ButtonCode_t, int> s_sourceToAwesomiumButtonMap;
	//std::map<int, const char*> m_awesomiumButtonMap;
	//std::map<int, const char*> m_awesomiumShiftedButtonMap;
};

#endif