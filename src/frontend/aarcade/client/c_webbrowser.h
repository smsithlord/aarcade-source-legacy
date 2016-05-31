#ifndef C_WEB_BROWSER_H
#define C_WEB_BROWSER_H

#include <Awesomium/WebCore.h>
#include <Awesomium/STLHelpers.h>
#include "c_awesomiumlisteners.h"
#include "c_awesomiumdatasources.h"
#include "c_webtab.h"
#include <map>

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

	// accessors
	int GetState() { return m_iState; }

	// master
	void OnMasterWebViewDocumentReady();

	// regular
	void PrepareWebView(WebView* pWebView);
	void OnCreateWebViewDocumentReady(WebView* pWebView, std::string id);
	void RegenerateTextureBits(C_WebTab* pWebTab, ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect);

private:
	int m_iState;
	WebCore* m_pWebCore;
	WebSession* m_pWebSession;
	std::map<C_WebTab*, WebView*> m_webViews;
	
	// master
	WebView* m_pMasterWebView;
	MasterLoadListener* m_pMasterLoadListener;
	MasterViewListener* m_pMasterViewListener;

	// regular
	LoadListener* m_pLoadListener;
	ViewListener* m_pViewListener;

	// JS Handlers next...
};

#endif