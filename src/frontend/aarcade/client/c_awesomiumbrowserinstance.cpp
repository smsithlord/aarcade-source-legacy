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
	m_imagesSessionId = g_pAnarchyManager->GenerateUniqueId();
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
	m_URL = "";
}

C_AwesomiumBrowserInstance::~C_AwesomiumBrowserInstance()
{
	DevMsg("AwesomiumBrowserInstance: Destructor\n");
	
	if (m_id == "hud")
		DevMsg("Hud destroyed!\n");

	if (m_pTexture && m_id != "images" )
	{
		m_pTexture->SetTextureRegenerator(null);

		g_pAnarchyManager->GetCanvasManager()->UnreferenceTexture(m_pTexture);
		m_pTexture->DecrementReferenceCount();	// FIXME: this is hte next statement to execute on an exit crash after checkin out a level...
		m_pTexture->DeleteIfUnreferenced();
		m_pTexture = null;
	}
}

void C_AwesomiumBrowserInstance::SelfDestruct()
{
	DevMsg("AwesomiumBrowserInstance: SelfDestruct\n");
	g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen()->NotifyInstanceAboutToDie(this);
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

	int iWidth = (id == "hud") ? AA_HUD_INSTANCE_WIDTH : AA_EMBEDDED_INSTANCE_WIDTH;// g_pAnarchyManager->GetWebManager()->GetWebSurfaceWidth();
	int iHeight = (id == "hud") ? AA_HUD_INSTANCE_HEIGHT : AA_EMBEDDED_INSTANCE_HEIGHT;// g_pAnarchyManager->GetWebManager()->GetWebSurfaceHeight();

	if (m_id == "images")
	{
		iWidth = AA_THUMBNAIL_SIZE;
		iHeight = AA_THUMBNAIL_SIZE;
	}
	
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

	if (m_id == "images")
		this->ResetImagesSession();
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
	if (g_pAnarchyManager->IsPaused())
		return;

	unsigned int width = (m_id == "hud") ? AA_HUD_INSTANCE_WIDTH : AA_EMBEDDED_INSTANCE_WIDTH;
	unsigned int height = (m_id == "hud") ? AA_HUD_INSTANCE_HEIGHT : AA_EMBEDDED_INSTANCE_HEIGHT;

	int iMouseX = x * width;
	int iMouseY = y * height;

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
	if (g_pAnarchyManager->IsPaused())
		return;

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
	if (g_pAnarchyManager->IsPaused())
		return;

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

void C_AwesomiumBrowserInstance::OnMouseWheeled(int delta)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	DevMsg("Awesomium instance mouse wheeled: %i\n", delta);

	if (g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity() && m_id == "hud" )
	{
		if (vgui::input()->IsKeyDown(KEY_LSHIFT) || vgui::input()->IsKeyDown(KEY_RSHIFT))
		{
			// scale instead of sending mouse wheel to the web view
			g_pAnarchyManager->GetMetaverseManager()->ScaleObject(g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity(), delta);
		}
		else if (vgui::input()->IsKeyDown(KEY_LCONTROL) || vgui::input()->IsKeyDown(KEY_RCONTROL))
		{
			// specialWheel instead of sending mouse wheel to the web view
			std::vector<std::string> params;
			params.push_back(std::string(VarArgs("%i", delta)));
			this->DispatchJavaScriptMethod("cmdListener", "specialWheel", params);
		}
		else if (vgui::input()->IsKeyDown(KEY_E))
		{
			// update the internal rotation axis
			int amount = (delta >= 0) ? 1 : -1;
			int axis = g_pAnarchyManager->GetMetaverseManager()->CycleSpawningRotationAxis(amount);

			// ALSO change rotation axis instead of sending mouse wheel to the web view
			std::vector<std::string> params;
			params.push_back(std::string(VarArgs("%i", axis)));
			this->DispatchJavaScriptMethod("cmdListener", "setRotationAxis", params);
		}
		else
			m_pWebView->InjectMouseWheel(20 * delta, 0);	// just wheel like normal
	}
	else
		m_pWebView->InjectMouseWheel(20 * delta, 0);
}

void C_AwesomiumBrowserInstance::OnKeyPressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState)
{
	// ignore input sometimes
	if (g_pAnarchyManager->IsPaused() || IsJoystickCode(code) || !IsKeyCode(code))
		return;

	//	if (code == KEY_ESCAPE || code == KEY_LALT || code == KEY_RALT || code == KEY_LCONTROL || code == KEY_RCONTROL || code == KEY_LWIN || code == KEY_RWIN || code == KEY_APP)
	//	return;

	// Forward input to the selected embedded instance if we are the HUD
	if (m_id == "hud" && !this->HasFocus())
	{
		g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance()->GetInputListener()->OnKeyCodePressed(code, bShiftState, bCtrlState, bAltState, bWinState, bAutorepeatState);
		return;
	}

	//	DevMsg("Code is %i\n", code);
	//input()->GetKeyCodeText();

	// catch the buttons bound to mouse movement
	/*
	bool m_bMoveUp, m_bMoveDown, m_bMoveLeft, m_bMoveRight;
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
	*/

	// a better way to catch buttons bound to special functions
	/*
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
	*/

	using namespace Awesomium;
	WebView* pWebView = m_pWebView;
	C_InputListenerAwesomiumBrowser* inputListener = dynamic_cast<C_InputListenerAwesomiumBrowser*>(this->GetInputListener());

	// 2 events will be built that will use nearly identical params

	// build the common params
	int virtualKeyCode = inputListener->ConvertSourceButtonToAwesomiumButton(code);
	std::string actualCharOutput = this->GetOutput(code, bShiftState, bCtrlState, bAltState, bWinState, bAutorepeatState);
	std::string unmodifiedOutput = this->GetOutput(code, false, false, false, false, false);
	bool hasChar = (actualCharOutput != "");
	bool hasUnmodifiedChar = (unmodifiedOutput != "");
	char outputChar = (hasChar) ? actualCharOutput[0] : null;
	char unmodifiedOutputChar = (hasUnmodifiedChar) ? unmodifiedOutput[0] : null;
	
	int modifiers = 0;
	modifiers |= (bShiftState) ? WebKeyboardEvent::kModShiftKey : 0;
	modifiers |= (bCtrlState) ? WebKeyboardEvent::kModControlKey : 0;
	modifiers |= (bAltState) ? WebKeyboardEvent::kModAltKey : 0;
	modifiers |= (bWinState) ? WebKeyboardEvent::kModMetaKey : 0;
	//modifiers |= (bKeypadState) ? WebKeyboardEvent::kModIsKeypad : 0;
	modifiers |= (code >= KEY_PAD_0 && code <= KEY_PAD_DECIMAL) ? WebKeyboardEvent::kModIsKeypad : 0;
	modifiers |= (bAutorepeatState) ? WebKeyboardEvent::kModIsAutorepeat : 0;

	char* keyIdentifier = new char[20];
	GetKeyIdentifierFromVirtualKeyCode(virtualKeyCode, &keyIdentifier);
	
	// build the keydown event
	WebKeyboardEvent pKeydownEvent;
	pKeydownEvent.type = WebKeyboardEvent::kTypeKeyDown;
	pKeydownEvent.modifiers = modifiers;
	pKeydownEvent.virtual_key_code = virtualKeyCode;
	pKeydownEvent.native_key_code = virtualKeyCode;
	strcpy(pKeydownEvent.key_identifier, keyIdentifier);
	pKeydownEvent.text[0] = outputChar;
	pKeydownEvent.unmodified_text[0] = unmodifiedOutputChar;

	pWebView->InjectKeyboardEvent(pKeydownEvent);

	// build the char event
	if (hasChar)
	{
		WebKeyboardEvent pCharEvent;
		pCharEvent.type = WebKeyboardEvent::kTypeChar;
		pCharEvent.modifiers = modifiers;
		pCharEvent.virtual_key_code = outputChar;
		pCharEvent.native_key_code = unmodifiedOutputChar;
		strcpy(pCharEvent.key_identifier, keyIdentifier);
		pCharEvent.text[0] = outputChar;
		pCharEvent.unmodified_text[0] = unmodifiedOutputChar;

		pWebView->InjectKeyboardEvent(pCharEvent);
	}

	delete[] keyIdentifier;
}

void C_AwesomiumBrowserInstance::OnKeyReleased(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	if (m_id == "hud" && !this->HasFocus())
	{
		g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance()->GetInputListener()->OnKeyCodeReleased(code, bShiftState, bCtrlState, bAltState, bWinState, bAutorepeatState);
		return;
	}

	using namespace Awesomium;
	WebView* pWebView = m_pWebView;
	C_InputListenerAwesomiumBrowser* inputListener = dynamic_cast<C_InputListenerAwesomiumBrowser*>(this->GetInputListener());

	// build the params
	int virtualKeyCode = inputListener->ConvertSourceButtonToAwesomiumButton(code);

	int modifiers = 0;
	modifiers |= (bShiftState) ? WebKeyboardEvent::kModShiftKey : 0;
	modifiers |= (bCtrlState) ? WebKeyboardEvent::kModControlKey : 0;
	modifiers |= (bAltState) ? WebKeyboardEvent::kModAltKey : 0;
	modifiers |= (bWinState) ? WebKeyboardEvent::kModMetaKey : 0;
	//modifiers |= (bKeypadState) ? WebKeyboardEvent::kModIsKeypad : 0;
	modifiers |= (code >= KEY_PAD_0 && code <= KEY_PAD_DECIMAL) ? WebKeyboardEvent::kModIsKeypad : 0;
	modifiers |= (bAutorepeatState) ? WebKeyboardEvent::kModIsAutorepeat : 0;

	char* keyIdentifier = new char[20];
	GetKeyIdentifierFromVirtualKeyCode(virtualKeyCode, &keyIdentifier);

	// build the keyup event
	WebKeyboardEvent pKeyupEvent;
	pKeyupEvent.type = WebKeyboardEvent::kTypeKeyUp;
	pKeyupEvent.modifiers = modifiers;
	pKeyupEvent.virtual_key_code = virtualKeyCode;	//getWebKeyFromSDLKey(event.key.keysym.sym);
	pKeyupEvent.native_key_code = virtualKeyCode;	//event.key.keysym.scancode;
	strcpy(pKeyupEvent.key_identifier, keyIdentifier);

	pWebView->InjectKeyboardEvent(pKeyupEvent);


	// FIXME: Dont' we need to send a keyup msg to awesomium?? (check the pressed msg to confirm)
	//if (code == KEY_ESCAPE || code == KEY_LALT || code == KEY_RALT || code == KEY_LCONTROL || code == KEY_RCONTROL || code == KEY_LWIN || code == KEY_RWIN || code == KEY_APP)
		//return;

	delete[] keyIdentifier;
}

void C_AwesomiumBrowserInstance::Update()
{
	if (g_pAnarchyManager->GetSuspendEmbedded())
		return;

	if (g_pAnarchyManager->IsPaused())
		return;

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
	if (m_bCopyingFrame || !m_bReadyToCopyFrame || g_pAnarchyManager->GetSuspendEmbedded())
		return;

	if (g_pAnarchyManager->IsPaused())
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

bool C_AwesomiumBrowserInstance::IsSelected()
{
	return (this == g_pAnarchyManager->GetAwesomiumBrowserManager()->GetSelectedAwesomiumBrowserInstance());
}

bool C_AwesomiumBrowserInstance::HasFocus()
{
	return (this == g_pAnarchyManager->GetAwesomiumBrowserManager()->GetFocusedAwesomiumBrowserInstance());
}

bool C_AwesomiumBrowserInstance::Focus()	// FIXME: Change this to a void???
{
	//return g_pAnarchyManager->GetAwesomiumBrowserManager()->SelectAwesomiumBrowserInstance(this);
	if (!m_pWebView)
		return false;

	m_pWebView->Focus();
	g_pAnarchyManager->GetAwesomiumBrowserManager()->FocusAwesomiumBrowserInstance(this);
	//return g_pAnarchyManager->GetAwesomiumBrowserManager()->SelectAwesomiumBrowserInstance(this);
	return true;
}

bool C_AwesomiumBrowserInstance::Blur()
{
	if (g_pAnarchyManager->GetAwesomiumBrowserManager()->GetFocusedAwesomiumBrowserInstance() == this)
		g_pAnarchyManager->GetAwesomiumBrowserManager()->FocusAwesomiumBrowserInstance(null);

	return true;
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
	if (g_pAnarchyManager->IsPaused())
		return;

	if (g_pAnarchyManager->GetSuspendEmbedded())
		return;

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

		m_iLastVisibleFrame = gpGlobals->framecount;
	//if (m_iLastRenderedFrame < gpGlobals->framecount)
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
			//DevMsg("Do a renderr...\n");
			Render();
			//DevMsg("did it.\n");

		//	if (m_id == "hud")
		//		DevMsg("Render PRIORITY (HUD)!\n");
		//	else
		//		DevMsg("Render AWESOMIUM!\n");
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
	if (g_pAnarchyManager->IsPaused())
		return;

	if (m_id == "images")
		return;
	//DevMsg("Rendering texture: %s\n", m_pTexture->GetName());
	//	DevMsg("Render Web Tab: %s\n", this->GetTexture()->Ge>GetId().c_str());
	//DevMsg("WebTab: Render: %s on %i\n", m_id.c_str(), gpGlobals->framecount);
//	DevMsg("Start dl ... ");
	g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen()->SetEmbeddedInstance(this);
	m_pTexture->Download();
	//DevMsg("done\n");

	m_iLastRenderedFrame = gpGlobals->framecount;

	if (g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(this))
		g_pAnarchyManager->GetCanvasManager()->SetLastPriorityRenderedFrame(gpGlobals->framecount);
	else
		g_pAnarchyManager->GetCanvasManager()->SetLastRenderedFrame(gpGlobals->framecount);
}

void C_AwesomiumBrowserInstance::RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect)
{
	if (!m_pWebView || g_pAnarchyManager->GetSuspendEmbedded() )
		return;

	if (g_pAnarchyManager->IsPaused())
		return;

	if (!pTexture || !pTexture->IsProcedural())
	{
		DevMsg("Error: RegenerateTextureBits called with a non-procedural texture!\n");
		return;
	}

//	if (m_id == "images")
//		DevMsg("Regenerating image bits...\n");

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

void C_AwesomiumBrowserInstance::ResetImagesSession()
{
	m_imagesSessionId = g_pAnarchyManager->GenerateUniqueId();
	m_iNumImagesLoading = 0;
	//m_iMaxImagesLoading = 10;
}

bool C_AwesomiumBrowserInstance::RequestLoadSimpleImage(std::string channel, std::string itemId)
{
	if (m_id != "images" || m_iNumImagesLoading == -1 || m_iNumImagesLoading >= m_iMaxImagesLoading)
		return false;

	// check if we are ready to accept a new image request
	std::vector<std::string> args;
	args.push_back(m_imagesSessionId);	// so we know when to quit loading old shit
	args.push_back(channel);
	args.push_back(itemId);	// these should also be remembered locally too, so we can load entire websites as images too.

	m_iNumImagesLoading++;
	DispatchJavaScriptMethod("imageLoader", "loadImage", args);
	return true;
}

// FIXME: we probably don't need sessionId's here
void C_AwesomiumBrowserInstance::OnSimpleImageReady(std::string sessionId, std::string channel, std::string itemId, std::string field, ITexture* pTexture)
{
	if (m_id != "images" || (sessionId != "ready" && sessionId != m_imagesSessionId))
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

void C_AwesomiumBrowserInstance::SaveImageToCache(std::string fieldVal)
{
	std::string filename = engine->GetGameDirectory();
	filename = filename.substr(0, filename.length() - 9);
	//DevMsg("Tester filename here is: %s\n", filename.c_str());
	filename += "/aarcade_user/imagecache/";
	//filename += g_pAnarchyManager->GenerateCRC32Hash(fieldVal.c_str());
	filename += g_pAnarchyManager->GenerateLegacyHash(fieldVal.c_str());
	filename += ".jpg";

	//char* filenameFixed = new char[AA_MAX_STRING];
	char filenameFixed[AA_MAX_STRING];
	Q_strcpy(filenameFixed, filename.c_str());
	V_FixSlashes(filenameFixed, '/');

	//DevMsg("AAAaannnnnd here it's: %s\n", filenameFixed);

	Awesomium::BitmapSurface* surface = static_cast<Awesomium::BitmapSurface*>(m_pWebView->surface());
	if (surface != 0)
	{
	//	DevMsg("Saving to JPG...\n");
		surface->SaveToJPEG(WSLit(filenameFixed), 90);
		//if (!surface->SaveToJPEG(WSLit(filenameFixed), 90))
		//	DevMsg("Failed to save to JPG!\n");
	}
}

C_EmbeddedInstance* C_AwesomiumBrowserInstance::GetParentSelectedEmbeddedInstance()
{
	return g_pAnarchyManager->GetAwesomiumBrowserManager()->GetSelectedAwesomiumBrowserInstance();
}

C_InputListener* C_AwesomiumBrowserInstance::GetInputListener()
{
	return g_pAnarchyManager->GetAwesomiumBrowserManager()->GetInputListener();
}