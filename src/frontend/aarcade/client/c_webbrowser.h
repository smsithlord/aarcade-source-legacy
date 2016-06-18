#ifndef C_WEB_BROWSER_H
#define C_WEB_BROWSER_H

#include <Awesomium/WebCore.h>
#include <Awesomium/STLHelpers.h>
#include "c_awesomiumlisteners.h"
#include "c_awesomiumdatasources.h"
#include "c_awesomiumjshandlers.h"
#include "c_webtab.h"
#include <map>
//#include "aa_globals.h"
//#include "c_webmanager.h"
//#include "vgui_controls/Controls.h"

class C_WebBrowser
{
public:
	C_WebBrowser();
	~C_WebBrowser();

	void Init();
	void Update();

	void CreateWebView(C_WebTab* pWebTab);
	WebView* FindWebView(C_WebTab* pWebTab);
	void OnSelectWebTab(C_WebTab* pWebTab);
	void OnDeselectWebTab(C_WebTab* pWebTab);
	void OnMouseMove(C_WebTab* pWebTab, float fMouseX, float fMouseY);
	void OnMousePress(C_WebTab* pWebTab, vgui::MouseCode code);
	void OnMouseRelease(C_WebTab* pWebTab, vgui::MouseCode code);

	void CreateAaApi(WebView* pWebView);

	void RemoveWebView(C_WebTab* pWebTab);

	// accessors
	int GetState() { return m_iState; }

	// master
	void OnMasterWebViewDocumentReady();

	// regular
	void PrepareWebView(WebView* pWebView, std::string id);
	void OnCreateWebViewDocumentReady(WebView* pWebView, std::string id);
	void RegenerateTextureBits(C_WebTab* pWebTab, ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect);

	void OnHudWebViewDocumentReady(WebView* pWebView, std::string id);
	void OnLoadingWebViewDocumentReady(WebView* pWebView, std::string id);
	void DispatchJavaScriptMethod(C_WebTab* pWebTab, std::string objectName, std::string objectMethod, std::vector<std::string> methodArguments);
	//void DispatchJavaScriptMethodBatch(C_WebTab* pWebTab, std::vector<MethodBatch_t*> batch);

private:
	int m_iState;
	WebCore* m_pWebCore;
	WebSession* m_pWebSession;
	std::map<C_WebTab*, WebView*> m_webViews;

	JSObject* m_aaApi;
	
	// master
	WebView* m_pMasterWebView;
	MasterLoadListener* m_pMasterLoadListener;
	MasterViewListener* m_pMasterViewListener;

	// regular
	JSHandler* m_pJSHandler;
	LoadListener* m_pLoadListener;
	ViewListener* m_pViewListener;

	// JS Handlers next...
};

inline const char* WebStringToCharString(WebString web_string)
{
	int len = web_string.ToUTF8(null, 0);
	char* buf = new char[len + 1];
	web_string.ToUTF8(buf, len);
	buf[len] = 0;	// null terminator

	std::string title = buf;
	delete[] buf;

	return VarArgs("%s", title.c_str());
}

#endif