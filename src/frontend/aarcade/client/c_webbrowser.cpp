#include "cbase.h"

#include "c_webbrowser.h"
#include "c_anarchymanager.h"
#include <Awesomium/BitmapSurface.h>
#include "Filesystem.h"
//#include "vgui_controls/KeyBindingMap.h"
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
	m_pMasterViewListener = null;

	m_pLoadListener = null;
	m_pViewListener = null;
	m_pMenuListener = null;
	m_pJSHandler = null;
}

C_WebBrowser::~C_WebBrowser()
{
	DevMsg("WebBrowser: destructor\n");
	WebCore::Shutdown();

	//delete pNewWindowDataSource
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

	//g_pFullFileSystem->AddSearchPath(VarArgs("%s\\screenshots", engine->GetGameDirectory()), "SCREENSHOTS");
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
	pWebView->set_menu_listener(m_pMenuListener);

	if (id == "hud")
	{
		pWebView->set_js_method_handler(m_pJSHandler);
		CreateAaApi(pWebView);
	}
}

void C_WebBrowser::OnMasterWebViewDocumentReady()
{
	m_iState = 2;	// initialized
//	g_pAnarchyManager->GetInputManager()->SetInputListener(g_pAnarchyManager->GetWebManager(), LISTENER_WEB_MANAGER);
	g_pAnarchyManager->GetWebManager()->OnBrowserInitialized();
}

void C_WebBrowser::ReleaseWebView(C_WebTab* pWebTab)
{
	WebView* pWebView = this->FindWebView(pWebTab);
	if (!pWebView)
		return;

	pWebView->Unfocus();
	pWebView->Destroy();
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

C_WebTab* C_WebBrowser::FindWebTab(WebView* pWebView)
{
	std::map<C_WebTab*, WebView*>::iterator it = m_webViews.begin();
	while (it != m_webViews.end())
	{
		if (it->second == pWebView)
			return it->first;

		it++;
	}

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

	// inject mouse movement into the HUD too, if its active.
	C_WebTab* pHudWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();
	if (pHudWebTab && pHudWebTab != pWebTab && g_pAnarchyManager->GetInputManager()->GetInputMode())
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
	//WebView* pWebView = FindWebView(g_pAnarchyManager->GetWebManager()->GetHudWebTab());
	if ( pWebView )
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
	//WebView* pWebView = FindWebView(g_pAnarchyManager->GetWebManager()->GetHudWebTab());
	if ( pWebView )
		pWebView->InjectMouseUp((MouseButton)iButtonId);
}

void C_WebBrowser::OnKeyCodePressed(C_WebTab* pWebTab, vgui::MouseCode code, bool shift, bool ctrl, bool alt)
{
	//using namespace vgui;
	//using namespace Awesomium;

	WebKeyboardEvent pWebKeyboardEvent;
	pWebKeyboardEvent.type = WebKeyboardEvent::kTypeKeyDown;

	pWebKeyboardEvent.modifiers = 0;
	/*
	bool shift = (input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT));
	bool ctrl = (input()->IsKeyDown(KEY_LCONTROL) || input()->IsKeyDown(KEY_RCONTROL));
	bool alt = (input()->IsKeyDown(KEY_LALT) || input()->IsKeyDown(KEY_RALT));
	*/
	
	// to avoid include errors with #include "vgui_controls/KeyBindingMap.h"
	enum
	{
		MODIFIER_SHIFT = (1 << 0),
		MODIFIER_CONTROL = (1 << 1),
		MODIFIER_ALT = (1 << 2),
	};

	if (shift)
		pWebKeyboardEvent.modifiers |= MODIFIER_SHIFT;

	if (ctrl)
		pWebKeyboardEvent.modifiers |= MODIFIER_CONTROL;

	if (alt)
		pWebKeyboardEvent.modifiers |= MODIFIER_ALT;

	int virtualKeyCode = KeyCodes::AK_UNKNOWN;
	std::string actualCharOutput = "";

	switch (code)
	{
	case KEY_0:
		virtualKeyCode = 0x30;
		if (shift)
			actualCharOutput = ')';
		else
			actualCharOutput = '0';
		break;

	case KEY_1:
		virtualKeyCode = 0x31;
		if (shift)
			actualCharOutput = '!';
		else
			actualCharOutput = '1';
		break;

	case KEY_2:
		virtualKeyCode = 0x32;
		if (shift)
			actualCharOutput = '@';
		else
			actualCharOutput = '2';
		break;

	case KEY_3:
		virtualKeyCode = 0x33;
		if (shift)
			actualCharOutput = '#';
		else
			actualCharOutput = '3';
		break;

	case KEY_4:
		virtualKeyCode = 0x34;
		if (shift)
			actualCharOutput = '$';
		else
			actualCharOutput = '4';
		break;

	case KEY_5:
		virtualKeyCode = 0x35;
		if (shift)
			actualCharOutput = '%';
		else
			actualCharOutput = '5';
		break;

	case KEY_6:
		virtualKeyCode = 0x36;
		if (shift)
			actualCharOutput = '^';
		else
			actualCharOutput = '6';
		break;

	case KEY_7:
		virtualKeyCode = 0x37;
		if (shift)
			actualCharOutput = '&';
		else
			actualCharOutput = '7';
		break;

	case KEY_8:
		virtualKeyCode = 0x38;
		if (shift)
			actualCharOutput = '*';
		else
			actualCharOutput = '8';
		break;

	case KEY_9:
		virtualKeyCode = 0x39;
		if (shift)
			actualCharOutput = '(';
		else
			actualCharOutput = '9';
		break;

	case KEY_A:
		virtualKeyCode = 0x41;
		if (shift)
			actualCharOutput = 'A';
		else
			actualCharOutput = 'a';
		break;

	case KEY_B:
		virtualKeyCode = 0x42;
		if (shift)
			actualCharOutput = 'B';
		else
			actualCharOutput = 'b';
		break;

	case KEY_C:
		virtualKeyCode = 0x43;
		if (shift)
			actualCharOutput = 'C';
		else
			actualCharOutput = 'c';
		break;

	case KEY_D:
		virtualKeyCode = 0x44;
		if (shift)
			actualCharOutput = 'D';
		else
			actualCharOutput = 'd';
		break;

	case KEY_E:
		virtualKeyCode = 0x45;
		if (shift)
			actualCharOutput = 'E';
		else
			actualCharOutput = 'e';
		break;

	case KEY_F:
		virtualKeyCode = 0x46;
		if (shift)
			actualCharOutput = 'F';
		else
			actualCharOutput = 'f';
		break;

	case KEY_G:
		virtualKeyCode = 0x47;
		if (shift)
			actualCharOutput = 'G';
		else
			actualCharOutput = 'g';
		break;

	case KEY_H:
		virtualKeyCode = 0x48;
		if (shift)
			actualCharOutput = 'H';
		else
			actualCharOutput = 'h';
		break;

	case KEY_I:
		virtualKeyCode = 0x49;
		if (shift)
			actualCharOutput = 'I';
		else
			actualCharOutput = 'i';
		break;

	case KEY_J:
		virtualKeyCode = 0x4A;
		if (shift)
			actualCharOutput = 'J';
		else
			actualCharOutput = 'j';
		break;

	case KEY_K:
		virtualKeyCode = 0x4B;
		if (shift)
			actualCharOutput = 'K';
		else
			actualCharOutput = 'k';
		break;

	case KEY_L:
		virtualKeyCode = 0x4C;
		if (shift)
			actualCharOutput = 'L';
		else
			actualCharOutput = 'l';
		break;

	case KEY_M:
		virtualKeyCode = 0x4D;
		if (shift)
			actualCharOutput = 'M';
		else
			actualCharOutput = 'm';
		break;

	case KEY_N:
		virtualKeyCode = 0x4E;
		if (shift)
			actualCharOutput = 'N';
		else
			actualCharOutput = 'n';
		break;

	case KEY_O:
		virtualKeyCode = 0x4F;
		if (shift)
			actualCharOutput = 'O';
		else
			actualCharOutput = 'o';
		break;

	case KEY_P:
		virtualKeyCode = 0x50;
		if (shift)
			actualCharOutput = 'P';
		else
			actualCharOutput = 'p';
		break;

	case KEY_Q:
		virtualKeyCode = 0x51;
		if (shift)
			actualCharOutput = 'Q';
		else
			actualCharOutput = 'q';
		break;

	case KEY_R:
		virtualKeyCode = 0x52;
		if (shift)
			actualCharOutput = 'R';
		else
			actualCharOutput = 'r';
		break;

	case KEY_S:
		virtualKeyCode = 0x53;
		if (shift)
			actualCharOutput = 'S';
		else
			actualCharOutput = 's';
		break;

	case KEY_T:
		virtualKeyCode = 0x54;
		if (shift)
			actualCharOutput = 'T';
		else
			actualCharOutput = 't';
		break;

	case KEY_U:
		virtualKeyCode = 0x55;
		if (shift)
			actualCharOutput = 'U';
		else
			actualCharOutput = 'u';
		break;

	case KEY_V:
		virtualKeyCode = 0x56;
		if (shift)
			actualCharOutput = 'V';
		else
			actualCharOutput = 'v';
		break;

	case KEY_W:
		virtualKeyCode = 0x57;
		if (shift)
			actualCharOutput = 'W';
		else
			actualCharOutput = 'w';
		break;

	case KEY_X:
		virtualKeyCode = 0x58;
		if (shift)
			actualCharOutput = 'X';
		else
			actualCharOutput = 'x';
		break;

	case KEY_Y:
		virtualKeyCode = 0x59;
		if (shift)
			actualCharOutput = 'Y';
		else
			actualCharOutput = 'y';
		break;

	case KEY_Z:
		virtualKeyCode = 0x5A;
		if (shift)
			actualCharOutput = 'Z';
		else
			actualCharOutput = 'z';
		break;

	case KEY_PAD_0:
		virtualKeyCode = 0x60;
		if (!shift)
			actualCharOutput = '0';
		break;

	case KEY_PAD_1:
		virtualKeyCode = 0x61;
		actualCharOutput = '1';
		break;

	case KEY_PAD_2:
		virtualKeyCode = 0x62;
		actualCharOutput = '2';
		break;

	case KEY_PAD_3:
		virtualKeyCode = 0x63;
		actualCharOutput = '3';
		break;

	case KEY_PAD_4:
		virtualKeyCode = 0x64;
		actualCharOutput = '4';
		break;

	case KEY_PAD_5:
		virtualKeyCode = 0x65;
		actualCharOutput = '5';
		break;

	case KEY_PAD_6:
		virtualKeyCode = 0x66;
		actualCharOutput = '6';
		break;

	case KEY_PAD_7:
		virtualKeyCode = 0x67;
		actualCharOutput = '7';
		break;

	case KEY_PAD_8:
		virtualKeyCode = 0x68;
		actualCharOutput = '8';
		break;

	case KEY_PAD_9:
		virtualKeyCode = 0x69;
		actualCharOutput = '9';
		break;

	case KEY_PAD_DIVIDE:
		virtualKeyCode = 0x6F;
		actualCharOutput = '/';
		break;

	case KEY_PAD_MULTIPLY:
		virtualKeyCode = 0x6A;
		actualCharOutput = '*';
		break;

	case KEY_PAD_MINUS:
		virtualKeyCode = 0x6D;
		actualCharOutput = '-';
		break;

	case KEY_PAD_PLUS:
		virtualKeyCode = 0x6B;
		actualCharOutput = '+';
		break;

	case KEY_PAD_ENTER:
		virtualKeyCode = 0x0D;
		actualCharOutput = '\r';
		break;

	case KEY_PAD_DECIMAL:
		virtualKeyCode = 0x6E;
		actualCharOutput = '.';
		break;

	case KEY_LBRACKET:
		virtualKeyCode = 0xDB;
		if (shift)
			actualCharOutput = '{';
		else
			actualCharOutput = '[';
		break;

	case KEY_RBRACKET:
		virtualKeyCode = 0xDD;
		if (shift)
			actualCharOutput = '}';
		else
			actualCharOutput = ']';
		break;

	case KEY_SEMICOLON:
		virtualKeyCode = 0xBA;
		if (shift)
			actualCharOutput = ':';
		else
			actualCharOutput = ';';
		break;

	case KEY_APOSTROPHE:
		virtualKeyCode = 0xDE;
		if (shift)
			actualCharOutput = '"';
		else
			actualCharOutput = '\'';
		break;

	case KEY_BACKQUOTE:
		virtualKeyCode = 0xC0;
		if (shift)
			actualCharOutput = '~';
		else
			actualCharOutput = '`';
		break;

	case KEY_COMMA:
		virtualKeyCode = 0xBC;
		if (shift)
			actualCharOutput = '<';
		else
			actualCharOutput = ',';
		break;

	case KEY_PERIOD:
		virtualKeyCode = 0xBE;
		if (shift)
			actualCharOutput = '>';
		else
			actualCharOutput = '.';
		break;

	case KEY_SLASH:
		virtualKeyCode = 0xBF;
		if (shift)
			actualCharOutput = '?';
		else
			actualCharOutput = '/';
		break;

	case KEY_BACKSLASH:
		virtualKeyCode = 0xDC;
		if (shift)
			actualCharOutput = '|';
		else
			actualCharOutput = '\\';
		break;

	case KEY_MINUS:
		virtualKeyCode = 0xBD;
		if (shift)
			actualCharOutput = '_';
		else
			actualCharOutput = '-';
		break;

	case KEY_EQUAL:
		virtualKeyCode = 0xBB;
		if (shift)
			actualCharOutput = '+';
		else
			actualCharOutput = '=';
		break;

	case KEY_ENTER:
		virtualKeyCode = 0x0D;
		actualCharOutput = '\r';
		break;

	case KEY_SPACE:
		virtualKeyCode = 0x20;
		actualCharOutput = ' ';
		break;

	case KEY_BACKSPACE:
		virtualKeyCode = 0x08;
		break;

	case KEY_TAB:
		virtualKeyCode = 0x09;
		break;

	case KEY_CAPSLOCK:
		virtualKeyCode = 0x14;
		break;

	case KEY_NUMLOCK:
		virtualKeyCode = 0x90;
		break;

	case KEY_ESCAPE:
		virtualKeyCode = 0x1B;
		break;

	case KEY_SCROLLLOCK:
		virtualKeyCode = 0x91;
		break;

	case KEY_INSERT:
		virtualKeyCode = 0x2D;
		break;

	case KEY_DELETE:
		virtualKeyCode = 0x2E;
		break;

	case KEY_HOME:
		virtualKeyCode = 0x24;
		break;

	case KEY_END:
		virtualKeyCode = 0x23;
		break;

	case KEY_PAGEUP:
		virtualKeyCode = 0x21;
		break;

	case KEY_PAGEDOWN:
		virtualKeyCode = 0x22;
		break;

	case KEY_LSHIFT:
		virtualKeyCode = 0xA0;
		break;

	case KEY_RSHIFT:
		virtualKeyCode = 0xA1;
		break;

	case KEY_LALT:
		virtualKeyCode = 0x12;
		break;

	case KEY_RALT:
		virtualKeyCode = 0x12;
		break;

	case KEY_LCONTROL:
		virtualKeyCode = 0xA2;
		break;

	case KEY_RCONTROL:
		virtualKeyCode = 0xA3;
		break;

	case KEY_LWIN:
		virtualKeyCode = 0x5B;
		break;

	case KEY_RWIN:
		virtualKeyCode = 0x5C;
		break;

	case KEY_APP:
		virtualKeyCode = 0x5D;
		break;

	case KEY_UP:
		virtualKeyCode = 0x26;
		break;

	case KEY_LEFT:
		virtualKeyCode = 0x25;
		break;

	case KEY_DOWN:
		virtualKeyCode = 0x28;
		break;

	case KEY_RIGHT:
		virtualKeyCode = 0x27;
		break;
	}

	char outputChar = actualCharOutput[0];

	char* buf = new char[20];
	pWebKeyboardEvent.virtual_key_code = virtualKeyCode;
	GetKeyIdentifierFromVirtualKeyCode(pWebKeyboardEvent.virtual_key_code, &buf);
	strcpy(pWebKeyboardEvent.key_identifier, buf);
	delete[] buf;

	pWebKeyboardEvent.native_key_code = pWebKeyboardEvent.virtual_key_code;

	bool hasChar = false;

	// If this key generates text output...
	if (actualCharOutput != "")
		hasChar = true;

	if (hasChar)
	{
		pWebKeyboardEvent.text[0] = outputChar;
		pWebKeyboardEvent.unmodified_text[0] = outputChar;
	}
	else
	{
		pWebKeyboardEvent.text[0] = null;
		pWebKeyboardEvent.unmodified_text[0] = null;
	}

	WebView* pWebView = FindWebView(pWebTab);
	if (pWebView)
		pWebView->InjectKeyboardEvent(pWebKeyboardEvent);

	// If this key has text output, we gotta send a char msg too
	if (hasChar)
	{
		pWebKeyboardEvent.type = WebKeyboardEvent::kTypeChar;

		pWebKeyboardEvent.virtual_key_code = virtualKeyCode;
		pWebKeyboardEvent.native_key_code = virtualKeyCode;

		pWebView->InjectKeyboardEvent(pWebKeyboardEvent);
	}
}

void C_WebBrowser::OnKeyCodeReleased(C_WebTab* pWebTab, vgui::MouseCode code, bool shift, bool ctrl, bool alt)
{
	WebKeyboardEvent pWebKeyboardEvent;
	pWebKeyboardEvent.type = WebKeyboardEvent::kTypeKeyUp;

	char* buf = new char[20];
	pWebKeyboardEvent.virtual_key_code = 0x42;
	GetKeyIdentifierFromVirtualKeyCode(pWebKeyboardEvent.virtual_key_code, &buf);
	strcpy(pWebKeyboardEvent.key_identifier, buf);
	delete[] buf;

	pWebKeyboardEvent.modifiers = 0;

	pWebKeyboardEvent.native_key_code = pWebKeyboardEvent.virtual_key_code;

	WebView* pWebView = FindWebView(pWebTab);
	if (pWebView)
		pWebView->InjectKeyboardEvent(pWebKeyboardEvent);
}

void C_WebBrowser::CreateAaApi(WebView* pWebView)
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

	/*
	result = pWebView->CreateGlobalJavascriptObject(WSLit("aaapi.metaverse"));
	if (!result.IsObject())
		return;

	JSObject& metaverseObject = result.ToObject();
	metaverseObject.SetCustomMethod(WSLit("OnSelectItem"), false);
	*/
}

void C_WebBrowser::RemoveWebView(C_WebTab* pWebTab)
{
	// FIXME: Blindly unfocusing because the web tab doesn't remember if its focused or not!!
	g_pAnarchyManager->GetWebManager()->DeselectWebTab(pWebTab);
	g_pAnarchyManager->GetInputManager()->DeactivateInputMode();

	std::map<C_WebTab*, WebView*>::iterator it = m_webViews.find(pWebTab);
	if (it != m_webViews.end())
	{
		WebView* pWebView = m_webViews[pWebTab];
		pWebView->Destroy();
		m_webViews.erase(it);
	}
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
		DevMsg("Loading initial URL: %s\n", pWebTab->GetInitialUrl().c_str());
	}
}

void C_WebBrowser::OnHudWebViewDocumentReady(WebView* pWebView, std::string id)
{
	//C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->FindWebTab(id);
	C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();

	pWebTab->SetState(2);
	m_webViews[pWebTab] = pWebView;

//	if (g_pAnarchyManager->GetWebManager()->GetHudWebTab() == pWebTab)	// FIXME: THIS IS POSSIBLY A RACE CONDITION.  IF AWESOMIUM WORKS SUPER FAST, THEN THIS WILL ALWAYS BE FALSE. This is what causes the web tab on the main menu to be blank?????
	g_pAnarchyManager->GetWebManager()->OnHudWebTabReady();
}

void C_WebBrowser::OnLoadingWebViewDocumentReady(WebView* pWebView, std::string id)
{
	/*
	C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->FindWebTab(id);
	if (g_pAnarchyManager->GetWebManager()->GetHudWebTab() == pWebTab)	// FIXME: THIS IS POSSIBLY A RACE CONDITION.  IF AWESOMIUM WORKS SUPER FAST, THEN THIS WILL ALWAYS BE FALSE. This is what causes the web tab on the main menu to be blank?
		g_pAnarchyManager->GetWebManager()->OnLoadingWebTabReady();
	else
		DevMsg("Wait\n\n\n\n\nERROR ERROR ERROR\n");
	*/
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

		object.InvokeAsync(WSLit(objectMethod.c_str()), arguments);
	}
}

void C_WebBrowser::DispatchJavaScriptMethods(C_WebTab* pWebTab)
{
	WebView* pWebView = m_webViews[pWebTab];

	std::string previousObjectName = "-1";

	JSValue response;
	JSObject responseObject;
	std::vector<JavaScriptMethodCall_t*>& methodCalls = pWebTab->GetJavaScriptMethodCalls();
	for (auto pJavaScriptMethodCall : methodCalls)
	{
		if (previousObjectName != pJavaScriptMethodCall->objectName)
		{
			previousObjectName = pJavaScriptMethodCall->objectName;
			response = pWebView->ExecuteJavascriptWithResult(WSLit(pJavaScriptMethodCall->objectName.c_str()), WSLit(""));
			if (!response.IsObject())
				continue;

			responseObject = response.ToObject();
		}

		JSArray arguments;
		for (auto argument : pJavaScriptMethodCall->methodArguments)
			arguments.Push(WSLit(argument.c_str()));

		responseObject.InvokeAsync(WSLit(pJavaScriptMethodCall->methodName.c_str()), arguments);
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