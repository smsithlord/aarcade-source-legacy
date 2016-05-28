#ifndef C_WEB_TAB_H
#define C_WEB_TAB_H

#include <Awesomium/WebCore.h>
#include <Awesomium/STLHelpers.h>
//#include "c_awesomiumListeners.h"
//#include "vgui_controls/Controls.h"

class C_WebTab
{
public:
	C_WebTab(std::string url);
	~C_WebTab();

	// accessors
	std::string GetId() { return m_id; }
	std::string GetInitialUrl() { return m_initialUrl; }

	// mutators
	void SetState(int state) { m_iState = state; }

private:
	int m_iState;
	std::string m_id;
	std::string m_initialUrl;
	Awesomium::WebView* m_pWebView;
};

#endif