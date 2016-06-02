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
	C_WebTab* GetSelectedWebTab() { return m_pSelectedWebTab; }
	C_WebBrowser* GetWebBrowser() { return m_pWebBrowser; }
	int GetWebSurfaceWidth() { return m_iWebSurfaceWidth; }
	int GetWebSurfaceHeight() { return m_iWebSurfaceHeight; }

	// mutators
	void SetLastRenderedFrame(int frame) { m_iLastRenderedFrame = frame; }

	void OnBrowserInitialized();
	C_WebTab* CreateWebTab(std::string url, std::string id = "");
	C_WebTab* FindWebTab(std::string id);
	C_WebTab* FindWebTab(IMaterial* pMaterial);
	void SetMaterialWebTabId(IMaterial* pMaterial, std::string id);
	void SelectWebTab(C_WebTab* pWebTab);
	void DeselectWebTab(C_WebTab* pWebTab);
	CWebSurfaceRegen* GetOrCreateWebSurfaceRegen();
	void IncrementVisibleWebTabsCurrentFrame() { m_iVisibleWebTabsCurrentFrame++; }
	bool ShouldRender(C_WebTab* pWebTab);
	void OnMouseMove(float fXAmount, float fYAmount);
	void OnMousePress(vgui::MouseCode code);
	void OnMouseRelease(vgui::MouseCode code);

	/*
	void RelayOnMouseMove(int x, int y);
	void RelayOnMouseDown(int id);
	void RelayOnMouseUp(int id);
	void RelayOnKeyDown(vgui::KeyCode code);
	*/
	
private:
	unsigned int m_iState;
	C_WebTab* m_pSelectedWebTab;
	C_WebBrowser* m_pWebBrowser;
	std::map<std::string, C_WebTab*> m_webTabs;
	std::map<IMaterial*, std::string> m_materialWebTabIds;
	CWebSurfaceRegen* m_pWebSurfaceRegen;
	int m_iVisibleWebTabsLastFrame;
	int m_iVisibleWebTabsCurrentFrame;
	int m_iLastRenderedFrame;
	int m_iWebSurfaceWidth;
	int m_iWebSurfaceHeight;
};

#endif