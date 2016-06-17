#include "cbase.h"

#include "c_webbrowser.h"
#include "c_anarchymanager.h"
#include <Awesomium/BitmapSurface.h>
#include "Filesystem.h"
//#include "aa_globals.h"
//#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_WebBrowser::C_WebBrowser()
{
	m_iState = 0;	// initialized
	m_pWebCore = null;
	m_pWebSession = null;
	m_pMasterWebView = null;
	m_pMasterLoadListener = null;
	m_pJSHandler = null;
}

C_WebBrowser::~C_WebBrowser()
{
	DevMsg("WebBrowser: destructor\n");
	WebCore::Shutdown();
}

void C_WebBrowser::Init()
{
	DevMsg("WebBrowser: Init\n");
	m_iState = 1;	// initializing

	using namespace Awesomium;

	WebConfig config;

	config.log_level = kLogLevel_Normal;
	config.child_process_path = WSLit("./AArcadeWebview.exe");

	m_pWebCore = WebCore::Initialize(config);

	// Create the master web view
	Awesomium::WebPreferences prefs;
	prefs.enable_plugins = true;
	prefs.enable_smooth_scrolling = true;
	prefs.user_stylesheet = WSLit("{}");// body{ background - color: #000000; }");

	m_pWebSession = m_pWebCore->CreateWebSession(WSLit(VarArgs("%s\\cache", engine->GetGameDirectory())), prefs);

	NewWindowDataSource* pNewWindowDataSource = new NewWindowDataSource();
	m_pWebSession->AddDataSource(WSLit("newwindow"), pNewWindowDataSource);

	g_pFullFileSystem->AddSearchPath(VarArgs("%s\\resource\\ui\\html", engine->GetGameDirectory()), "UI");
	UiDataSource* pUiDataSource = new UiDataSource();
	m_pWebSession->AddDataSource(WSLit("ui"), pUiDataSource);

	// MASTER
	m_pMasterLoadListener = new MasterLoadListener;
	m_pMasterViewListener = new MasterViewListener;

	// REGULAR
	m_pJSHandler = new JSHandler();
	m_pLoadListener = new LoadListener;
	m_pViewListener = new ViewListener;

	m_pMasterWebView = m_pWebCore->CreateWebView(g_pAnarchyManager->GetWebManager()->GetWebSurfaceWidth(), g_pAnarchyManager->GetWebManager()->GetWebSurfaceHeight(), m_pWebSession);
	m_pMasterWebView->set_load_listener(m_pMasterLoadListener);
	m_pMasterWebView->set_view_listener(m_pMasterViewListener);

	m_pMasterWebView->LoadURL(WebURL(WSLit("asset://newwindow/master")));
}

void C_WebBrowser::PrepareWebView(Awesomium::WebView* pWebView, std::string id)
{
	pWebView->Resize(g_pAnarchyManager->GetWebManager()->GetWebSurfaceWidth(), g_pAnarchyManager->GetWebManager()->GetWebSurfaceHeight());
	pWebView->set_load_listener(m_pLoadListener);
	pWebView->set_view_listener(m_pViewListener);

	if (id == "metaverse")
	{
		pWebView->set_js_method_handler(m_pJSHandler);
		CreateAaApi(pWebView);
	}
}

void C_WebBrowser::OnMasterWebViewDocumentReady()
{
	m_iState = 2;	// initialized
	g_pAnarchyManager->GetInputManager()->SetInputListener(g_pAnarchyManager->GetWebManager(), LISTENER_WEB_MANAGER);
	g_pAnarchyManager->GetWebManager()->OnBrowserInitialized();
}

void C_WebBrowser::CreateWebView(C_WebTab* pWebTab)
{
	DevMsg("WebBrowser: CreateWebView\n");

	std::string id = pWebTab->GetId();
	m_pMasterWebView->ExecuteJavascript(WSLit(VarArgs("window.open('asset://newwindow/%s', '', 'width=200,height=100');", id.c_str())), WSLit(""));
}

WebView* C_WebBrowser::FindWebView(C_WebTab* pWebTab)
{
	auto foundWebView = m_webViews.find(pWebTab);
	if (foundWebView != m_webViews.end())
		return m_webViews[pWebTab];
	else
		return null;
}

void C_WebBrowser::OnSelectWebTab(C_WebTab* pWebTab)
{
	WebView* pWebView = FindWebView(pWebTab);
	if (pWebView)
		pWebView->Focus();
}

void C_WebBrowser::OnDeselectWebTab(C_WebTab* pWebTab)
{
	WebView* pWebView = FindWebView(pWebTab);
	if (pWebView)
		pWebView->Unfocus();
}

void C_WebBrowser::OnMouseMove(C_WebTab* pWebTab, float fMouseX, float fMouseY)
{
	// the mouse position is between 0 and 1
	// translate the mouse position to actual pixel values
	int iMouseX = fMouseX * g_pAnarchyManager->GetWebManager()->GetWebSurfaceWidth();
	int iMouseY = fMouseY * g_pAnarchyManager->GetWebManager()->GetWebSurfaceHeight();

	WebView* pWebView = FindWebView(pWebTab);
	if ( pWebView )
		pWebView->InjectMouseMove(iMouseX, iMouseY);

	// FIXME: ALWAYS injecting into hud web tab.  this should only happen if the hud is active.
	C_WebTab* pHudWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();
	if (pHudWebTab && pHudWebTab != pWebTab)
	{
		WebView* pHudWebView = FindWebView(pHudWebTab);
		if (pHudWebView)
			pHudWebView->InjectMouseMove(iMouseX, iMouseY);
	}
}

void C_WebBrowser::OnMousePress(C_WebTab* pWebTab, vgui::MouseCode code)
{
	// translate the vgui::MouseCode into an Awesomium::MouseButton
	int iButtonId = -1;
	if (code == MOUSE_LEFT)
		iButtonId = 0;
	else if (code == MOUSE_MIDDLE)
		iButtonId = 1;
	else if (code == MOUSE_RIGHT)
		iButtonId = 2;

	WebView* pWebView = FindWebView(pWebTab);
	pWebView->InjectMouseDown((MouseButton)iButtonId);
}

void C_WebBrowser::OnMouseRelease(C_WebTab* pWebTab, vgui::MouseCode code)
{
	// translate the vgui::MouseCode into an Awesomium::MouseButton
	int iButtonId = -1;
	if (code == MOUSE_LEFT)
		iButtonId = 0;
	else if (code == MOUSE_MIDDLE)
		iButtonId = 1;
	else if (code == MOUSE_RIGHT)
		iButtonId = 2;

	WebView* pWebView = FindWebView(pWebTab);
	pWebView->InjectMouseUp((MouseButton)iButtonId);
}

void C_WebBrowser::CreateAaApi(WebView* pWebView)
{
	JSValue result = pWebView->CreateGlobalJavascriptObject(WSLit("aaapi"));
	if (!result.IsObject())
		return;

	JSObject& aaapiObject = result.ToObject();

	result = pWebView->CreateGlobalJavascriptObject(WSLit("aaapi.metaverse"));
	if (!result.IsObject())
		return;

	JSObject& metaverseObject = result.ToObject();
	metaverseObject.SetCustomMethod(WSLit("OnSelectItem"), false);
}

void C_WebBrowser::OnCreateWebViewDocumentReady(WebView* pWebView, std::string id)
{
	// The master webview has created a new webview on demand.
	DevMsg("WebBrowser: OnCreateWebViewDocumentReady: %s\n", id.c_str());

	// TODO: Add global JS API object to the web view.

	C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->FindWebTab(id);
	if (pWebTab)
	{
		pWebTab->SetState(2);
		m_webViews[pWebTab] = pWebView;

		/*
		if (pWebTab->GetId() == "metaverse")
		{
			// add or create the global aaapi object
			
		}
		*/

		//g_pAnarchyManager->GetMetaverseManager()->OnWebTabCreated(pWebTab);

		if (pWebTab->GetTexture()->GetImageFormat() == IMAGE_FORMAT_BGRA8888)
			pWebView->SetTransparent(true);

		pWebView->LoadURL(WebURL(WSLit(pWebTab->GetInitialUrl().c_str())));
	}
}

void C_WebBrowser::OnHudWebViewDocumentReady(WebView* pWebView, std::string id)
{
	C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->FindWebTab(id);
	if (g_pAnarchyManager->GetWebManager()->GetHudWebTab() == pWebTab)	// FIXME: THIS IS POSSIBLY A RACE CONDITION.  IF AWESOMIUM WORKS SUPER FAST, THEN THIS WILL ALWAYS BE FALSE. This is what causes the web tab on the main menu to be blank?
		g_pAnarchyManager->GetWebManager()->OnHudWebTabReady();
}

void C_WebBrowser::OnLoadingWebViewDocumentReady(WebView* pWebView, std::string id)
{
	C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->FindWebTab(id);
	if (g_pAnarchyManager->GetWebManager()->GetHudWebTab() == pWebTab)	// FIXME: THIS IS POSSIBLY A RACE CONDITION.  IF AWESOMIUM WORKS SUPER FAST, THEN THIS WILL ALWAYS BE FALSE. This is what causes the web tab on the main menu to be blank?
		g_pAnarchyManager->GetWebManager()->OnLoadingWebTabReady();
}

void C_WebBrowser::DispatchJavaScriptMethod(C_WebTab* pWebTab, std::string objectName, std::string objectMethod, std::vector<std::string> methodArguments)
{
	WebView* pWebView = m_webViews[pWebTab];

	JSValue response = pWebView->ExecuteJavascriptWithResult(WSLit(objectName.c_str()), WSLit(""));
	if (response.IsObject())
	{
		JSObject object = response.ToObject();
		JSArray arguments;

		for (auto argument : methodArguments)
			arguments.Push(WSLit(argument.c_str()));

		object.Invoke(WSLit(objectMethod.c_str()), arguments);
	}
}

/*
void C_WebBrowser::DispatchJavaScriptMethodBatch(C_WebTab* pWebTab, std::vector<MethodBatch_t*> batch)
{
	WebView* pWebView = m_webViews[pWebTab];

	JSValue response = pWebView->ExecuteJavascriptWithResult(WSLit(objectName.c_str()), WSLit(""));
	if (response.IsObject())
	{
		JSObject object = response.ToObject();
		JSArray arguments;

		for (auto eventArg : eventArgs)
		{
			JSObject eventObject;
			eventObject.SetProperty(WSLit("name"), WSLit(eventArg->name.c_str()));

			JSArray args;
			for (auto arg : eventArg->args)
				args.Push(WSLit(arg.c_str()));

			arguments.Push(eventObject);
		}

		object.Invoke(WSLit(objectMethod.c_str()), arguments);
	}
}
*/

void C_WebBrowser::Update()
{
	if (m_pWebCore)
		m_pWebCore->Update();
}

void C_WebBrowser::RegenerateTextureBits(C_WebTab* pWebTab, ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect)
{
	//DevMsg("WebBrowser: RegenerateTextureBits\n");
	WebView* pWebView = FindWebView(pWebTab);
	if (!pWebView)
		return;

	C_LibretroInstance* pLibretroInstance = null;
	LibretroInstanceInfo_t* info = null;

	bool bHasActiveLibretro = false;
	C_LibretroManager* pLibretroManager = g_pAnarchyManager->GetLibretroManager();
	if (pLibretroManager)
	{
		pLibretroInstance = pLibretroManager->GetSelectedLibretroInstance();
		if (pLibretroInstance)
		{
			if (pLibretroInstance)
			{
				info = g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance()->GetInfo();
				bHasActiveLibretro = true;
			}
		}
	}

	if (bHasActiveLibretro && g_pAnarchyManager->GetWebManager()->GetHudWebTab() != pWebTab && pLibretroInstance && info->state == 5)
			g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance()->CopyLastFrame(pVTFTexture->ImageData(0, 0, 0), pSubRect->width, pSubRect->height, pSubRect->width * 4, 4);
	else
	{
		BitmapSurface* surface = static_cast<BitmapSurface*>(pWebView->surface());
		if (surface != 0)
			surface->CopyTo(pVTFTexture->ImageData(0, 0, 0), pSubRect->width * 4, 4, false, false);
	}
}