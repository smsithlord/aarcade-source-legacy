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

	void PrepareWebView(Awesomium::WebView* pWebView);
	void OnMasterWebViewDocumentReady();
	void OnCreateWebViewDocumentReady(WebView* pWebView, std::string id);

	void CreateWebView(C_WebTab* pWebTab);

	int GetState() { return m_iState; }
	/*
	Awesomium::WebCore* GetWebCore() { return m_pWebCore; };
	Awesomium::WebView* GetAwesomiumMasterWebView() { return m_pMasterWebView; }
	Awesomium::WebView* GetAwesomiumDynamicImageWebView() { return m_pDynamicImageWebView->GetAwesomiumWebView(); }
	void SetAwesomiumMasterWebView(Awesomium::WebView* pWebView) { m_pMasterWebView = pWebView; }
	void SetAwesomiumDynamicImageWebView(Awesomium::WebView* pWebView) { m_pDynamicImageWebView->SetAwesomiumWebView(pWebView); }
	*/

	/*
	void RelayOnMouseMove(int x, int y);
	void RelayOnMouseDown(int id);
	void RelayOnMouseUp(int id);
	void RelayOnKeyDown(vgui::KeyCode code);
	*/

private:
	int m_iState;
	Awesomium::WebCore* m_pWebCore;
	Awesomium::WebView* m_pMasterWebView;
	Awesomium::WebSession* m_pWebSession;

	std::map<std::string, bool> mCreateWebViewRequests;

	// MASTER
	MasterLoadListener* m_pMasterLoadListener;
	MasterViewListener* m_pMasterViewListener;

	// REGULAR
	LoadListener* m_pLoadListener;
	ViewListener* m_pViewListener;

	// LISTENERS NEXT, followed by JS handlers.
};

#endif