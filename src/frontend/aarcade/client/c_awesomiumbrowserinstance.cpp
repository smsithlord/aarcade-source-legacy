// ;..\..\portaudio\lib\portaudio_x86.lib

#include "cbase.h"
#include "aa_globals.h"

//#include "aa_globals.h"
#include "c_awesomiumbrowserinstance.h"
#include "c_anarchymanager.h"
#include "../../../public/vgui_controls/Controls.h"
#include "vgui/IInput.h"
#include "c_canvasregen.h"
#include "c_embeddedinstance.h"
//#include <mutex>
//#include "vgui_controls/Controls.h"
//#include "vgui/IVGUI.h"
//#include "vgui/IInput.h"
//#include "vgui/ISystem.h"
//#include "vgui_controls/Controls.h"
//#include "vgui_controls/KeyBindingMap.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_AwesomiumBrowserInstance::C_AwesomiumBrowserInstance()
{
	m_iNumImagesLoading = -1;
	m_iMaxImagesLoading = 10;

	DevMsg("AwesomiumBrowserInstance: Constructor\n");
	m_id = "";
	m_bAlpha = false;
	m_pTexture = null;
	m_iLastRenderedFrame = -1;
	m_pLastFrameData = null;
	m_bReadyForNextFrame = true;
	m_bCopyingFrame = false;
	m_bReadyToCopyFrame = false;
//	m_pPostData = null;
	m_initialURL = "";
	m_bIsDirty = false;
	m_pWebView = null;
	m_iState = 1;	// initializing
	m_iLastVisibleFrame = -1;
}

C_AwesomiumBrowserInstance::~C_AwesomiumBrowserInstance()
{
	DevMsg("AwesomiumBrowserInstance: Destructor\n");
	
	if (m_pTexture)
	{
		m_pTexture->SetTextureRegenerator(null);

		m_pTexture->DecrementReferenceCount();	// FIXME: this is hte next statement to execute on an exit crash after checkin out a level...
		m_pTexture->DeleteIfUnreferenced();
		m_pTexture = null;
	}
}

void C_AwesomiumBrowserInstance::SelfDestruct()
{
	DevMsg("AwesomiumBrowserInstance: SelfDestruct\n");
	m_pWebView->Destroy();
	//steamapicontext->SteamHTMLSurface()->RemoveBrowser(m_unBrowserHandle);

//	if (m_pLastFrameData)
//		free(m_pLastFrameData);

//	if (m_pPostData)
//		free(m_pPostData);

	delete this;
}

void C_AwesomiumBrowserInstance::Init(std::string id, std::string url, bool alpha)
{
	DevMsg("heeeeeeere: %s\n", id.c_str());
	m_id = id;
	m_initialURL = url;
	m_bAlpha = alpha;

	// create the texture (each instance has its own texture)
	std::string textureName = "canvas_";
	textureName += m_id;

	int iWidth = 1280;// g_pAnarchyManager->GetWebManager()->GetWebSurfaceWidth();
	int iHeight = 720;// g_pAnarchyManager->GetWebManager()->GetWebSurfaceHeight();

	if (m_id == "images")
	{
		iWidth = 512;
		iHeight = 512;
	}

	//int iWidth = 1920;
	//int iHeight = 1080;

	//m_pTexture = g_pMaterialSystem->FindTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, false, 1);

	if (!g_pMaterialSystem->IsTextureLoaded(textureName.c_str()))
	{
		if (m_bAlpha)
			m_pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, iWidth, iHeight, IMAGE_FORMAT_BGRA8888, 1);
		else
			m_pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, iWidth, iHeight, IMAGE_FORMAT_BGR888, 1);
	}
	else
		m_pTexture = g_pMaterialSystem->FindTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, false, 1);

	// get the regen and assign it
	CCanvasRegen* pRegen = g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen();
	//pRegen->SetEmbeddedInstance(this);
	m_pTexture->SetTextureRegenerator(pRegen);
}

/*
void C_SteamBrowserInstance::OnBrowserInstanceCreated(HTML_BrowserReady_t *pResult, bool bIOFailure)
{
	DevMsg("SteamworksBrowser: OnBrowserCreated - %s\n", pResult->GetCallbackName());

	// create the texture (each instance has its own texture)
	std::string textureName = "canvas_";
	textureName += m_id;

	int iWidth = 1280;// g_pAnarchyManager->GetWebManager()->GetWebSurfaceWidth();
	int iHeight = 720;// g_pAnarchyManager->GetWebManager()->GetWebSurfaceHeight();
	//int iWidth = 1920;
	//int iHeight = 1080;

	m_pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, iWidth, iHeight, IMAGE_FORMAT_BGR888, 1);

	// get the regen and assign it
	CCanvasRegen* pRegen = g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen();
	//pRegen->SetEmbeddedInstance(this);
	m_pTexture->SetTextureRegenerator(pRegen);



	// tell the input manager that the steam browser instance is active
	C_InputListenerSteamBrowser* pListener = g_pAnarchyManager->GetSteamBrowserManager()->GetInputListener();
	g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(m_pTexture);
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, (C_InputListener*)pListener);



	m_unBrowserHandle = pResult->unBrowserHandle;

	steamapicontext->SteamHTMLSurface()->SetSize(m_unBrowserHandle, iWidth, iHeight);

	g_pAnarchyManager->GetSteamBrowserManager()->OnSteamBrowserInstanceCreated(this);
	steamapicontext->SteamHTMLSurface()->LoadURL(m_unBrowserHandle, m_initialURL.c_str(), "");
}
*/

/*
void C_SteamBrowserInstance::OnNeedsPaint(const void* data, unsigned int width, unsigned int height, unsigned int depth)
{
	//DevMsg("Needs paint bra\n");
	
	m_bIsDirty = true;

	if (!m_bReadyForNextFrame || m_bCopyingFrame)
		return;

	m_bReadyForNextFrame = false;
	m_bReadyToCopyFrame = false;
	//m_bCopyingFrame = false;

	//WORD red_mask = 0xF800;
	//WORD green_mask = 0x7E0;
	//WORD blue_mask = 0x1F;


	void* dest = malloc(width*height*depth);
	Q_memcpy(dest, data, width*height*depth);

	if (m_pLastFrameData)
		free(m_pLastFrameData);

	m_pLastFrameData = dest;
	m_bReadyToCopyFrame = true;

	//this->CopyLastFrame()
}
*/

void C_AwesomiumBrowserInstance::OnMouseMove(float x, float y)
{
	int iMouseX = x * 1280;
	int iMouseY = y * 720;

	Awesomium::WebView* pWebView = this->GetWebView();
	if (pWebView)
		pWebView->InjectMouseMove(iMouseX, iMouseY);

	/*
	// inject mouse movement into the HUD too, if its active.
	C_WebTab* pHudWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();
	if (pHudWebTab && pHudWebTab != pWebTab && g_pAnarchyManager->GetInputManager()->GetInputMode())
	{
		WebView* pHudWebView = FindWebView(pHudWebTab);
		if (pHudWebView)
			pHudWebView->InjectMouseMove(iMouseX, iMouseY);
	}
	*/
}

void C_AwesomiumBrowserInstance::OnMousePressed(vgui::MouseCode code)
{
	int iButtonId = -1;
	if (code == MOUSE_LEFT)
		iButtonId = 0;
	else if (code == MOUSE_MIDDLE)
		iButtonId = 1;
	else if (code == MOUSE_RIGHT)
		iButtonId = 2;

	m_pWebView->InjectMouseDown((MouseButton)iButtonId);
}

void C_AwesomiumBrowserInstance::OnMouseReleased(vgui::MouseCode code)
{
	// translate the vgui::MouseCode into an Awesomium::MouseButton
	int iButtonId = -1;
	if (code == MOUSE_LEFT)
		iButtonId = 0;
	else if (code == MOUSE_MIDDLE)
		iButtonId = 1;
	else if (code == MOUSE_RIGHT)
		iButtonId = 2;
	
	m_pWebView->InjectMouseUp((MouseButton)iButtonId);
}
/*
void C_AwesomiumBrowserInstance::OnKeyPressed(vgui::KeyCode code)
{
	//	DevMsg("Code is %i\n", code);
	//input()->GetKeyCodeText();

	bool m_bMoveUp, m_bMoveDown, m_bMoveLeft, m_bMoveRight;

	//	if( IsJoystickAxisCode(code) ) {
	if (code == KEY_XBUTTON_UP)
	{
		m_bMoveUp = true;
		return;
	}
	else if (code == KEY_XBUTTON_DOWN)
	{
		m_bMoveDown = true;
		return;
	}
	else if (code == KEY_XBUTTON_LEFT)
	{
		m_bMoveLeft = true;
		return;
	}
	else if (code == KEY_XBUTTON_RIGHT)
	{
		m_bMoveRight = true;
		return;
	}

	//		return;
	//	}

	if (engine->Key_BindingForKey(code))
	{
		if (!Q_strcmp(engine->Key_BindingForKey(code), "alt_escape")) {
			engine->ClientCmd("alt_escape;\n");
			return;
		}
		else if (!Q_strcmp(engine->Key_BindingForKey(code), "alt_mouse_left")) {
			this->OnMousePressed(MOUSE_LEFT);
			this->OnMouseReleased(MOUSE_LEFT);
			return;
		}
		else if (!Q_strcmp(engine->Key_BindingForKey(code), "alt_mouse_right")) {
			this->OnMousePressed(MOUSE_RIGHT);
			this->OnMouseReleased(MOUSE_RIGHT);
			return;
		}
		else if (!Q_strcmp(engine->Key_BindingForKey(code), "alt_mouse_middle")) {
			this->OnMousePressed(MOUSE_MIDDLE);
			this->OnMouseReleased(MOUSE_MIDDLE);
			return;
		}
		else if (!Q_strcmp(engine->Key_BindingForKey(code), "alt_mouse_wheel_up")) {
	//		this->OnMouseWheeled(1);
			return;
		}
		else if (!Q_strcmp(engine->Key_BindingForKey(code), "alt_mouse_wheel_down")) {
	//		this->OnMouseWheeled(-1);
			return;
		}
	}

	if (IsJoystickCode(code) || !IsKeyCode(code)) return;

	using namespace Awesomium;

	WebView* pWebView = m_pWebView;

	WebKeyboardEvent pWebKeyboardEvent;
	pWebKeyboardEvent.type = WebKeyboardEvent::kTypeKeyDown;

	pWebKeyboardEvent.modifiers = 0;

	bool shift = (vgui::input()->IsKeyDown(KEY_LSHIFT) || vgui::input()->IsKeyDown(KEY_RSHIFT));
	bool ctrl = (vgui::input()->IsKeyDown(KEY_LCONTROL) || vgui::input()->IsKeyDown(KEY_RCONTROL));
	bool alt = (vgui::input()->IsKeyDown(KEY_LALT) || vgui::input()->IsKeyDown(KEY_RALT));

	if (shift)
		pWebKeyboardEvent.modifiers |= vgui::MODIFIER_SHIFT;

	if (ctrl)
		pWebKeyboardEvent.modifiers |= vgui::MODIFIER_CONTROL;

	if (alt)
		pWebKeyboardEvent.modifiers |= vgui::MODIFIER_ALT;

	int virtualKeyCode = KeyCodes::AK_UNKNOWN;
	std::string actualCharOutput = "";

	//	// SHIFT events are sent with keystrokes, so no need to send the SHIFT (or other modifiers) directly.
	//	if( code == KEY_ESCAPE || code == KEY_LSHIFT || code == KEY_RSHIFT || code == KEY_LALT || code == KEY_RALT || code == KEY_LCONTROL || code == KEY_RCONTROL || code == KEY_LWIN || code == KEY_RWIN || code == KEY_APP )
	if (code == KEY_ESCAPE || code == KEY_LALT || code == KEY_RALT || code == KEY_LCONTROL || code == KEY_RCONTROL || code == KEY_LWIN || code == KEY_RWIN || code == KEY_APP)
		return;

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

		//		case KEY_ESCAPE:
		//			virtualKeyCode = 0x1B;
		//			DevMsg("ESCAPE PRESSED!\n");
		//			break;

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
*/

void C_AwesomiumBrowserInstance::Update()
{
	//if (m_info->state == 1)
		this->OnProxyBind(null);
}

void C_AwesomiumBrowserInstance::ResizeFrameFromRGB565(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth)
{
//	uint uId = ThreadGetCurrentId();
//	C_SteamBrowserInstance* pSteamBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(uId);
//	SteamBrowserInstanceInfo_t* info = pSteamBrowserInstance->GetInfo();

	if (!m_pLastFrameData)
		return;
	
	m_bReadyForNextFrame = false;

	//	DevMsg("Resizing a %ux%u %iBBP (%i pitch) image to %ux%u %iBBP (%i pitch)\n", sourceWidth, sourceHeight, sourceDepth, sourcePitch, destWidth, destHeight, destDepth, destPitch);

	WORD red_mask = 0xF800;
	WORD green_mask = 0x7E0;
	WORD blue_mask = 0x1F;

	uint16* pRealSrc = (uint16*)pSrc;

	unsigned char* pDstRow = (unsigned char*)pDst;
	for (int dstY = 0; dstY<destHeight; dstY++)
	{

		unsigned int srcY = dstY * sourceHeight / destHeight;
		uint16* pSrcRow = pRealSrc + (srcY * ((int)sourcePitch / 2));

		unsigned char* pDstCur = pDstRow;

		for (int dstX = 0; dstX<destWidth; dstX++)
		{
			int srcX = dstX * sourceWidth / destWidth;

			int red = (pSrcRow[srcX] & red_mask) >> 11;
			int green = (pSrcRow[srcX] & green_mask) >> 5;
			int blue = (pSrcRow[srcX] & blue_mask);

			pDstCur[0] = blue * (255 / 31);
			pDstCur[1] = green * (255 / 63);
			pDstCur[2] = red * (255 / 31);

			pDstCur[3] = 255;

			pDstCur += destDepth;
		}

		pDstRow += destPitch;
	}

	m_bReadyForNextFrame = true;
}

void C_AwesomiumBrowserInstance::ResizeFrameFromRGB1555(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth)
{
//	uint uId = ThreadGetCurrentId();
//	C_SteamBrowserInstance* pSteamBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(uId);
//	SteamBrowserInstanceInfo_t* info = pSteamBrowserInstance->GetInfo();

	if (!m_pLastFrameData)
		return;

	m_bReadyForNextFrame = false;

	//	DevMsg("Resizing a %ux%u %iBBP (%i pitch) image to %ux%u %iBBP (%i pitch)\n", sourceWidth, sourceHeight, sourceDepth, sourcePitch, destWidth, destHeight, destDepth, destPitch);

	WORD red_mask = 0x7C00;
	WORD green_mask = 0x03E0;
	WORD blue_mask = 0x001F;

	uint16* pRealSrc = (uint16*)pSrc;

	unsigned char* pDstRow = (unsigned char*)pDst;
	for (int dstY = 0; dstY<destHeight; dstY++)
	{

		unsigned int srcY = dstY * sourceHeight / destHeight;
		uint16* pSrcRow = pRealSrc + (srcY * ((int)sourcePitch / 2));

		unsigned char* pDstCur = pDstRow;

		for (int dstX = 0; dstX<destWidth; dstX++)
		{
			int srcX = dstX * sourceWidth / destWidth;

			int red = (pSrcRow[srcX] & red_mask) >> 10;
			int green = (pSrcRow[srcX] & green_mask) >> 5;
			int blue = (pSrcRow[srcX] & blue_mask);

			pDstCur[0] = blue * (255 / 31);
			pDstCur[1] = green * (255 / 31);
			pDstCur[2] = red * (255 / 31);

			pDstCur[3] = 255;

			pDstCur += destDepth;
		}

		pDstRow += destPitch;
	}

	m_bReadyForNextFrame = true;
}

void C_AwesomiumBrowserInstance::ResizeFrameFromXRGB8888(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth)
{
	//DevMsg("Thread ID: %u\n", ThreadGetCurrentId);
//	uint uId = ThreadGetCurrentId();
//	C_SteamBrowserInstance* pSteamBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(uId);
//	SteamBrowserInstanceInfo_t* info = pSteamBrowserInstance->GetInfo();
	//SteamBrowserInstanceInfo_t* info = m_info;

	//if (!m_info->lastframedata)
//	DevMsg("Main Lock\n");

	if (!m_pLastFrameData)
		return;

	m_bReadyForNextFrame = false;

//	m_mutex.lock();
//	if (!m_info->lastframedata || !m_info->readyfornextframe)
	//	return;


	//m_info->readyfornextframe = false;

	//DevMsg("Resizing a %ux%u %iBBP (%i pitch) image to %ux%u %iBBP (%i pitch)\n", sourceWidth, sourceHeight, sourceDepth, sourcePitch, destWidth, destHeight, destDepth, destPitch);
//	DevMsg("Test: %s\n", pDest);

	unsigned int sourceWidthCopy = sourceWidth;
	unsigned int sourceHeightCopy = sourceHeight;
	size_t sourcePitchCopy = sourcePitch;
	unsigned int sourceDepthCopy = sourceDepth;

	//void* pSrcCopy = malloc(sourcePitchCopy * sourceHeightCopy);
	//Q_memcpy(pSrcCopy, pSrc, sourcePitchCopy * sourceHeightCopy);


	const unsigned char* pRealSrc = (const unsigned char*)pSrc;
	unsigned char* pDstRow = (unsigned char*)pDst;
	for (int dstY = 0; dstY<destHeight; dstY++)
	{
		unsigned int srcY = dstY * sourceHeight / destHeight;
		const unsigned char* pSrcRow = pRealSrc + srcY*(sourcePitch);

		unsigned char* pDstCur = pDstRow;

		for (int dstX = 0; dstX<destWidth; dstX++)
		{
			int srcX = dstX * sourceWidth / destWidth;
			pDstCur[0] = pSrcRow[srcX*sourceDepth + 0];
			pDstCur[1] = pSrcRow[srcX*sourceDepth + 1];
			pDstCur[2] = pSrcRow[srcX*sourceDepth + 2];

			pDstCur[3] = 255;

			pDstCur += destDepth;
		}

		pDstRow += destPitch;
	}

	/*
	const unsigned char* pRealSrc = (const unsigned char*)pSrc;
	unsigned char* pDstRow = (unsigned char*)pDst;
	for (int dstY = 0; dstY<destHeight; dstY++)
	{
		unsigned int srcY = dstY * sourceHeight / destHeight;
		const unsigned char* pSrcRow = pRealSrc + srcY*(sourcePitch);

		unsigned char* pDstCur = pDstRow;

		for (int dstX = 0; dstX<destWidth; dstX++)
		{
			int srcX = dstX * sourceWidth / destWidth;
			pDstCur[0] = pSrcRow[srcX*sourceDepth + 0];
			pDstCur[1] = pSrcRow[srcX*sourceDepth + 1];
			pDstCur[2] = pSrcRow[srcX*sourceDepth + 2];

			pDstCur[3] = 255;

			pDstCur += destDepth;
		}

		pDstRow += destPitch;
	}
	*/

//	free(pSrcCopy);

	m_bReadyForNextFrame = true;

//	m_mutex.unlock();
//	DevMsg("Main Unlock\n");
}

void C_AwesomiumBrowserInstance::CopyLastFrame(unsigned char* dest, unsigned int width, unsigned int height, size_t pitch, unsigned int depth)
{
	if (m_bCopyingFrame || !m_bReadyToCopyFrame)
		return;

	m_bCopyingFrame = true;
	m_bReadyToCopyFrame = false;
	//DevMsg("Copied.\n");
	memcpy(dest, m_pLastFrameData, pitch * height);

//	if (m_info->videoformat == RETRO_PIXEL_FORMAT_RGB565)
//		this->ResizeFrameFromRGB565(m_info->lastframedata, dest, m_info->lastframewidth, m_info->lastframeheight, m_info->lastframepitch, 3, width, height, pitch, depth);
//	else if (m_info->videoformat == RETRO_PIXEL_FORMAT_XRGB8888)
//		this->ResizeFrameFromXRGB8888(m_info->lastframedata, dest, m_info->lastframewidth, m_info->lastframeheight, m_info->lastframepitch, 4, width, height, pitch, depth);
//	else
//		this->ResizeFrameFromRGB1555(m_info->lastframedata, dest, m_info->lastframewidth, m_info->lastframeheight, m_info->lastframepitch, 3, width, height, pitch, depth);

	m_bReadyForNextFrame = true;
	m_bCopyingFrame = false;
	m_bIsDirty = false;
}

void C_AwesomiumBrowserInstance::SetUrl(std::string url)
{
	if (m_pWebView)
		m_pWebView->LoadURL(WebURL(WSLit(url.c_str())));
}

bool C_AwesomiumBrowserInstance::HasFocus()
{
	return (this == g_pAnarchyManager->GetAwesomiumBrowserManager()->GetSelectedAwesomiumBrowserInstance());
}

bool C_AwesomiumBrowserInstance::Focus()
{
	return g_pAnarchyManager->GetAwesomiumBrowserManager()->SelectAwesomiumBrowserInstance(this);
}

bool C_AwesomiumBrowserInstance::Select()
{
	return g_pAnarchyManager->GetAwesomiumBrowserManager()->SelectAwesomiumBrowserInstance(this);
}

bool C_AwesomiumBrowserInstance::Deselect()
{
	return g_pAnarchyManager->GetAwesomiumBrowserManager()->SelectAwesomiumBrowserInstance(null);
}

void C_AwesomiumBrowserInstance::Close()
{
	g_pAnarchyManager->GetAwesomiumBrowserManager()->DestroyAwesomiumBrowserInstance(this);
}

void C_AwesomiumBrowserInstance::OnProxyBind(C_BaseEntity* pBaseEntity)
{
	if (m_id == "images")
		return;

	/*
	if ( pBaseEntity )
	DevMsg("WebTab: OnProxyBind: %i\n", pBaseEntity->entindex());
	else
	DevMsg("WebTab: OnProxyBind\n");
	*/

	//if (m_id == "hud")
//		DevMsg("Found hud should be priority!!\n");

	// visiblity test
	if (m_iLastVisibleFrame < gpGlobals->framecount)
	{
		if (!g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(this))
			g_pAnarchyManager->GetCanvasManager()->IncrementVisibleCanvasesCurrentFrame();
		else
			g_pAnarchyManager->GetCanvasManager()->IncrementVisiblePriorityCanvasesCurrentFrame();
	}
	m_iLastVisibleFrame = gpGlobals->framecount;

	if (m_iLastRenderedFrame < gpGlobals->framecount)
	{
		/*
		if (!g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(this))
			g_pAnarchyManager->GetCanvasManager()->IncrementVisibleCanvasesCurrentFrame();
		else
			g_pAnarchyManager->GetCanvasManager()->IncrementVisiblePriorityCanvasesCurrentFrame();
		*/

		bool bIsDirty = false;
		if (m_pWebView)
		{
			Awesomium::BitmapSurface* surface = static_cast<Awesomium::BitmapSurface*>(m_pWebView->surface());
			if (surface != 0)
				bIsDirty = surface->is_dirty();
		}

		if (g_pAnarchyManager->GetCanvasManager()->ShouldRender(this) && bIsDirty)//&& m_bIsDirty)
		{
			Render();

			if (m_id == "hud")
				DevMsg("Render PRIORITY (HUD)!\n");
			else
				DevMsg("Render AWESOMIUM!\n");
		}
		else
		{
		//	if (m_id == "hud")
		//		DevMsg("Don't render HUD\n");
		}
	}
}

void C_AwesomiumBrowserInstance::Render()
{
	if (m_id == "images")
		return;
	//DevMsg("Rendering texture: %s\n", m_pTexture->GetName());
	//	DevMsg("Render Web Tab: %s\n", this->GetTexture()->Ge>GetId().c_str());
	//DevMsg("WebTab: Render: %s on %i\n", m_id.c_str(), gpGlobals->framecount);
	g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen()->SetEmbeddedInstance(this);
	m_pTexture->Download();

	m_iLastRenderedFrame = gpGlobals->framecount;

	if (g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(this))
		g_pAnarchyManager->GetCanvasManager()->SetLastPriorityRenderedFrame(gpGlobals->framecount);
	else
		g_pAnarchyManager->GetCanvasManager()->SetLastRenderedFrame(gpGlobals->framecount);
}

void C_AwesomiumBrowserInstance::RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect)
{
	if (!m_pWebView)
		return;

	Awesomium::BitmapSurface* surface = static_cast<Awesomium::BitmapSurface*>(m_pWebView->surface());
	if (surface != 0)
		surface->CopyTo(pVTFTexture->ImageData(0, 0, 0), pSubRect->width * 4, 4, false, false);

	//if (m_info->state == 1)
		//this->CopyLastFrame(pVTFTexture->ImageData(0, 0, 0), pSubRect->width, pSubRect->height, pSubRect->width * 4, 4);
}

void C_AwesomiumBrowserInstance::DispatchJavaScriptMethod(std::string objectName, std::string objectMethod, std::vector<std::string> methodArguments)
{
	//g_pAnarchyManager->GetWebManager()->GetWebBrowser()->DispatchJavaScriptMethod(this, objectName, objectMethod, methodArguments);
	g_pAnarchyManager->GetAwesomiumBrowserManager()->DispatchJavaScriptMethod(this, objectName, objectMethod, methodArguments);
}

void C_AwesomiumBrowserInstance::DispatchJavaScriptMethodCalls()
{
	// do it in batches to reduce sync calls to the web view to O(1) instead of O(N) (assuming every method using the same JS object.)
	//g_pAnarchyManager->GetWebManager()->DispatchJavaScriptMethods(this);
	g_pAnarchyManager->GetAwesomiumBrowserManager()->DispatchJavaScriptMethods(this);
	m_javaScriptMethodCalls.clear();
}

void C_AwesomiumBrowserInstance::SetHudTitle(std::string title)
{
	if (g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud") != this)
		return;

//	if (g_pAnarchyManager->GetWebManager()->GetHudWebTab() != this)
//		return;

	JavaScriptMethodCall_t* pJavaScriptMethodCall = new JavaScriptMethodCall_t;
	pJavaScriptMethodCall->objectName = "arcadeHud";
	pJavaScriptMethodCall->methodName = "setHudeTitle";
	pJavaScriptMethodCall->methodArguments.push_back(title);
	m_javaScriptMethodCalls.push_back(pJavaScriptMethodCall);

	if (m_iState >= 2)
		this->DispatchJavaScriptMethodCalls();
}

void C_AwesomiumBrowserInstance::AddHudLoadingMessage(std::string type, std::string text, std::string title, std::string id, std::string min, std::string max, std::string current, std::string callbackMethod)
{
	if (g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud") != this)
		return;

	JavaScriptMethodCall_t* pJavaScriptMethodCall = new JavaScriptMethodCall_t;
	pJavaScriptMethodCall->objectName = "arcadeHud";
	pJavaScriptMethodCall->methodName = "addHudLoadingMessage";
	pJavaScriptMethodCall->methodArguments.push_back(type);
	pJavaScriptMethodCall->methodArguments.push_back(text);
	pJavaScriptMethodCall->methodArguments.push_back(title);
	pJavaScriptMethodCall->methodArguments.push_back(id);
	pJavaScriptMethodCall->methodArguments.push_back(min);
	pJavaScriptMethodCall->methodArguments.push_back(max);
	pJavaScriptMethodCall->methodArguments.push_back(current);
	pJavaScriptMethodCall->methodArguments.push_back(callbackMethod);
	m_javaScriptMethodCalls.push_back(pJavaScriptMethodCall);

	if (m_iState >= 2)
		this->DispatchJavaScriptMethodCalls();
}

bool C_AwesomiumBrowserInstance::RequestLoadSimpleImage(std::string channel, std::string itemId)
{
	if (m_id != "images" || m_iNumImagesLoading == -1 || m_iNumImagesLoading >= m_iMaxImagesLoading)
		return false;

	// check if we are ready to accept a new image request
	std::vector<std::string> args;
	args.push_back(channel);
	args.push_back(itemId);	// these should also be remembered locally too, so we can load entire websites as images too.

	m_iNumImagesLoading++;
	DispatchJavaScriptMethod("imageLoader", "loadImage", args);
	return true;
}

void C_AwesomiumBrowserInstance::OnSimpleImageReady(std::string channel, std::string itemId, std::string field, ITexture* pTexture)
{
	if (m_id != "images")
		return;

	CWebSurfaceProxy::OnSimpleImageRendered(channel, itemId, field, pTexture);
	m_iNumImagesLoading--;

	if (pTexture)
	{
		std::vector<std::string> args;
		this->DispatchJavaScriptMethod("imageLoader", "onImageRender", args);
	}
	//m_bReadyForNextSimpleImage = true;
}

C_EmbeddedInstance* C_AwesomiumBrowserInstance::GetParentSelectedEmbeddedInstance()
{
	return g_pAnarchyManager->GetAwesomiumBrowserManager()->GetSelectedAwesomiumBrowserInstance();
}

C_InputListener* C_AwesomiumBrowserInstance::GetInputListener()
{
	return g_pAnarchyManager->GetAwesomiumBrowserManager()->GetInputListener();
}