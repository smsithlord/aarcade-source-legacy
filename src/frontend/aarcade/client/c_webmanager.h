#ifndef C_WEB_MANAGER_H
#define C_WEB_MANAGER_H

//#include <Awesomium/WebCore.h>
//#include <Awesomium/STLHelpers.h>
//#include "vgui_controls/Controls.h"
#include "c_webbrowser.h"
#include "c_webtab.h"
#include "c_websurfaceregen.h"
#include "c_websurfaceproxy.h"
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
	C_WebTab* GetHudWebTab() { return m_pHudWebTab; }
	C_WebBrowser* GetWebBrowser() { return m_pWebBrowser; }
	int GetWebSurfaceWidth() { return m_iWebSurfaceWidth; }
	int GetWebSurfaceHeight() { return m_iWebSurfaceHeight; }
	bool GetHudPriority() { return m_bHudPriority;}
	bool GetSelectedPriority() { return m_bSelectedPriority; }
	bool GetHudReady() { return m_bHudReady; }

	// mutators
	void SetLastRenderedFrame(int frame) { m_iLastRenderedFrame = frame; }
	void SetLastPriorityRenderedFrame(int frame) { m_iLastPriorityRenderedFrame = frame; }

	void LevelShutdownPreEntity();

	void OnBrowserInitialized();
	void OnHudWebTabReady();
	void OnLoadingWebTabReady();
	void RemoveWebTab(C_WebTab* pWebTab);
	C_WebTab* CreateHudWebTab();
	C_WebTab* CreateWebTab(std::string url, std::string id = "", bool bAlpha = false);
	C_WebTab* FindWebTab(std::string id);
	C_WebTab* FindWebTab(IMaterial* pMaterial);
	void SetMaterialWebTabId(IMaterial* pMaterial, std::string id);
	void SelectWebTab(C_WebTab* pWebTab);
	void DeselectWebTab(C_WebTab* pWebTab);
	CWebSurfaceRegen* GetOrCreateWebSurfaceRegen();
	void IncrementVisibleWebTabsCurrentFrame() { m_iVisibleWebTabsCurrentFrame++; }
	void IncrementVisiblePriorityWebTabsCurrentFrame() { m_iVisiblePriorityWebTabsCurrentFrame++; }

	bool IsPriorityWebTab(C_WebTab* pWebTab);
	unsigned int GetNumPriorityWebTabs() { return 2; }

	bool ShouldRender(C_WebTab* pWebTab);
	void OnMouseMove(float fXAmount, float fYAmount);
	void OnMousePress(vgui::MouseCode code);
	void OnMouseRelease(vgui::MouseCode code);
	void OnKeyCodePressed(vgui::MouseCode code, bool bShiftState, bool bCtrlState, bool bAltState);
	void OnKeyCodeReleased(vgui::MouseCode code, bool bShiftState, bool bCtrlState, bool bAltState);

	void DispatchJavaScriptMethod(C_WebTab* pWebTab, std::string objectName, std::string objectMethod, std::vector<std::string> methodArguments);
	void DispatchJavaScriptMethods(C_WebTab* pWebTab);
	//void DispatchJavaScriptMethodBatch(C_WebTab* pWebTab, std::vector<MethodBatch_t*> batch);
	//void DispatchJavaScriptEventMessages(C_WebTab* pWebTab, std::string objectName, std::string objectMethod, std::vector<JSEventMessage_t*> args);

	/*
	void RelayOnMouseMove(int x, int y);
	void RelayOnMouseDown(int id);
	void RelayOnMouseUp(int id);
	void RelayOnKeyDown(vgui::KeyCode code);
	*/
	
	void RegisterProxy(CWebSurfaceProxy* pProxy);

private:
	bool m_bHudReady;
	bool m_bHudPriority;
	bool m_bSelectedPriority;
	unsigned int m_iState;
	C_WebTab* m_pSelectedWebTab;
	C_WebTab* m_pHudWebTab;
	C_WebBrowser* m_pWebBrowser;
	std::map<std::string, C_WebTab*> m_webTabs;
	std::map<IMaterial*, std::string> m_materialWebTabIds;
	CWebSurfaceRegen* m_pWebSurfaceRegen;
	std::vector<CWebSurfaceProxy*> m_webSurfaceProxies;
	int m_iVisibleWebTabsLastFrame;
	int m_iVisiblePriorityWebTabsLastFrame;
	int m_iVisibleWebTabsCurrentFrame;
	int m_iVisiblePriorityWebTabsCurrentFrame;
	int m_iLastRenderedFrame;
	int m_iLastPriorityRenderedFrame;
	int m_iWebSurfaceWidth;
	int m_iWebSurfaceHeight;
};

#endif