#ifndef C_WEB_MANAGER_H
#define C_WEB_MANAGER_H

//#include <Awesomium/WebCore.h>
//#include <Awesomium/STLHelpers.h>
//#include "vgui_controls/Controls.h"
#include "c_webbrowser.h"
#include "c_webtab.h"
#include "c_websurfaceregen.h"
#include <map>

class C_WebManager
{
public:
	C_WebManager();
	~C_WebManager();

	void Init();
	void Update();

	// accessors
	C_WebBrowser* GetWebBrowser() { return m_pWebBrowser; }
	int GetWebSurfaceWidth() { return m_iWebSurfaceWidth; }
	int GetWebSurfaceHeight() { return m_iWebSurfaceHeight; }

	C_WebTab* CreateWebTab(std::string url, std::string id = "");
	C_WebTab* FindWebTab(std::string id);
	CWebSurfaceRegen* GetOrCreateWebSurfaceRegen();

	/*
	void RelayOnMouseMove(int x, int y);
	void RelayOnMouseDown(int id);
	void RelayOnMouseUp(int id);
	void RelayOnKeyDown(vgui::KeyCode code);
	*/
	
private:
	unsigned int m_iState;
	C_WebBrowser* m_pWebBrowser;
	std::map<std::string, C_WebTab*> m_webTabs;
	CWebSurfaceRegen* m_pWebSurfaceRegen;
	int m_iWebSurfaceWidth;
	int m_iWebSurfaceHeight;
};

#endif