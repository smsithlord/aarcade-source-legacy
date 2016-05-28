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

	// accessors
	int GetState() { return m_iState; }

	// master
	void OnMasterWebViewDocumentReady();

	// regular
	void PrepareWebView(Awesomium::WebView* pWebView);
	void OnCreateWebViewDocumentReady(WebView* pWebView, std::string id);

private:
	int m_iState;
	Awesomium::WebCore* m_pWebCore;
	Awesomium::WebSession* m_pWebSession;
	
	// master
	Awesomium::WebView* m_pMasterWebView;
	MasterLoadListener* m_pMasterLoadListener;
	MasterViewListener* m_pMasterViewListener;

	// regular
	LoadListener* m_pLoadListener;
	ViewListener* m_pViewListener;

	// JS Handlers next...
};

#endif