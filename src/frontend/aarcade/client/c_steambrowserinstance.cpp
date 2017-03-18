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

vgui::KeyCode KeyCode_VirtualKeyToVGUI(int key)
{
	// Some tools load vgui for localization and never use input
	if (!g_pInputSystem)
		return KEY_NONE;
	return g_pInputSystem->VirtualKeyToButtonCode(key);
}

int KeyCode_VGUIToVirtualKey(vgui::KeyCode code)
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
int GetKeyModifiersAlt()
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

C_SteamBrowserInstance::C_SteamBrowserInstance() : m_StartRequest(this, &C_SteamBrowserInstance::BrowserInstanceStartRequest),
m_FinishedRequest(this, &C_SteamBrowserInstance::BrowserInstanceFinishedRequest),
m_NeedsPaint(this, &C_SteamBrowserInstance::BrowserInstanceNeedsPaint),
m_NewWindow(this, &C_SteamBrowserInstance::BrowserPopupHTMLWindow),
m_URLChanged(this, &C_SteamBrowserInstance::BrowserURLChanged),
m_ChangeTitle(this, &C_SteamBrowserInstance::BrowserSetHTMLTitle)
//m_FinishedRequest(this, &C_SteamBrowserInstance::BrowserFinishedRequest)
//m_StatusText(this, &C_SteamBrowserInstance::BrowserStatusText)
{
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
	m_iOriginalEntIndex = -1;
}

C_SteamBrowserInstance::~C_SteamBrowserInstance()
{
	DevMsg("SteamBrowserInstance: Destructor\n");

	if (m_id == "images" || m_id == "hud")
	{
		DevMsg("ERROR: C_SteamBrowserInstance with a reserved or invalid name!!\n");
		return;
	}
	
	if (m_pTexture)
	{
		m_pTexture->SetTextureRegenerator(null);

		g_pAnarchyManager->GetCanvasManager()->UnreferenceTexture(m_pTexture);
		//m_pTexture->SetTextureRegenerator(null);
		m_pTexture->DecrementReferenceCount();
		m_pTexture->DeleteIfUnreferenced();
		m_pTexture = null;
	}
}

void C_SteamBrowserInstance::SelfDestruct()
{
	DevMsg("SteamBrowserInstance: SelfDestruct %s\n", m_id.c_str());
	g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen()->NotifyInstanceAboutToDie(this);
	steamapicontext->SteamHTMLSurface()->RemoveBrowser(m_unHandle);// m_unBrowserHandle);

	m_bReadyForNextFrame = false;
	m_bReadyToCopyFrame = false;
	
	if (m_pLastFrameData)
		free(m_pLastFrameData);

	if (m_pPostData)
		free(m_pPostData);

	delete this;
}

void C_SteamBrowserInstance::Init(std::string id, std::string url, const char* pchPostData, int entindex)
{
	m_id = id;

	if (m_id == "")
		m_id = g_pAnarchyManager->GenerateUniqueId();

	m_initialURL = url;
	m_pPostData = (void*)pchPostData;
	m_iOriginalEntIndex = entindex;

	g_pAnarchyManager->GetSteamBrowserManager()->AddFreshSteamBrowserInstance(this);

	SteamAPICall_t hAPICall = steamapicontext->SteamHTMLSurface()->CreateBrowser("", "");
	m_CreateBrowserInstance.Set(hAPICall, this, &C_SteamBrowserInstance::OnBrowserInstanceCreated);

	// create the texture (each instance has its own texture)
	std::string textureName = "canvas_";
	textureName += m_id;// pSelf->GetId();// m_id;

	int iWidth = (id == "hud") ? AA_HUD_INSTANCE_WIDTH : AA_EMBEDDED_INSTANCE_WIDTH;
	int iHeight = (id == "hud") ? AA_HUD_INSTANCE_HEIGHT : AA_EMBEDDED_INSTANCE_HEIGHT;
	//int iWidth = 1920;
	//int iHeight = 1080;

	//m_pTexture 

	//m_pTexture = g_pMaterialSystem->FindTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, false, 1);

	if (!g_pMaterialSystem->IsTextureLoaded(textureName.c_str()))
		m_pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, iWidth, iHeight, IMAGE_FORMAT_BGR888, 1);
	else
		m_pTexture = g_pMaterialSystem->FindTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, false, 1);

	//m_pTexture = pTexture;
	//pSelf->SetTexture(pTexture);
	DevMsg("Init: %s\n", m_id.c_str());
	// get the regen and assign it
	CCanvasRegen* pRegen = g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen();
	//pRegen->SetEmbeddedInstance(this);
	m_pTexture->SetTextureRegenerator(pRegen);
}

void C_SteamBrowserInstance::OnBrowserInstanceCreated(HTML_BrowserReady_t *pResult, bool bIOFailure)
{
	if (m_unHandle)
		return;
	/*
	C_SteamBrowserInstance* pSelf = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pResult->unBrowserHandle);
	
	if (!pSelf)
		pSelf = g_pAnarchyManager->GetSteamBrowserManager()->GetPendingSteamBrowserInstance();

	if (!pSelf)
	{
		DevMsg("Error: New Steam Browser instances created with no pending slots waiting for it!\n");
		return;
	}
	*/

	DevMsg("SteamworksBrowser: OnBrowserCreated - %s for %s\n", pResult->GetCallbackName(), m_id.c_str());

	/*
	// create the texture (each instance has its own texture)
	std::string textureName = "canvas_";
	textureName += m_id;// pSelf->GetId();// m_id;

	int iWidth = 1280;// g_pAnarchyManager->GetWebManager()->GetWebSurfaceWidth();
	int iHeight = 720;// g_pAnarchyManager->GetWebManager()->GetWebSurfaceHeight();
	//int iWidth = 1920;
	//int iHeight = 1080;

	//m_pTexture 
	ITexture* pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, iWidth, iHeight, IMAGE_FORMAT_BGR888, 1);
	m_pTexture = pTexture;
	//pSelf->SetTexture(pTexture);

	// get the regen and assign it
	CCanvasRegen* pRegen = g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen();
	//pRegen->SetEmbeddedInstance(this);
	pTexture->SetTextureRegenerator(pRegen);
	*/


	// tell the input manager that the steam browser instance is active
	//C_InputListenerSteamBrowser* pListener = g_pAnarchyManager->GetSteamBrowserManager()->GetInputListener();
	//g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(m_pTexture);


	//g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, this);// (C_InputListener*)pListener);
	
	//m_unBrowserHandle = pResult->unBrowserHandle;
	m_unHandle = pResult->unBrowserHandle;
	//pSelf->SetHandle(pResult->unBrowserHandle);
	
	int iWidth = (m_id == "hud") ? AA_HUD_INSTANCE_WIDTH : AA_EMBEDDED_INSTANCE_WIDTH;
	int iHeight = (m_id == "hud") ? AA_HUD_INSTANCE_HEIGHT : AA_EMBEDDED_INSTANCE_HEIGHT;
	steamapicontext->SteamHTMLSurface()->SetSize(m_unHandle, iWidth, iHeight);

	g_pAnarchyManager->GetSteamBrowserManager()->OnSteamBrowserInstanceCreated(this);// pSelf);
	steamapicontext->SteamHTMLSurface()->LoadURL(m_unHandle, m_initialURL.c_str(), "");
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
	steamapicontext->SteamHTMLSurface()->LoadURL(m_unHandle, url.c_str(), "");
}

void C_SteamBrowserInstance::BrowserInstanceStartRequest(HTML_StartRequest_t *pCmd)
{
	//if (g_pAnarchyManager->IsPaused())
	//	return;

	//	if (m_unHandle != pCmd->unBrowserHandle)
	//{
	//C_SteamBrowserInstance* pExists = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pCmd->unBrowserHandle);
	//	if ( !pExists )
	//		{
	//		steamapicontext->SteamHTMLSurface()->AllowStartRequest(pCmd->unBrowserHandle, false);
	//steamapicontext->SteamHTMLSurface()->RemoveBrowser(pCmd->unBrowserHandle);
	//		return;
	//	}

	//DevMsg("Start Request Detected By %s\n", m_id.c_str());
	//C_SteamBrowserInstance* pSelf = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pCmd->unBrowserHandle);
	std::string urlBuf = pCmd->pchURL;

	if (m_unHandle == pCmd->unBrowserHandle)
	{
		if (urlBuf.find("http://www.aarcadeapicall.com.net.org/?doc=") == 0)
		{
			std::string stuff = urlBuf.substr(43, std::string::npos);

			C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
			C_SteamBrowserInstance* pSteamBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(pEmbeddedInstance);	// FIXME: What if the browser being scraped is NOT a Steam browser instance?

			std::vector<std::string> params;
			if (pSteamBrowserInstance)
				params.push_back(pSteamBrowserInstance->GetURL());
			else
				params.push_back("");

			params.push_back(stuff);

			C_AwesomiumBrowserInstance* pHudInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
			pHudInstance->DispatchJavaScriptMethod("arcadeHud", "onDOMGot", params);
		}
	}

	if (urlBuf.find("http://www.aarcadeapicall.com.net.org/?doc=") == 0)
	{
		// if ANY browser returns allow, then it is allowed.
		steamapicontext->SteamHTMLSurface()->AllowStartRequest(pCmd->unBrowserHandle, false);
		return;
	}

	bool bRes = OnStartRequest(pCmd->pchURL, pCmd->pchTarget, pCmd->pchPostData, pCmd->bIsRedirect);
	steamapicontext->SteamHTMLSurface()->AllowStartRequest(pCmd->unBrowserHandle, bRes);
}

//-----------------------------------------------------------------------------
// Purpose: browser wants to start loading this url, do we let it?
//-----------------------------------------------------------------------------
bool C_SteamBrowserInstance::OnStartRequest(const char *url, const char *target, const char *pchPostData, bool bIsRedirect)
{
	return true;

//	if (!url || !Q_stricmp(url, "about:blank"))
//		return true; // this is just webkit loading a new frames contents inside an existing page

	//HideFindDialog();
	// see if we have a custom handler for this
	/*
	bool bURLHandled = false;
	for (int i = 0; i < m_CustomURLHandlers.Count(); i++)
	{
	if (!Q_strnicmp(m_CustomURLHandlers[i].url, url, Q_strlen(m_CustomURLHandlers[i].url)))
	{
	// we have a custom handler
	Panel *targetPanel = m_CustomURLHandlers[i].hPanel;
	if (targetPanel)
	{
	PostMessage(targetPanel, new KeyValues("CustomURL", "url", m_CustomURLHandlers[i].url));
	}

	bURLHandled = true;
	}
	}

	if (bURLHandled)
	return false;
	*/

	/*
	if (m_bNewWindowsOnly && bIsRedirect)
	{
	if (target && (!Q_stricmp(target, "_blank") || !Q_stricmp(target, "_new"))) // only allow NEW windows (_blank ones)
	{
	return true;
	}
	else
	{
	return false;
	}
	}

	if (target && !Q_strlen(target))
	{
	m_sCurrentURL = url;

	KeyValues *pMessage = new KeyValues("OnURLChanged");
	pMessage->SetString("url", url);
	pMessage->SetString("postdata", pchPostData);
	pMessage->SetInt("isredirect", bIsRedirect ? 1 : 0);

	PostActionSignal(pMessage);
	}
	*/

//	DevMsg("SteamworksBrowser: OnStartRequest - %s\n", url);
	

	//return true;
}

//-----------------------------------------------------------------------------
// Purpose: finished loading this page
//-----------------------------------------------------------------------------
void C_SteamBrowserInstance::BrowserInstanceFinishedRequest(HTML_FinishedRequest_t *pCmd)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	return;	// DISABLED for consistent usage of onurlchanged for meta scraping

	if (m_unHandle != pCmd->unBrowserHandle || m_scraperId == "")	// only continue if we have an active scraper waiting for load finished events. (FIXME: This might change when the UI's address bar gets updated.)
		return;

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");

	std::vector<std::string> params;
	params.push_back(std::string(pCmd->pchURL));
	params.push_back(m_scraperId);
	params.push_back(m_scraperItemId);
	params.push_back(m_scraperField);
	pHudBrowserInstance->DispatchJavaScriptMethod("arcadeHud", "onBrowserFinishedRequest", params);
}

//void C_SteamBrowserInstance::OnFinishRequest(const char *url, const char *pageTitle, const CUtlMap < CUtlString, CUtlString > &headers)
//{
//	if (!url || !Q_stricmp(url, "about:blank"))
//		return;
//
	//DevMsg("Request finished!!\n");
	//	steamapicontext->SteamHTMLSurface()->
	//	m_unBrowserHandle
//}

//-----------------------------------------------------------------------------
// Purpose: we have a new texture to update
//-----------------------------------------------------------------------------
void C_SteamBrowserInstance::BrowserInstanceNeedsPaint(HTML_NeedsPaint_t *pCallback)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	if (m_unHandle != pCallback->unBrowserHandle)
		return;

	/*
	int tw = 0, tt = 0;
	if ( m_iHTMLTextureID != 0 )
	{
		tw = m_allocedTextureWidth;
		tt = m_allocedTextureHeight;
	}

	if ( m_iHTMLTextureID != 0 && ( ( _vbar->IsVisible() && pCallback->unScrollY > 0 && abs( (int)pCallback->unScrollY - m_scrollVertical.m_nScroll) > 5 ) || ( _hbar->IsVisible() && pCallback->unScrollX > 0 && abs( (int)pCallback->unScrollX - m_scrollHorizontal.m_nScroll ) > 5 ) ) )
	{
		m_bNeedsFullTextureUpload = true;
		return;
	}

	// update the vgui texture
	if ( m_bNeedsFullTextureUpload || m_iHTMLTextureID == 0  || tw != (int)pCallback->unWide || tt != (int)pCallback->unTall )
	{
		m_bNeedsFullTextureUpload = false;
		if ( m_iHTMLTextureID != 0 )
			surface()->DeleteTextureByID( m_iHTMLTextureID );

		// if the dimensions changed we also need to re-create the texture ID to support the overlay properly (it won't resize a texture on the fly, this is the only control that needs
		//   to so lets have a tiny bit more code here to support that)
		m_iHTMLTextureID = surface()->CreateNewTextureID( true );
		surface()->DrawSetTextureRGBAEx( m_iHTMLTextureID, (const unsigned char *)pCallback->pBGRA, pCallback->unWide, pCallback->unTall, IMAGE_FORMAT_BGRA8888 );// BR FIXME - this call seems to shift by some number of pixels?
		m_allocedTextureWidth = pCallback->unWide;
		m_allocedTextureHeight = pCallback->unTall;
	}
	else if ( (int)pCallback->unUpdateWide > 0 && (int)pCallback->unUpdateTall > 0 )
	{
		// same size texture, just bits changing in it, lets twiddle
		surface()->DrawUpdateRegionTextureRGBA( m_iHTMLTextureID, pCallback->unUpdateX, pCallback->unUpdateY, (const unsigned char *)pCallback->pBGRA, pCallback->unUpdateWide, pCallback->unUpdateTall, IMAGE_FORMAT_BGRA8888 );
	}
	else
	{
		surface()->DrawSetTextureRGBAEx( m_iHTMLTextureID, (const unsigned char *)pCallback->pBGRA,pCallback->unWide, pCallback->unTall, IMAGE_FORMAT_BGRA8888 );
	}

	// need a paint next time
	Repaint();
	*/
	//C_SteamBrowserInstance* pSelf = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(pCallback->unBrowserHandle);
	OnNeedsPaint(pCallback->pBGRA, pCallback->unWide, pCallback->unTall, 4);
}

void C_SteamBrowserInstance::OnNeedsPaint(const void* data, unsigned int width, unsigned int height, unsigned int depth)
{
	//DevMsg("Needs paint bra\n");
	
	if (!m_bReadyForNextFrame || m_bCopyingFrame)
		return;

	//DevMsg("SteamBrowserInstance: OnNeedsPaint started\n");

	m_bIsDirty = true;

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
	/*
	info->lastframewidth = width;
	info->lastframeheight = height;
	info->lastframepitch = pitch;

	info->readytocopyframe = true;
	*/

	m_bReadyToCopyFrame = true;

	//DevMsg("SteamBrowserInstance: OnNeedsPaint finished\n");

	//this->CopyLastFrame()
}

//-----------------------------------------------------------------------------
// Purpose: container class for any external popup windows the browser requests
//-----------------------------------------------------------------------------
class HTMLPopup : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(HTMLPopup, vgui::Frame);
	class PopupHTML : public vgui::HTML
	{
		DECLARE_CLASS_SIMPLE(PopupHTML, vgui::HTML);
	public:
		PopupHTML(Frame *parent, const char *pchName, bool allowJavaScript, bool bPopupWindow) : HTML(parent, pchName, allowJavaScript, bPopupWindow) { m_pParent = parent; }

		virtual void OnSetHTMLTitle(const char *pchTitle)
		{
			BaseClass::OnSetHTMLTitle(pchTitle);
			m_pParent->SetTitle(pchTitle, true);
		}

	private:
		Frame *m_pParent;
	};
public:
	HTMLPopup(Panel *parent, const char *pchURL, const char *pchTitle) : Frame(NULL, "HtmlPopup", true)
	{
		m_pHTML = new PopupHTML(this, "htmlpopupchild", true, true);
		m_pHTML->OpenURL(pchURL, NULL, false);
		SetTitle(pchTitle, true);
	}

	~HTMLPopup()
	{
	}

	enum
	{
		vert_inset = 40,
		horiz_inset = 6
	};

	void PerformLayout()
	{
		BaseClass::PerformLayout();
		int wide, tall;
		GetSize(wide, tall);
		m_pHTML->SetPos(horiz_inset, vert_inset);
		m_pHTML->SetSize(wide - horiz_inset * 2, tall - vert_inset * 2);
	}

	void SetBounds(int x, int y, int wide, int tall)
	{
		BaseClass::SetBounds(x, y, wide + horiz_inset * 2, tall + vert_inset * 2);
	}

	MESSAGE_FUNC(OnCloseWindow, "OnCloseWindow")
	{
		Close();
	}
private:
	PopupHTML *m_pHTML;
};

void C_SteamBrowserInstance::BrowserURLChanged(HTML_URLChanged_t *pCmd)
{
	//if (g_pAnarchyManager->IsPaused())
	//	return;

	if (pCmd->unBrowserHandle != m_unHandle)
		return;

	m_URL = pCmd->pchURL;

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

	//)
	//m_URL

	//m_title = pCmd->pchPageTitle;
}

//void C_SteamBrowserInstance::BrowserStatusText(HTML_StatusText_t *pCmd)

void C_SteamBrowserInstance::BrowserSetHTMLTitle(HTML_ChangedTitle_t *pCmd)
{
	if (pCmd->unBrowserHandle != m_unHandle)
		return;

	std::vector<std::string> params;
	params.push_back(pCmd->pchTitle);

	C_AwesomiumBrowserInstance* pHudInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	pHudInstance->DispatchJavaScriptMethod("arcadeHud", "onTitleChanged", params);

	//DevMsg("Response is: %s\n", pCmd->pchTitle);
}



//#include "ienginevgui.h"
//#include "c_inputslate.h"
void C_SteamBrowserInstance::BrowserPopupHTMLWindow(HTML_NewWindow_t *pCmd)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	if (pCmd->unBrowserHandle != m_unHandle)
		return;

	std::string uri = pCmd->pchURL;
	this->SetUrl(uri);
	steamapicontext->SteamHTMLSurface()->RemoveBrowser(pCmd->unNewWindow_BrowserHandle);


	// if this URL is allowed, just change US to it (as long as we are not the HUD)

	//DevMsg("flag for: \n");
	return;
//	if (pCmd->unBrowserHandle != m_unHandle)
	//	return;
	
	/*
	vgui::Panel* panel = InputSlate->GetPanel();
	HTMLPopup *p = new HTMLPopup(panel, pCmd->pchURL, "");

	//HTMLPopup(Panel *parent, const char *pchURL, const char *pchTitle)
	//HTMLPopup *p = new HTMLPopup(enginevgui->GetPanel(PANEL_ROOT), pCmd->pchURL, "");

	int wide = pCmd->unWide;
	int tall = pCmd->unTall;
	if (wide == 0 || tall == 0)
	{
		wide = 640;// MAX(640, panel->GetWide());
		tall = 480;// MAX(480, panel->GetTall());
	}

	p->SetBounds(pCmd->unX, pCmd->unY, wide, tall);
	p->SetDeleteSelfOnClose(true);
	if (pCmd->unX == 0 || pCmd->unY == 0)
		p->MoveToCenterOfScreen();
	p->Activate();
	*/
}

void C_SteamBrowserInstance::OnMouseMove(float x, float y)
{
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

void C_SteamBrowserInstance::ResizeFrameFromRGB565(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth)
{
//	uint uId = ThreadGetCurrentId();
//	C_SteamBrowserInstance* pSteamBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(uId);
//	SteamBrowserInstanceInfo_t* info = pSteamBrowserInstance->GetInfo();

	if (!m_pLastFrameData)	// should be checking psrc or pdest, not the member variable.
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

void C_SteamBrowserInstance::ResizeFrameFromRGB1555(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth)
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

void C_SteamBrowserInstance::ResizeFrameFromXRGB8888(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth)
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

void C_SteamBrowserInstance::CopyLastFrame(unsigned char* dest, unsigned int width, unsigned int height, size_t pitch, unsigned int depth)
{
	if (m_bCopyingFrame || !m_bReadyToCopyFrame || g_pAnarchyManager->GetSuspendEmbedded())
		return;

	if (g_pAnarchyManager->IsPaused())
		return;

//	DevMsg("SteamBrowserInstance: Start copy\n");

	m_bCopyingFrame = true;
	m_bReadyToCopyFrame = false;
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

//	DevMsg("SteamBrowserInstance: Finish copy\n");
}

void C_SteamBrowserInstance::OnProxyBind(C_BaseEntity* pBaseEntity)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	if (g_pAnarchyManager->GetSuspendEmbedded())
		return;

	if (!m_unHandle)
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
		if (g_pAnarchyManager->GetCanvasManager()->ShouldRender(this) && m_bIsDirty && m_bReadyToCopyFrame)
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
	//}

	m_iLastRenderedFrame = gpGlobals->framecount;

	if (g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(this))
		g_pAnarchyManager->GetCanvasManager()->SetLastPriorityRenderedFrame(gpGlobals->framecount);
	else
		g_pAnarchyManager->GetCanvasManager()->SetLastRenderedFrame(gpGlobals->framecount);
}

void C_SteamBrowserInstance::RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect)
{
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

C_EmbeddedInstance* C_SteamBrowserInstance::GetParentSelectedEmbeddedInstance()
{
	return g_pAnarchyManager->GetSteamBrowserManager()->GetSelectedSteamBrowserInstance();
}

void C_SteamBrowserInstance::OnKeyCodePressed(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState)
{
	// don't send alt button for now (it can cause crashes sometimes?
	if (code == KEY_LALT || code == KEY_RALT)
		return;

	// FIXME: Can crash at the following line, probably due to a Steamworks browser crash!!
	steamapicontext->SteamHTMLSurface()->KeyDown(m_unHandle, KeyCode_VGUIToVirtualKey(code), (ISteamHTMLSurface::EHTMLKeyModifiers)GetKeyModifiersAlt());

	std::string s_output = this->GetOutput(code, bShiftState, bCtrlState, bAltState);
	if (s_output != "")
	{
		char a = s_output.at(0);
		const char *b = &a;

		wchar_t value;
		mbtowc(&value, b, 1);
		steamapicontext->SteamHTMLSurface()->KeyChar(m_unHandle, value, (ISteamHTMLSurface::EHTMLKeyModifiers)GetKeyModifiersAlt());
	}
}

void C_SteamBrowserInstance::OnKeyCodeReleased(vgui::KeyCode code, bool bShiftState, bool bCtrlState, bool bAltState, bool bWinState, bool bAutorepeatState)
{
	// don't send alt button for now (it can cause crashes sometimes?
	if (code == KEY_LALT || code == KEY_RALT)
		return;

	steamapicontext->SteamHTMLSurface()->KeyUp(m_unHandle, KeyCode_VGUIToVirtualKey(code), (ISteamHTMLSurface::EHTMLKeyModifiers)GetKeyModifiersAlt());
}

void C_SteamBrowserInstance::InjectJavaScript(std::string code)
{
//	steamapicontext->SteamHTMLSurface()->
	steamapicontext->SteamHTMLSurface()->ExecuteJavascript(m_unHandle, code.c_str());
}

/*
wchar_t C_SteamBrowserInstance::GetTypedChar(vgui::KeyCode code)
{
	//wchar_t unichar = L'';
	//wchar_t wc = g_pInputSystem->ButtonCodeToString(code)[0];
	//wchar_t unichar = g_pInputSystem->ButtonCodeToString(code);
	//steamapicontext->SteamHTMLSurface()->KeyChar(m_unHandle, wc, (ISteamHTMLSurface::EHTMLKeyModifiers)GetKeyModifiersAlt());
	//g_pInputSystem->ButtonCodeToVirtualKey(code);

}
*/