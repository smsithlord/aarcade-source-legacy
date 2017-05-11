// ;..\..\portaudio\lib\portaudio_x86.lib

#include "cbase.h"
#include "aa_globals.h"

//#include "aa_globals.h"
//#include "c_steambrowserinstance.h"
#include "c_inputlistenersteambrowser.h"
#include "c_anarchymanager.h"
//#include "../../../public/vgui_controls/Controls.h"  
#include "vgui/IInput.h"
//#include "vgui/VGUI.h"
#include "c_canvasregen.h"
#include "c_embeddedinstance.h"
#include "inputsystem/iinputsystem.h"

//#include <mutex>
#include "../../../public/vgui_controls/HTML.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

vgui::KeyCode C_SteamBrowserInstance::KeyCode_VirtualKeyToVGUI(int key)
{
	// Some tools load vgui for localization and never use input
	if (!g_pInputSystem)
		return KEY_NONE;
	return g_pInputSystem->VirtualKeyToButtonCode(key);
}

int C_SteamBrowserInstance::KeyCode_VGUIToVirtualKey(vgui::KeyCode code)
{
	// Some tools load vgui for localization and never use input
	if (!g_pInputSystem)
		return VK_RETURN;

	return g_pInputSystem->ButtonCodeToVirtualKey(code);
}

///*
//-----------------------------------------------------------------------------
// Purpose: return the bitmask of any modifier keys that are currently down
//-----------------------------------------------------------------------------
int C_SteamBrowserInstance::GetKeyModifiersAlt()
{
	// Any time a key is pressed reset modifier list as well
	int nModifierCodes = 0;
	if (vgui::input()->IsKeyDown(KEY_LCONTROL) || vgui::input()->IsKeyDown(KEY_RCONTROL))
		nModifierCodes |= ISteamHTMLSurface::k_eHTMLKeyModifier_CtrlDown;

	if (vgui::input()->IsKeyDown(KEY_LALT) || vgui::input()->IsKeyDown(KEY_RALT))
		nModifierCodes |= ISteamHTMLSurface::k_eHTMLKeyModifier_AltDown;

	if (vgui::input()->IsKeyDown(KEY_LSHIFT) || vgui::input()->IsKeyDown(KEY_RSHIFT))
		nModifierCodes |= ISteamHTMLSurface::k_eHTMLKeyModifier_ShiftDown;

#ifdef OSX
	// for now pipe through the cmd-key to be like the control key so we get copy/paste
	if (vgui::input()->IsKeyDown(KEY_LWIN) || vgui::input()->IsKeyDown(KEY_RWIN))
		nModifierCodes |= ISteamHTMLSurface::k_eHTMLKeyModifier_CtrlDown;
#endif

	return nModifierCodes;
}
//*/

C_SteamBrowserInstance::C_SteamBrowserInstance()
{
	m_bSteamworksCopying = false;
	DevMsg("SteamBrowserInstance: Constructor\n");
	m_pTexture = null;
	m_iLastRenderedFrame = -1;
	m_pLastFrameData = null;
	m_bReadyForNextFrame = true;
	m_bCopyingFrame = false;
	m_bReadyToCopyFrame = false;
	m_pPostData = null;
	m_initialURL = "";
	m_bIsDirty = false;
	m_unHandle = 0;
	m_iLastVisibleFrame = -1;
	m_URL = "";
	m_title = "";
	m_id = "";
	m_iOriginalEntIndex = -1;
}

C_SteamBrowserInstance::~C_SteamBrowserInstance()
{
	DevMsg("SteamBrowserInstance: Destructor\n");
}
/*
void C_SteamBrowserInstance::DoDefunctDestruct(bool& result)
{
	DevMsg("SteamBrowserInstance: DoDefunctDestruct %s\n", m_id.c_str());

	//if (!m_bReadyForNextFrame || m_bCopyingFrame)
	if (!m_bReadyForNextFrame || m_bCopyingFrame)
	{
		DevMsg("CRITICAL WARNING: Frame is STILL copying right now, but wants to self descruct! %i %i\n", m_bReadyForNextFrame, m_bCopyingFrame);
		result = false;
	}
	else
	{
		g_pAnarchyManager->GetCanvasManager()->UnreferenceEmbeddedInstance(this);
		g_pAnarchyManager->GetCanvasManager()->UnreferenceTexture(m_pTexture);
		g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen()->NotifyInstanceAboutToDie(this);

		steamapicontext->SteamHTMLSurface()->RemoveBrowser(m_unHandle);

		m_bReadyForNextFrame = false;
		m_bReadyToCopyFrame = false;

		if (m_pLastFrameData)
		{
			free(m_pLastFrameData);
			m_pLastFrameData = null;
		}

		m_pPostData = "";

//		if (m_pPostData)
	//	{
	//		free(m_pPostData);
	//		m_pPostData = null;
	//	}

		m_bDying = true;
		result = true;
		delete this;
	}

}
*/

void C_SteamBrowserInstance::SelfDestruct()
{
	DevMsg("SteamBrowserInstance: SelfDestruct %s\n", m_id.c_str());
	DevMsg("\tInstance Texture Name: %s\n", m_pTexture->GetName());
	DevMsg("\tIs Texture Loaded: %i\n", (g_pMaterialSystem->IsTextureLoaded(m_pTexture->GetName())));

	// TODO: poll every AArcade sub-system that could be holding a reference to: ***the texture*** or the instance
	// Including:
	// - Canvas Manager's DisplayInstance
	// - Canvas Manager's FirstInstanceToDisplay
	// - Input Manager's EmbeddedInstance
	// - Input Slate's CanvasTexture
	// - 

	DevMsg("\tIs Canvas Manager's DisplayInstance: %i\n", (g_pAnarchyManager->GetCanvasManager()->GetDisplayInstance() == this));
	//DevMsg("\tIs Canvas Manager's FirstInstanceToDisplay: %i\n", (g_pAnarchyManager->GetCanvasManager()->GetFirstInstanceToDisplay() == this));	// calling this upon map transition crashes if the firsttodisplay instance is among the open ones that gets closed all.
	DevMsg("\tIs Input Manager's EmbeddedInstance: %i\n", (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == this));
	DevMsg("\tIs Input Slate's CanvasTexture: %i\n", (g_pAnarchyManager->GetInputManager()->GetInputSlateCanvasTexture() == m_pTexture));

	if (g_pAnarchyManager->GetCanvasManager()->GetDisplayInstance() == this)
		g_pAnarchyManager->GetCanvasManager()->SetDifferentDisplayInstance(this);
		//g_pAnarchyManager->GetCanvasManager()->SetDisplayInstance(null);


	// selected steamworks browser instance
	if (this == g_pAnarchyManager->GetSteamBrowserManager()->GetSelectedSteamBrowserInstance())
		g_pAnarchyManager->GetSteamBrowserManager()->SelectSteamBrowserInstance(null);

	// focused steamworks browser instance
	if (this == g_pAnarchyManager->GetSteamBrowserManager()->GetFocusedSteamBrowserInstance())
		g_pAnarchyManager->GetSteamBrowserManager()->FocusSteamBrowserInstance(null);

	// display instance
	if (this == g_pAnarchyManager->GetCanvasManager()->GetDisplayInstance())
		g_pAnarchyManager->GetCanvasManager()->SetDisplayInstance(null);

	// first instance to display
	// do nothing.

	// is input instance
	if (this == g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance())
	{
		if (g_pAnarchyManager->GetInputManager()->GetInputMode())
			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

		g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);	// reduntant?
	}

	// tell the canvas manager we're on our way out
	this->CleanUpTexture();	// m_pTexture will be NULL after this.

	steamapicontext->SteamHTMLSurface()->RemoveBrowser(m_unHandle);

	m_bReadyForNextFrame = false;
	m_bReadyToCopyFrame = false;

	if (m_pLastFrameData)
	{
		free(m_pLastFrameData);
		m_pLastFrameData = null;
	}

	m_pPostData = "";

	/* this was a const char* passed us by the Steamworks HTML Surface API. possibly invalided on their end.
	if (m_pPostData)
	{
		free(m_pPostData);
		m_pPostData = null;
	}
	*/

	delete this;
}

void C_SteamBrowserInstance::Init(std::string id, std::string url, std::string title, const char* pchPostData, int entindex)
{
	std::string goodTitle = (title != "") ? title : "Untitled Steamworks Browser Instance";
	m_title = title;
	m_id = id;

	if (m_id == "")
		m_id = g_pAnarchyManager->GenerateUniqueId();

	m_initialURL = url;
	m_pPostData = (void*)pchPostData;
	m_iOriginalEntIndex = entindex;

	g_pAnarchyManager->GetSteamBrowserManager()->AddFreshSteamBrowserInstance(this);

	// create the texture (each instance has its own texture)
	std::string textureName = "canvas_";
	textureName += m_id;

	int iWidth = (id == "hud") ? AA_HUD_INSTANCE_WIDTH : AA_EMBEDDED_INSTANCE_WIDTH;
	int iHeight = (id == "hud") ? AA_HUD_INSTANCE_HEIGHT : AA_EMBEDDED_INSTANCE_HEIGHT;

	int multiplyer = g_pAnarchyManager->GetDynamicMultiplyer();
	if (!g_pMaterialSystem->IsTextureLoaded(textureName.c_str()))
		m_pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, iWidth * multiplyer, iHeight * multiplyer, IMAGE_FORMAT_BGR888, 1);
	else
	{
		m_pTexture = g_pMaterialSystem->FindTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, false, 1);
		g_pAnarchyManager->GetCanvasManager()->TextureNotDeferred(m_pTexture);
	}

	DevMsg("Init: %s\n", m_id.c_str());

	// get the regen and assign it
	CCanvasRegen* pRegen = g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen();
	m_pTexture->SetTextureRegenerator(pRegen);

	//SteamAPICall_t hAPICall = steamapicontext->SteamHTMLSurface()->CreateBrowser("", "");
	g_pAnarchyManager->GetSteamBrowserManager()->GetBrowserListener()->CreateBrowser(m_id);
	//m_BrowserReadyInitial.Set(hAPICall, this, &C_SteamBrowserInstance::BrowserInstanceBrowserReadyInitial);
}

void C_SteamBrowserInstance::OnBrowserInstanceReady(unsigned int unHandle)
{
	if ( m_unHandle )
		DevMsg("CRITICAL ERROR: Steamworks browser already has a handle!\n");

	m_unHandle = unHandle;

	int iWidth = (m_id == "hud") ? AA_HUD_INSTANCE_WIDTH : AA_EMBEDDED_INSTANCE_WIDTH;
	int iHeight = (m_id == "hud") ? AA_HUD_INSTANCE_HEIGHT : AA_EMBEDDED_INSTANCE_HEIGHT;
	steamapicontext->SteamHTMLSurface()->SetSize(m_unHandle, iWidth, iHeight);

	g_pAnarchyManager->GetSteamBrowserManager()->OnSteamBrowserInstanceCreated(this);
	steamapicontext->SteamHTMLSurface()->LoadURL(m_unHandle, m_initialURL.c_str(), "");
}

void C_SteamBrowserInstance::OnBrowserInstanceWantsToClose()
{
	DevMsg("TODO: A steamworks browser instance wants to close. Acquiesce to the request.\n");
}

bool C_SteamBrowserInstance::IsSelected()
{
	return (this == g_pAnarchyManager->GetSteamBrowserManager()->GetSelectedSteamBrowserInstance());
}

bool C_SteamBrowserInstance::HasFocus()
{
	return (this == g_pAnarchyManager->GetSteamBrowserManager()->GetFocusedSteamBrowserInstance());
}

bool C_SteamBrowserInstance::Focus()
{
	//return g_pAnarchyManager->GetSteamBrowserManager()->SelectSteamBrowserInstance(this);
	//g_pAnarchyManager->GetSteamBrowserManager()->SelectSteamBrowserInstance(this);
	g_pAnarchyManager->GetSteamBrowserManager()->FocusSteamBrowserInstance(this);
	return true;
}

bool C_SteamBrowserInstance::Blur()
{
	if (this == g_pAnarchyManager->GetSteamBrowserManager()->GetFocusedSteamBrowserInstance())
		g_pAnarchyManager->GetSteamBrowserManager()->FocusSteamBrowserInstance(null);
	return true;
}

bool C_SteamBrowserInstance::Select()
{
	return g_pAnarchyManager->GetSteamBrowserManager()->SelectSteamBrowserInstance(this);
}

bool C_SteamBrowserInstance::Deselect()
{
	return g_pAnarchyManager->GetSteamBrowserManager()->SelectSteamBrowserInstance(null);
}

void C_SteamBrowserInstance::Close()
{
	g_pAnarchyManager->GetSteamBrowserManager()->DestroySteamBrowserInstance(this);
}

void C_SteamBrowserInstance::SetUrl(std::string url)
{
	if (!m_pTexture || !m_unHandle)
		return;

	steamapicontext->SteamHTMLSurface()->LoadURL(m_unHandle, url.c_str(), "");
}

void C_SteamBrowserInstance::OnBrowserInstanceStartRequest(const char* pchURL, const char* pchTarget, const char* pchPostData, bool IsRedirect)
{
	std::string urlBuf = pchURL;
	if (urlBuf.find("http://www.aarcadeapicall.com.net.org/?doc=") == 0)
	{
		steamapicontext->SteamHTMLSurface()->AllowStartRequest(m_unHandle, false);

		std::string stuff = urlBuf.substr(43, std::string::npos);

		C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
		C_SteamBrowserInstance* pSteamBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(pEmbeddedInstance);	// FIXME: What if the browser being scraped is NOT a Steam browser instance?

		std::vector<std::string> params;
		if (pSteamBrowserInstance)
			params.push_back(pSteamBrowserInstance->GetURL());
		else
			params.push_back("");

		params.push_back(stuff);
		DevMsg("URL is: %s\n", params[0].c_str());
		C_AwesomiumBrowserInstance* pHudInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
		pHudInstance->DispatchJavaScriptMethod("arcadeHud", "onDOMGot", params);
	}
	else
	{
		//	if (!url || !Q_stricmp(url, "about:blank"))
		//		return true; // this is just webkit loading a new frames contents inside an existing page

		// TODO: This is where URL filtering would be applied.
		steamapicontext->SteamHTMLSurface()->AllowStartRequest(m_unHandle, true);
	}
}

void C_SteamBrowserInstance::OnBrowserInstanceFinishedRequest(const char* pchURL, const char* pchPageTitle)
{
	/*
	if (m_bDying)
	{
		DevMsg("Critical Error: Steamworks browser instance received a callback exactly as it was dying!\n");
		return;
	}
	*/

	// THIS IS NOW TRIGGERED FROM THE HUD
	/*
	if ( m_scraperId == "")	// only continue if we have an active scraper waiting for load finished events. (FIXME: This might change when the UI's address bar gets updated.)
		return;

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");

	std::vector<std::string> params;
	params.push_back(std::string(pchURL));
	params.push_back(m_scraperId);
	params.push_back(m_scraperItemId);
	params.push_back(m_scraperField);
	pHudBrowserInstance->DispatchJavaScriptMethod("arcadeHud", "onBrowserFinishedRequest", params);
	*/
}

//void C_SteamBrowserInstance::BrowserInstanceOpenLinkInTab(HTML_OpenLinkInNewTab_t* pResult)
void C_SteamBrowserInstance::OnBrowserInstanceOpenLinkInTab(const char* pchURL)
{
	DevMsg("TODO: A steamworks browser instance wants to open a URL in a new window. Acquiesce to the request (probably).\n");
}

void C_SteamBrowserInstance::OnBrowserInstanceNeedsPaint(const char* pBGRA, unsigned int unWide, unsigned int unTall, unsigned int unUpdateX, unsigned int unUpdateY, unsigned int unUpdateWide, unsigned int unUpdateTall, unsigned int unScrollX, unsigned int unScrollY, float flPageScale, unsigned int unPageSerial)
{
	// copy the frame to our buffer, if we're not currently reading from it.
	this->CopyLastFrame(pBGRA, unWide, unTall, 4);
}

void C_SteamBrowserInstance::CopyLastFrame(const void* data, unsigned int width, unsigned int height, unsigned int depth)
{
	// NOTE: !m_bReadyForNextFrame is making us ignore all requests to copy until the previous frame is read. (Checking for m_bIsDirty here has the same effect as well.)
	// NOTE 2: m_bCopyingFrame is only really needed if Steamworks browser is allowed to REPLACE the frame that is waiting to render with a newer one. (because the isDirty & readyfornext checks wouldnt' be used in that case.)
	if ( m_bSteamworksCopying || m_bCopyingFrame || !m_bReadyForNextFrame || m_bIsDirty)
		return;
	
	m_bSteamworksCopying = true;
	m_bIsDirty = true;

	m_bReadyForNextFrame = false;
	m_bReadyToCopyFrame = false;	// this is only needed if steamworks browser is allowed to replace the frame that is waiting to render with a newer one.

	void* dest = malloc(width*height*depth);
	Q_memcpy(dest, data, width*height*depth);

	if (m_pLastFrameData)
	{
		free(m_pLastFrameData);	// make sure to set this to non-garbage right away
		m_pLastFrameData = null;
	}
	
	m_pLastFrameData = dest;
	m_bReadyToCopyFrame = true;
	m_bSteamworksCopying = false;

	// if we started dying during our last copy there, really make us die now.
	//if (m_bDefunct)
	//	g_pAnarchyManager->GetSteamBrowserManager()->DestroyDefunctInstance(this);
		//g_pAnarchyManager->GetSteamBrowserManager()->DestroySteamBrowserInstance(this);
		//this->SelfDestruct();
}

void C_SteamBrowserInstance::OnBrowserInstanceURLChanged(const char* pchURL, const char* pchPostData, bool bIsRedirect, const char* pchPageTitle, bool bNewNavigation)
{
	m_URL = pchURL;

	// notify the HUD
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	if (pHudBrowserInstance)
	{
		std::vector<std::string> params;
		params.push_back(m_URL);
		params.push_back(m_scraperId);
		params.push_back(m_scraperItemId);
		params.push_back(m_scraperField);

		pHudBrowserInstance->DispatchJavaScriptMethod("arcadeHud", "onURLChanged", params);
	}
}

void C_SteamBrowserInstance::OnBrowserInstanceChangedTitle(const char* pchTitle)
{
	m_title = (pchTitle == "") ? "Untitled" : pchTitle;

	//pHudInstance->DispatchJavaScriptMethod("arcadeHud", "onTitleChanged", params);	// FIXME: OBSOLETE!  Get rid of that arcadeHud message (probably) and pack titles in with general status updates.
}

void C_SteamBrowserInstance::OnBrowserInstanceSearchResults(unsigned int unResults, unsigned int unCurrentMatch)
{
}

void C_SteamBrowserInstance::OnBrowserInstanceCanGoBackAndForward(bool bCanGoBack, bool bCanGoForward)
{
}

void C_SteamBrowserInstance::OnBrowserInstanceHorizontalScroll(unsigned int unScrollMax, unsigned int unScrollCurrent, float flPageScale, bool bVisible, unsigned int unPageSize)
{
}

void C_SteamBrowserInstance::OnBrowserInstanceVerticalScroll(unsigned int unScrollMax, unsigned int unScrollCurrent, float flPageScale, bool bVisible, unsigned int unPageSize)
{
}

void C_SteamBrowserInstance::OnBrowserInstanceLinkAtPosition(unsigned int x, unsigned int y, const char* pchURL, bool bInput, bool bLiveLink)
{
	// NOTE: x and y are NOT currently set by the Steamworks browser in this callback.
}

void C_SteamBrowserInstance::OnBrowserInstanceJSAlert(const char* pchMessage)
{
}

void C_SteamBrowserInstance::OnBrowserInstanceJSConfirm(const char* pchMessage)
{
}

void C_SteamBrowserInstance::OnBrowserInstanceFileOpenDialog(const char* pchTitle, const char* pchInitialFile)
{
}

void C_SteamBrowserInstance::OnBrowserInstanceNewWindow(const char* pchURL, unsigned int unX, unsigned int unY, unsigned int unWide, unsigned int unTall, unsigned int unNewWindow_BrowserHandle)
{
	DevMsg("Replacing Steamworks web tab with a pop-up that it opened.\n");

	std::string uri = pchURL;
	this->SetUrl(uri);
	steamapicontext->SteamHTMLSurface()->RemoveBrowser(unNewWindow_BrowserHandle);
}

void C_SteamBrowserInstance::OnBrowserInstanceSetCursor(unsigned int eMouseCursor)
{
}

void C_SteamBrowserInstance::OnBrowserInstanceStatusText(const char* pchMsg)
{
}

void C_SteamBrowserInstance::OnBrowserInstanceShowToolTip(const char* pchMsg)
{
}

void C_SteamBrowserInstance::OnBrowserInstanceUpdateToolTip(const char* pchMsg)
{
}

void C_SteamBrowserInstance::OnBrowserInstanceHideTollTip()
{
}

void C_SteamBrowserInstance::OnMouseMove(float x, float y)
{
	if (!m_unHandle)
		return;

	if (g_pAnarchyManager->GetSteamBrowserManager()->GetSelectedSteamBrowserInstance() != this)
		return;

	unsigned int width = (m_id == "hud") ? AA_HUD_INSTANCE_WIDTH : AA_EMBEDDED_INSTANCE_WIDTH;
	unsigned int height = (m_id == "hud") ? AA_HUD_INSTANCE_HEIGHT : AA_EMBEDDED_INSTANCE_HEIGHT;

	int goodX = (width * x) / 1;
	int goodY = (height * y) / 1;
	steamapicontext->SteamHTMLSurface()->MouseMove(m_unHandle, goodX, goodY);
}

void C_SteamBrowserInstance::OnMousePressed(vgui::MouseCode code)
{
	if (!m_unHandle)
		return;

	ISteamHTMLSurface::EHTMLMouseButton goodButton;

	switch (code)
	{
		case vgui::MouseCode::MOUSE_LEFT:
			goodButton = ISteamHTMLSurface::EHTMLMouseButton::eHTMLMouseButton_Left;
			break;

		case vgui::MouseCode::MOUSE_RIGHT:
			goodButton = ISteamHTMLSurface::EHTMLMouseButton::eHTMLMouseButton_Right;
			break;

		case vgui::MouseCode::MOUSE_MIDDLE:
			goodButton = ISteamHTMLSurface::EHTMLMouseButton::eHTMLMouseButton_Middle;
			break;
	}

	steamapicontext->SteamHTMLSurface()->MouseDown(m_unHandle, goodButton);
}

void C_SteamBrowserInstance::OnMouseReleased(vgui::MouseCode code)
{
	if (!m_unHandle)
		return;

	ISteamHTMLSurface::EHTMLMouseButton goodButton;

	switch (code)
	{
	case vgui::MouseCode::MOUSE_LEFT:
		goodButton = ISteamHTMLSurface::EHTMLMouseButton::eHTMLMouseButton_Left;
		break;

	case vgui::MouseCode::MOUSE_RIGHT:
		goodButton = ISteamHTMLSurface::EHTMLMouseButton::eHTMLMouseButton_Right;
		break;

	case vgui::MouseCode::MOUSE_MIDDLE:
		goodButton = ISteamHTMLSurface::EHTMLMouseButton::eHTMLMouseButton_Middle;
		break;
	}

	steamapicontext->SteamHTMLSurface()->MouseUp(m_unHandle, goodButton);
}

void C_SteamBrowserInstance::Update()
{
	if (g_pAnarchyManager->GetSuspendEmbedded())
		return;

	if (g_pAnarchyManager->IsPaused())
		return;

	if (!m_unHandle)
		return;

	//if (m_info->state == 1)
	//if (m_pLastFrameData)
	this->OnProxyBind(null);
}

void C_SteamBrowserInstance::CopyLastFrame(unsigned char* dest, unsigned int width, unsigned int height, size_t pitch, unsigned int depth)
{
	if (!m_unHandle)
		return;

	if (m_bCopyingFrame || !m_bReadyToCopyFrame || g_pAnarchyManager->GetSuspendEmbedded())
		return;

	if (g_pAnarchyManager->IsPaused())
		return;

//	DevMsg("SteamBrowserInstance: Start copy\n");

	m_bCopyingFrame = true;
	m_bReadyToCopyFrame = false;
	//memcpy(dest, m_pLastFrameData, pitch * height);
	Q_memcpy(dest, m_pLastFrameData, pitch * height);

//	if (m_info->videoformat == RETRO_PIXEL_FORMAT_RGB565)
//		this->ResizeFrameFromRGB565(m_info->lastframedata, dest, m_info->lastframewidth, m_info->lastframeheight, m_info->lastframepitch, 3, width, height, pitch, depth);
//	else if (m_info->videoformat == RETRO_PIXEL_FORMAT_XRGB8888)
//		this->ResizeFrameFromXRGB8888(m_info->lastframedata, dest, m_info->lastframewidth, m_info->lastframeheight, m_info->lastframepitch, 4, width, height, pitch, depth);
//	else
//		this->ResizeFrameFromRGB1555(m_info->lastframedata, dest, m_info->lastframewidth, m_info->lastframeheight, m_info->lastframepitch, 3, width, height, pitch, depth);

	m_bReadyForNextFrame = true;
	m_bCopyingFrame = false;
	m_bIsDirty = false;

	// if we started dying during our last copy there, really make us die now.
	//if (m_bDying)
		//this->SelfDestruct();

//	DevMsg("SteamBrowserInstance: Finish copy\n");
}

void C_SteamBrowserInstance::OnProxyBind(C_BaseEntity* pBaseEntity)
{
	/*
	if (m_bDefunct)
	{
		g_pAnarchyManager->GetSteamBrowserManager()->DestroyDefunctInstance(this);
		return;
	}
	*/

	if (!m_unHandle)
		return;

	if (g_pAnarchyManager->IsPaused())
		return;

	if (g_pAnarchyManager->GetSuspendEmbedded())
		return;

//	if (m_id == "images")
//		return;

	/*
	if ( pBaseEntity )
	DevMsg("WebTab: OnProxyBind: %i\n", pBaseEntity->entindex());
	else
	DevMsg("WebTab: OnProxyBind\n");
	*/

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
		if (g_pAnarchyManager->GetCanvasManager()->ShouldRender(this) && m_bIsDirty && m_bReadyToCopyFrame && !m_bSteamworksCopying)
		{
		//	if (g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(this))
			//	DevMsg("Render PRIORITY (STEAM)!\n");
		//	else
			//	DevMsg("Render STEAM\n");
			Render();
		}
		else
		{
			
			//DevMsg("Don't render STEAM!\n");
		}
	}
}

void C_SteamBrowserInstance::Render()
{
//	if (m_id == "images")
	//	return;
	//DevMsg("Rendering texture: %s\n", m_pTexture->GetName());
	//	DevMsg("Render Web Tab: %s\n", this->GetTexture()->Ge>GetId().c_str());
	//DevMsg("WebTab: Render: %s on %i\n", m_id.c_str(), gpGlobals->framecount);

	//if (m_bIsDirty && m_bReadyToCopyFrame)	// THIS IS THE ABSOLUTE LATEST THAT A RENDER CALL CAN BE ABORTED WITHOUT FLICKER SIDE-EFFECTS!@@@@@@
	//{
		g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen()->SetEmbeddedInstance(this);
		m_pTexture->Download();
		g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen()->SetEmbeddedInstance(null);
	//}

	m_iLastRenderedFrame = gpGlobals->framecount;

	if (g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(this))
		g_pAnarchyManager->GetCanvasManager()->SetLastPriorityRenderedFrame(gpGlobals->framecount);
	else
		g_pAnarchyManager->GetCanvasManager()->SetLastRenderedFrame(gpGlobals->framecount);
}

void C_SteamBrowserInstance::RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect)
{
	if (!m_unHandle || !m_pLastFrameData)
		return;

	if (g_pAnarchyManager->IsPaused())
		return;

	if (g_pAnarchyManager->GetSuspendEmbedded())
		return;

	//if (m_info->state == 1)
	//DevMsg("copy last frame\n");
		this->CopyLastFrame(pVTFTexture->ImageData(0, 0, 0), pSubRect->width, pSubRect->height, pSubRect->width * 4, 4);
//		DevMsg("Done copying frame.\n");
}

C_InputListener* C_SteamBrowserInstance::GetInputListener()
{
	return g_pAnarchyManager->GetSteamBrowserManager()->GetInputListener();
}

void C_SteamBrowserInstance::CleanUpTexture()
{
	if (m_pTexture)
	{
		m_pTexture->SetTextureRegenerator(null);
		g_pAnarchyManager->GetCanvasManager()->UnreferenceEmbeddedInstance(this);
		g_pAnarchyManager->GetCanvasManager()->UnreferenceTexture(m_pTexture);
		g_pAnarchyManager->GetCanvasManager()->DoOrDeferTextureCleanup(m_pTexture);
		m_pTexture = null;
	}
}

C_EmbeddedInstance* C_SteamBrowserInstance::GetParentSelectedEmbeddedInstance()
{
	return g_pAnarchyManager->GetSteamBrowserManager()->GetSelectedSteamBrowserInstance();
}

void C_SteamBrowserInstance::OnKeyCodePressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState)
{
	if (!m_unHandle)
		return;

	// don't send alt button for now (it can cause crashes sometimes? (PROBABLY AN OBSOLETE ASSUMPTION!)
	//if (code == KEY_LALT || code == KEY_RALT || code == BUTTON_CODE_NONE || code == BUTTON_CODE_INVALID)
		//return;

	if (code == BUTTON_CODE_NONE || code == BUTTON_CODE_INVALID)
		return;

	// FIXME: Can crash at the following line, probably due to a Steamworks browser crash!! (from from the CODE_NONE and CODE_INVALID checks that didn't used to be included above...)
	// NO still crashes for multiple users.  Insufficent checks, need a better fix or figure out wtf is going on with this call.
	// -TRY: Making GetKeyModifiersAlt a MEMBER method.
	// -TRY: Getting the key modifiers BEFORE the KeyDown call.
	// - TRY: Printing debug info BEFORE the KeyDown call.
	// -TRY: Making KeyCode_VGUIToVirtualKey a MEMBER method.
	// -TRY: confirming that m_unHandle is valid. (with m_bDying)

	int iModifiers = this->GetKeyModifiersAlt();
	int iVirtualKeyCode = this->KeyCode_VGUIToVirtualKey(code);
	DevMsg("Sending keypress to browser\n");
	steamapicontext->SteamHTMLSurface()->KeyDown(m_unHandle, iVirtualKeyCode, (ISteamHTMLSurface::EHTMLKeyModifiers)iModifiers);

	std::string s_output = this->GetOutput(code, bShiftState, bCtrlState, bAltState);
	if (s_output != "")
	{
		char a = s_output.at(0);
		const char *b = &a;

		wchar_t value;
		mbtowc(&value, b, 1);
		steamapicontext->SteamHTMLSurface()->KeyChar(m_unHandle, value, (ISteamHTMLSurface::EHTMLKeyModifiers)iModifiers);
	}
}

void C_SteamBrowserInstance::OnKeyCodeReleased(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState)
{
	if (!m_unHandle)
		return;

	// don't send alt button for now (it can cause crashes sometimes?
	if (code == KEY_LALT || code == KEY_RALT || code == BUTTON_CODE_NONE || code == BUTTON_CODE_INVALID)
		return;

	steamapicontext->SteamHTMLSurface()->KeyUp(m_unHandle, this->KeyCode_VGUIToVirtualKey(code), (ISteamHTMLSurface::EHTMLKeyModifiers)this->GetKeyModifiersAlt());
}

void C_SteamBrowserInstance::InjectJavaScript(std::string code)
{
	if (!m_unHandle)
		return;

	steamapicontext->SteamHTMLSurface()->ExecuteJavascript(m_unHandle, code.c_str());
}