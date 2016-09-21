#include "cbase.h"

//#include "aa_globals.h"
#include "c_awesomiumbrowsermanager.h"
#include "c_anarchymanager.h"
//#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_AwesomiumBrowserManager::C_AwesomiumBrowserManager()
{
	DevMsg("AwesomiumBrowserManager: Constructor\n");

	m_pWebCore = null;
	m_pWebSession = null;
	m_pMasterWebView = null;

	m_pMasterLoadListener = null;
	m_pMasterViewListener = null;

	m_pLoadListener = null;
	m_pViewListener = null;
	m_pMenuListener = null;

	m_pJSHandler = null;

	m_pInputListener = new C_InputListenerAwesomiumBrowser();
	m_pFocusedAwesomiumBrowserInstance = null;

	/*
	m_bSoundEnabled = true;
	m_pSelectedSteamBrowserInstance = null;

	steamapicontext->SteamHTMLSurface()->Init();

	m_pInputListener = new C_InputListenerSteamBrowser();

	//g_pAnarchyManager->SetState(AASTATE_WEBMANAGER);
	*/

//	DevMsg("WebBrowser: Init\n");
	//m_iState = 1;	// initializing


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

	g_pFullFileSystem->AddSearchPath(VarArgs("%s\\screenshots", engine->GetGameDirectory()), "SCREENSHOTS");
	ScreenshotDataSource* pScreenshotDataSource = new ScreenshotDataSource();
	m_pWebSession->AddDataSource(WSLit("screenshots"), pScreenshotDataSource);

	// MASTER
	m_pMasterLoadListener = new MasterLoadListener;
	m_pMasterViewListener = new MasterViewListener;

	// REGULAR
	m_pJSHandler = new JSHandler();

	m_pLoadListener = new LoadListener;
	m_pViewListener = new ViewListener;
	m_pMenuListener = new MenuListener;

//	m_pMasterWebView = m_pWebCore->CreateWebView(g_pAnarchyManager->GetWebManager()->GetWebSurfaceWidth(), g_pAnarchyManager->GetWebManager()->GetWebSurfaceHeight(), m_pWebSession);
	unsigned int width = 1280;
	unsigned int height = 720;
	m_pMasterWebView = m_pWebCore->CreateWebView(width, height, m_pWebSession);
	m_pMasterWebView->set_load_listener(m_pMasterLoadListener);
	m_pMasterWebView->set_view_listener(m_pMasterViewListener);

	m_pMasterWebView->LoadURL(WebURL(WSLit("asset://newwindow/master")));
	//m_pMasterWebView->LoadURL(WebURL(WSLit("http://www.smsithlord.com/")));
}

C_AwesomiumBrowserManager::~C_AwesomiumBrowserManager()
{
	DevMsg("AwesomiumBrowserManager: Destructor\n");

	// iterate over all web tabs and call their destructors
	for (auto it = m_awesomiumBrowserInstances.begin(); it != m_awesomiumBrowserInstances.end(); ++it)
	{
		C_AwesomiumBrowserInstance* pInstance = it->second;
		if (pInstance == m_pSelectedAwesomiumBrowserInstance)
		{
			this->SelectAwesomiumBrowserInstance(null);
			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
		}

		//std::string nameTest = "";
		//nameTest += pInstance->GetId();

		//DevMsg("Remove awesomium instance %s\n", nameTest.c_str());
//		if (pInstance->GetTexture() && g_pAnarchyManager->GetInputManager()->GetInputCanvasTexture() == pInstance->GetTexture())
		if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance)
		{
			g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
			//g_pAnarchyManager->GetInputManager()->SetInputListener(null);
			//g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(null);
		}

//		auto foundAwesomiumBrowserInstance = m_awesomiumBrowserInstances.find(pInstance->GetId());
//		if (foundAwesomiumBrowserInstance != m_awesomiumBrowserInstances.end())
//			m_awesomiumBrowserInstances.erase(foundAwesomiumBrowserInstance);

		pInstance->SelfDestruct();
	}

	m_awesomiumBrowserInstances.clear();

	m_pMasterWebView->Destroy();
	m_pMasterWebView = null;

	if ( m_pInputListener )
		delete m_pInputListener;

	WebCore::Shutdown();
}

void C_AwesomiumBrowserManager::RunEmbeddedAwesomiumBrowser()
{
	DevMsg("Run embedded awesomium test!\n");

//	C_AwesomiumBrowserInstance* pAwesomiumBrowserInstance = this->CreateAwesomiumBrowserInstance("", "http://smarcade.net/dlcv2/view_youtube.php?id=CmRih_VtVAs&autoplay=1", false);
	C_AwesomiumBrowserInstance* pAwesomiumBrowserInstance = this->CreateAwesomiumBrowserInstance("", "http://www.youtube.com/", false);
	pAwesomiumBrowserInstance->Select();
	//pAwesomiumBrowserInstance->Focus();
	// tell the input manager that the steam browser instance is active
	g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(pAwesomiumBrowserInstance);	// including an embedded instance in the activate input mode call overrides this
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pAwesomiumBrowserInstance);

//	C_InputListenerAwesomiumBrowser* pListener = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetInputListener();
//	g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(pAwesomiumBrowserInstance->GetTexture());
//	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, (C_InputListener*)pListener);

	//pAwesomiumBrowserInstance->Init();
//	g_pAnarchyManager->IncrementState();
}

void C_AwesomiumBrowserManager::DestroyAwesomiumBrowserInstance(C_AwesomiumBrowserInstance* pInstance)
{
	if (pInstance == m_pSelectedAwesomiumBrowserInstance)
	{
		this->SelectAwesomiumBrowserInstance(null);
		g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
	}

//	if (g_pAnarchyManager->GetInputManager()->GetInputCanvasTexture() == pInstance->GetTexture())
	if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance)
	{
		g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
		//g_pAnarchyManager->GetInputManager()->SetInputListener(null);
		//g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(null);
	}

	auto foundAwesomiumBrowserInstance = m_awesomiumBrowserInstances.find(pInstance->GetId());
	if (foundAwesomiumBrowserInstance != m_awesomiumBrowserInstances.end())
		m_awesomiumBrowserInstances.erase(foundAwesomiumBrowserInstance);

	pInstance->SelfDestruct();
}

void C_AwesomiumBrowserManager::OnMasterWebViewDocumentReady()
{
	g_pAnarchyManager->IncrementState();
	//m_iState = 2;	// initialized
	//	g_pAnarchyManager->GetInputManager()->SetInputListener(g_pAnarchyManager->GetWebManager(), LISTENER_WEB_MANAGER);
	//g_pAnarchyManager->GetWebManager()->OnBrowserInitialized();
}

C_AwesomiumBrowserInstance* C_AwesomiumBrowserManager::CreateAwesomiumBrowserInstance(std::string id, std::string initialURL, bool alpha)
{
	std::string goodId = (id != "") ? id : g_pAnarchyManager->GenerateUniqueId();

	C_AwesomiumBrowserInstance* pAwesomiumBrowserInstance = new C_AwesomiumBrowserInstance();
	pAwesomiumBrowserInstance->Init(goodId, initialURL, alpha);

	m_awesomiumBrowserInstances[goodId] = pAwesomiumBrowserInstance;

	DevMsg("AwesomiumBrowserManager: CreateWebView\n");
	m_pMasterWebView->ExecuteJavascript(WSLit(VarArgs("window.open('asset://newwindow/%s', '', 'width=200,height=100');", goodId.c_str())), WSLit(""));

	if (goodId != "hud")	// don't select the hud until its needed
		SelectAwesomiumBrowserInstance(pAwesomiumBrowserInstance);

	return pAwesomiumBrowserInstance;
}

C_AwesomiumBrowserInstance* C_AwesomiumBrowserManager::FindAwesomiumBrowserInstance(std::string id)
{
	auto foundBrowserInstance = m_awesomiumBrowserInstances.find(id);
	if (foundBrowserInstance != m_awesomiumBrowserInstances.end())
	{
		return foundBrowserInstance->second;
	}
	else
		return null;
}

C_AwesomiumBrowserInstance* C_AwesomiumBrowserManager::FindAwesomiumBrowserInstance(Awesomium::WebView* pWebView)
{
	// iterate over all web tabs and call their destructors
	for (auto it = m_awesomiumBrowserInstances.begin(); it != m_awesomiumBrowserInstances.end(); ++it)
	{
		if (it->second->GetWebView() == pWebView)
			return it->second;
	}

	return null;
}

void C_AwesomiumBrowserManager::PrepareWebView(Awesomium::WebView* pWebView, std::string id)
{
	unsigned int width = 1280;
	unsigned int height = 720;

	unsigned int imageWidth = 512;
	unsigned int imageHeight = 512;
	if (id == "images")
		pWebView->Resize(imageWidth, imageHeight);
	else
		pWebView->Resize(width, height);

	pWebView->set_load_listener(m_pLoadListener);
	pWebView->set_view_listener(m_pViewListener);
	pWebView->set_menu_listener(m_pMenuListener);

	if (id == "hud" || id == "images")
	{
		DevMsg("need to add JS API here!\n");
		pWebView->set_js_method_handler(m_pJSHandler);
		this->CreateAaApi(pWebView);
	}
}

void C_AwesomiumBrowserManager::CreateAaApi(WebView* pWebView)
{
	JSValue result = pWebView->CreateGlobalJavascriptObject(WSLit("aaapi"));
	if (!result.IsObject())
		return;

	JSObject& aaapiObject = result.ToObject();

	// SYSTEM
	result = pWebView->CreateGlobalJavascriptObject(WSLit("aaapi.system"));
	if (!result.IsObject())
		return;

	JSObject& systemObject = result.ToObject();
	systemObject.SetCustomMethod(WSLit("quit"), false);
	systemObject.SetCustomMethod(WSLit("launchItem"), false);
	systemObject.SetCustomMethod(WSLit("spawnItem"), false);
	systemObject.SetCustomMethod(WSLit("didSelectPopupMenuItem"), true);
	systemObject.SetCustomMethod(WSLit("didCancelPopupMenu"), true);
	systemObject.SetCustomMethod(WSLit("loadFirstLocalApp"), false);
	systemObject.SetCustomMethod(WSLit("loadNextLocalApp"), false);
	systemObject.SetCustomMethod(WSLit("loadLocalAppClose"), false);
	systemObject.SetCustomMethod(WSLit("detectAllMapScreenshots"), true);
	systemObject.SetCustomMethod(WSLit("getAllMapScreenshots"), true);
	systemObject.SetCustomMethod(WSLit("getAllMaps"), true);
	systemObject.SetCustomMethod(WSLit("loadMap"), false);
	systemObject.SetCustomMethod(WSLit("deactivateInputMode"), false);
	systemObject.SetCustomMethod(WSLit("forceInputMode"), false);
	systemObject.SetCustomMethod(WSLit("hudMouseDown"), false);
	systemObject.SetCustomMethod(WSLit("hudMouseUp"), false);
	systemObject.SetCustomMethod(WSLit("getSelectedWebTab"), true);
	systemObject.SetCustomMethod(WSLit("requestActivateInputMode"), false);
	systemObject.SetCustomMethod(WSLit("simpleImageReady"), false);
	systemObject.SetCustomMethod(WSLit("getMapInstances"), true);
	systemObject.SetCustomMethod(WSLit("spawnNearestObject"), false);
	systemObject.SetCustomMethod(WSLit("setNearestObjectDist"), false);

	// LIBRARY
	result = pWebView->CreateGlobalJavascriptObject(WSLit("aaapi.library"));
	if (!result.IsObject())
		return;

	JSObject& libraryObject = result.ToObject();
	libraryObject.SetCustomMethod(WSLit("getAllLibraryTypes"), true);
	libraryObject.SetCustomMethod(WSLit("getLibraryType"), true);
	libraryObject.SetCustomMethod(WSLit("getFirstLibraryItem"), true);
	libraryObject.SetCustomMethod(WSLit("getNextLibraryItem"), true);
	libraryObject.SetCustomMethod(WSLit("getLibraryItem"), true);
	libraryObject.SetCustomMethod(WSLit("getSelectedLibraryItem"), true);
	libraryObject.SetCustomMethod(WSLit("findFirstLibraryItem"), true);
	libraryObject.SetCustomMethod(WSLit("findNextLibraryItem"), true);
	libraryObject.SetCustomMethod(WSLit("findLibraryItem"), true);

	// CALLBACKS
	result = pWebView->CreateGlobalJavascriptObject(WSLit("aaapi.callbacks"));
	if (!result.IsObject())
		return;

	JSObject& callbacksObject = result.ToObject();
	callbacksObject.SetCustomMethod(WSLit("loadNextLocalAppCallback"), false);
	callbacksObject.SetCustomMethod(WSLit("mountNextWorkshopCallback"), false);
	callbacksObject.SetCustomMethod(WSLit("loadNextLocalItemLegacyCallback"), false);
	callbacksObject.SetCustomMethod(WSLit("detectNextMapCallback"), false);
	callbacksObject.SetCustomMethod(WSLit("spawnNextObjectCallback"), false);

	/*
	result = pWebView->CreateGlobalJavascriptObject(WSLit("aaapi.metaverse"));
	if (!result.IsObject())
	return;

	JSObject& metaverseObject = result.ToObject();
	metaverseObject.SetCustomMethod(WSLit("OnSelectItem"), false);
	*/
}

void C_AwesomiumBrowserManager::DispatchJavaScriptMethod(C_AwesomiumBrowserInstance* pBrowserInstance, std::string objectName, std::string objectMethod, std::vector<std::string> methodArguments)
{
	//WebView* pWebView = m_webViews[pWebTab];

	JSValue response = pBrowserInstance->GetWebView()->ExecuteJavascriptWithResult(WSLit(objectName.c_str()), WSLit(""));
	if (response.IsObject())
	{
		JSObject object = response.ToObject();
		JSArray arguments;

		for (auto argument : methodArguments)
			arguments.Push(WSLit(argument.c_str()));

		object.InvokeAsync(WSLit(objectMethod.c_str()), arguments);
	}

	//m_pWebBrowser->DispatchJavaScriptMethod(pWebTab, objectName, objectMethod, methodArguments);
	/*
	for (auto arg : args)
	{
	DevMsg("Argument: %s\n", arg->text.c_str());
	}
	*/
}

void C_AwesomiumBrowserManager::DispatchJavaScriptMethods(C_AwesomiumBrowserInstance* pBrowserInstance)
{
	//WebView* pWebView = m_webViews[pWebTab];

	std::string previousObjectName = "-1";

	JSValue response;
	JSObject responseObject;
	std::vector<JavaScriptMethodCall_t*>& methodCalls = pBrowserInstance->GetJavaScriptMethodCalls();
	for (auto pJavaScriptMethodCall : methodCalls)
	{
		if (previousObjectName != pJavaScriptMethodCall->objectName)
		{
			previousObjectName = pJavaScriptMethodCall->objectName;
			response = pBrowserInstance->GetWebView()->ExecuteJavascriptWithResult(WSLit(pJavaScriptMethodCall->objectName.c_str()), WSLit(""));
			if (!response.IsObject())
				continue;

			responseObject = response.ToObject();
		}

		JSArray arguments;
		for (auto argument : pJavaScriptMethodCall->methodArguments)
			arguments.Push(WSLit(argument.c_str()));

		responseObject.InvokeAsync(WSLit(pJavaScriptMethodCall->methodName.c_str()), arguments);
	}

	//m_pWebBrowser->DispatchJavaScriptMethods(pWebTab);
}

void C_AwesomiumBrowserManager::OnCreateWebViewDocumentReady(WebView* pWebView, std::string id)
{
	// The master webview has created a new webview on demand.
	DevMsg("AwesomiumBrowserManager: OnCreateWebViewDocumentReady: %s\n", id.c_str());

	// TODO: Add global JS API object to the web view.

	//C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->FindWebTab(id);
	C_AwesomiumBrowserInstance* pBrowserInstance = this->FindAwesomiumBrowserInstance(id);
	if (pBrowserInstance)
	{
		pBrowserInstance->SetWebView(pWebView);
		pBrowserInstance->SetState(2);
		//m_webViews[pBrowserInstance] = pWebView;	// obsolete perhaps??
		
		ITexture* pTexture = pBrowserInstance->GetTexture();
		if (pTexture && pTexture->GetImageFormat() == IMAGE_FORMAT_BGRA8888)
			pWebView->SetTransparent(true);

		std::string uri = (id == "images") ? "asset://ui/imageLoader.html" : pBrowserInstance->GetInitialURL();

		pWebView->LoadURL(WebURL(WSLit(uri.c_str())));
		DevMsg("Loading initial URL: %s\n", uri.c_str());

		if (id == "hud" )	// is this too early??
			g_pAnarchyManager->IncrementState();
		else if (id == "images" && AASTATE_AWESOMIUMBROWSERMANAGERIMAGESWAIT)
			g_pAnarchyManager->IncrementState();
	}
}

void C_AwesomiumBrowserManager::OnHudWebViewDocumentReady(WebView* pWebView, std::string id)
{
	DevMsg("AwesomiumBrowserManager: OnHudWebViewDocumentReady: %s\n", id.c_str());
//	C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();
	C_AwesomiumBrowserInstance* pBrowserInstance = this->FindAwesomiumBrowserInstance(id);
	if (pBrowserInstance)
	{
		pBrowserInstance->SetWebView(pWebView);
		pBrowserInstance->SetState(2);
		//m_webViews[pBrowserInstance] = pWebView;

		ITexture* pTexture = pBrowserInstance->GetTexture();
		if (pTexture && pTexture->GetImageFormat() == IMAGE_FORMAT_BGRA8888)
			pWebView->SetTransparent(true);

		// need to wait longer

		//	if (g_pAnarchyManager->GetWebManager()->GetHudWebTab() == pWebTab)	// FIXME: THIS IS POSSIBLY A RACE CONDITION.  IF AWESOMIUM WORKS SUPER FAST, THEN THIS WILL ALWAYS BE FALSE. This is what causes the web tab on the main menu to be blank?????
		//g_pAnarchyManager->GetWebManager()->OnHudWebTabReady();
//		g_pAnarchyManager->IncrementState();

		std::string initialURL = pBrowserInstance->GetInitialURL();
		std::string uri = (initialURL == "") ? "asset://ui/blank.html" : initialURL;

		pWebView->LoadURL(WebURL(WSLit(uri.c_str())));

//		if (id == "hud")	// FIXME: Should wait until the hud loads its 1st page before moving on.
			g_pAnarchyManager->IncrementState();
	}






	/*


	// The master webview has created a new webview on demand.
	DevMsg("AwesomiumBrowserManager: OnCreateWebViewDocumentReady: %s\n", id.c_str());

	// TODO: Add global JS API object to the web view.

	//C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->FindWebTab(id);
	C_AwesomiumBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance(id);
	if (pBrowserInstance)
	{
		//pBrowserInstance->SetState(2);
		m_webViews[pBrowserInstance] = pWebView;

		ITexture* pTexture = pBrowserInstance->GetTexture();
		if (pTexture && pTexture->GetImageFormat() == IMAGE_FORMAT_BGRA8888)
			pWebView->SetTransparent(true);

		std::string uri = (id == "images") ? "asset://ui/imageLoader.html" : pBrowserInstance->GetInitialURL();

		pWebView->LoadURL(WebURL(WSLit(uri.c_str())));
		DevMsg("Loading initial URL: %s\n", uri.c_str());

		if (id == "hud")
			g_pAnarchyManager->IncrementState();
	}
	*/
}

void C_AwesomiumBrowserManager::Update()
{
	if (m_pWebCore)
		m_pWebCore->Update();
	/*
	for (auto it = m_awesomiumBrowserInstances.begin(); it != m_awesomiumBrowserInstances.end(); ++it)
	{
		C_AwesomiumBrowserInstance* pAwesomiumBrowserInstance = it->second;
		pAwesomiumBrowserInstance->Update();
	}
	*/

	for (auto it = m_awesomiumBrowserInstances.begin(); it != m_awesomiumBrowserInstances.end(); ++it)
	{
		if (g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(it->second))
			it->second->Update();
	}

//	if (m_pSelectedAwesomiumBrowserInstance)	// don't need
	//	m_pSelectedAwesomiumBrowserInstance->Update();


	

	//DevMsg("SteamBrowserManager: Update\n");
	//info->state = state;
//	if (m_pSelectedSteamBrowserInstance)
//		m_pSelectedSteamBrowserInstance->Update();
}
/*
C_AwesomiumBrowserInstance* C_AwesomiumBrowserManager::CreateAwesomiumBrowserInstance()
{
	C_SteamBrowserInstance* pSteamBrowserInstance = new C_SteamBrowserInstance();
	SelectSteamBrowserInstance(pSteamBrowserInstance);
	return pSteamBrowserInstance;
}
*/
bool C_AwesomiumBrowserManager::FocusAwesomiumBrowserInstance(C_AwesomiumBrowserInstance* pAwesomiumBrowserInstance)
{
	m_pFocusedAwesomiumBrowserInstance = pAwesomiumBrowserInstance;
	return true;
}

bool C_AwesomiumBrowserManager::SelectAwesomiumBrowserInstance(C_AwesomiumBrowserInstance* pAwesomiumBrowserInstance)
{
	m_pSelectedAwesomiumBrowserInstance = pAwesomiumBrowserInstance;
	return true;
}
/*
void C_SteamBrowserManager::OnSteamBrowserInstanceCreated(C_SteamBrowserInstance* pSteamBrowserInstance)
{
	std::string id = pSteamBrowserInstance->GetId();
	m_steamBrowserInstances[id] = pSteamBrowserInstance;

	//pSteamBrowserInstance->GetInfo()->state = 1;
}

C_SteamBrowserInstance* C_SteamBrowserManager::FindSteamBrowserInstance(std::string id)
{
	auto foundSteamBrowserInstance = m_steamBrowserInstances.find(id);
	if (foundSteamBrowserInstance != m_steamBrowserInstances.end())
	{
		return foundSteamBrowserInstance->second;
			//return m_steamBrowserInstances[foundSteamBrowserInstance];
	}
	else
		return null;
}


void C_SteamBrowserManager::RunEmbeddedSteamBrowser()
{
	C_SteamBrowserInstance* pSteamBrowserInstance = this->CreateSteamBrowserInstance();
	pSteamBrowserInstance->Init("", "https://www.youtube.com/watch?v=0s4LADs8QnE", null);

	// http://anarchyarcade.com/press.html
	// https://www.youtube.com/html5
	// http://smarcade.net/dlcv2/view_youtube.php?id=CmRih_VtVAs&autoplay=1
}

void C_SteamBrowserManager::DestroySteamBrowserInstance(C_SteamBrowserInstance* pInstance)
{
	if (pInstance == m_pSelectedSteamBrowserInstance)
	{
		this->SelectSteamBrowserInstance(null);
		g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
	}

	if (g_pAnarchyManager->GetInputManager()->GetInputCanvasTexture() == pInstance->GetTexture())
	{
		g_pAnarchyManager->GetInputManager()->SetInputListener(null);
		g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(null);
	}

	auto foundSteamBrowserInstance = m_steamBrowserInstances.find(pInstance->GetId());
	if (foundSteamBrowserInstance != m_steamBrowserInstances.end())
		m_steamBrowserInstances.erase(foundSteamBrowserInstance);

	pInstance->SelfDestruct();
}
*/