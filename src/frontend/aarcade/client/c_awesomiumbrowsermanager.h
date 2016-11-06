#ifndef C_AWESOMIUMBROWSER_MANAGER_H
#define C_AWESOMIUMBROWSER_MANAGER_H

#include "c_awesomiumbrowserinstance.h"
#include <Awesomium/WebCore.h>
#include <Awesomium/STLHelpers.h>
#include <Awesomium/BitmapSurface.h>
#include "c_awesomiumlisteners.h"
#include "c_awesomiumjshandlers.h"
#include "c_awesomiumdatasources.h"
#include "c_awesomiumjshandlers.h"
#include "c_inputlistenerawesomiumbrowser.h"
//#include "c_libretrosurfaceregen.h"
#include <map>

class C_AwesomiumBrowserManager
{
public:
	C_AwesomiumBrowserManager();
	~C_AwesomiumBrowserManager();

	void Update();
	void CloseAllInstances(bool bDeleteHudAndImages = false);

	C_AwesomiumBrowserInstance* CreateAwesomiumBrowserInstance(std::string id = "", std::string initialURL = "", bool alpha = false);
	C_AwesomiumBrowserInstance* FindAwesomiumBrowserInstance(std::string id);
	C_AwesomiumBrowserInstance* FindAwesomiumBrowserInstance(Awesomium::WebView* pWebView);
	//C_AwesomiumBrowserInstance* FindAwesomiumBrowserInstance(IMaterial* pMaterial);

	void RunEmbeddedAwesomiumBrowser();
	void DestroyAwesomiumBrowserInstance(C_AwesomiumBrowserInstance* pInstance);

	bool FocusAwesomiumBrowserInstance(C_AwesomiumBrowserInstance* pAwesomiumBrowserInstance);
	bool SelectAwesomiumBrowserInstance(C_AwesomiumBrowserInstance* pAwesomiumBrowserInstance);

	// master
	void OnMasterWebViewDocumentReady();

	// regular
	void PrepareWebView(Awesomium::WebView* pWebView, std::string id);
	void OnCreateWebViewDocumentReady(Awesomium::WebView* pWebView, std::string id);
	void OnHudWebViewDocumentReady(Awesomium::WebView* pWebView, std::string id);

	/*
	C_SteamBrowserInstance* CreateSteamBrowserInstance();
	bool SelectSteamBrowserInstance(C_SteamBrowserInstance* pSteamBrowserInstance);
	void OnSteamBrowserInstanceCreated(C_SteamBrowserInstance* pSteamBrowserInstance);
	//C_SteamBrowserInstance* FindSteamBrowserInstance(CSysModule* pModule);
	C_SteamBrowserInstance* FindSteamBrowserInstance(std::string id);
	
	void RunEmbeddedSteamBrowser();
	void DestroySteamBrowserInstance(C_SteamBrowserInstance* pInstance);
	
	*/

	void CreateAaApi(WebView* pWebView);

	void DispatchJavaScriptMethod(C_AwesomiumBrowserInstance* pBrowserInstance, std::string objectName, std::string objectMethod, std::vector<std::string> methodArguments);
	void DispatchJavaScriptMethods(C_AwesomiumBrowserInstance* pBrowserInstance);
	
	// accessors
	C_AwesomiumBrowserInstance* GetFocusedAwesomiumBrowserInstance() { return m_pFocusedAwesomiumBrowserInstance; }
	C_AwesomiumBrowserInstance* GetSelectedAwesomiumBrowserInstance() { return m_pSelectedAwesomiumBrowserInstance; }
	C_InputListenerAwesomiumBrowser* GetInputListener() { return m_pInputListener; }

	// mutators	

private:
	C_InputListenerAwesomiumBrowser* m_pInputListener;

	Awesomium::WebCore* m_pWebCore;
	Awesomium::WebSession* m_pWebSession;
	Awesomium::WebView* m_pMasterWebView;

	MasterLoadListener* m_pMasterLoadListener;
	MasterViewListener* m_pMasterViewListener;

	LoadListener* m_pLoadListener;
	ViewListener* m_pViewListener;
	MenuListener* m_pMenuListener;

	JSHandler* m_pJSHandler;

	C_AwesomiumBrowserInstance* m_pFocusedAwesomiumBrowserInstance;
	C_AwesomiumBrowserInstance* m_pSelectedAwesomiumBrowserInstance;
	//std::map<C_AwesomiumBrowserInstance*, Awesomium::WebView*> m_webViews;
	std::map<std::string, C_AwesomiumBrowserInstance*> m_awesomiumBrowserInstances;

	/*
	bool m_bSoundEnabled;
	C_InputListenerSteamBrowser* m_pInputListener;
	C_SteamBrowserInstance* m_pSelectedSteamBrowserInstance;
	std::map<std::string, C_SteamBrowserInstance*> m_steamBrowserInstances;
	//std::map<uint, CSysModule*> m_steamBrowserInstancesModules;
	*/
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