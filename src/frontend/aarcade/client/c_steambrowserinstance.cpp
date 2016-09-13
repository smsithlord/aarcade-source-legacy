// ;..\..\portaudio\lib\portaudio_x86.lib

#include "cbase.h"
#include "aa_globals.h"

//#include "aa_globals.h"
//#include "c_steambrowserinstance.h"
#include "c_inputlistenersteambrowser.h"
#include "c_anarchymanager.h"
#include "../../../public/vgui_controls/Controls.h"  
#include "vgui/IInput.h"
#include "c_canvasregen.h"
#include "c_embeddedinstance.h"
#include <mutex>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_SteamBrowserInstance::C_SteamBrowserInstance() : m_StartRequest(this, &C_SteamBrowserInstance::BrowserInstanceStartRequest),
m_FinishedRequest(this, &C_SteamBrowserInstance::BrowserInstanceFinishedRequest),
m_NeedsPaint(this, &C_SteamBrowserInstance::BrowserInstanceNeedsPaint)
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
}

C_SteamBrowserInstance::~C_SteamBrowserInstance()
{
	DevMsg("SteamBrowserInstance: Destructor\n");
	
	if (m_pTexture)
	{
		m_pTexture->SetTextureRegenerator(null);
//		m_pTexture->DecrementReferenceCount();
//		m_pTexture->DeleteIfUnreferenced();
	}
}

void C_SteamBrowserInstance::SelfDestruct()
{
	DevMsg("SteamBrowserInstance: SelfDestruct\n");
	steamapicontext->SteamHTMLSurface()->RemoveBrowser(m_unBrowserHandle);

	if (m_pLastFrameData)
		free(m_pLastFrameData);

	if (m_pPostData)
		free(m_pPostData);

	delete this;
}

void C_SteamBrowserInstance::Init(std::string id, std::string url, const char* pchPostData)
{
	m_id = id;
	if (m_id == "")
		m_id = g_pAnarchyManager->GenerateUniqueId();

	m_initialURL = url;
	m_pPostData = (void*)pchPostData;

	SteamAPICall_t hAPICall = steamapicontext->SteamHTMLSurface()->CreateBrowser("", "");
	m_CreateBrowserInstance.Set(hAPICall, this, &C_SteamBrowserInstance::OnBrowserInstanceCreated);
}

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
	//C_InputListenerSteamBrowser* pListener = g_pAnarchyManager->GetSteamBrowserManager()->GetInputListener();
	//g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(m_pTexture);
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, this);// (C_InputListener*)pListener);



	m_unBrowserHandle = pResult->unBrowserHandle;

	steamapicontext->SteamHTMLSurface()->SetSize(m_unBrowserHandle, iWidth, iHeight);

	g_pAnarchyManager->GetSteamBrowserManager()->OnSteamBrowserInstanceCreated(this);
	steamapicontext->SteamHTMLSurface()->LoadURL(m_unBrowserHandle, m_initialURL.c_str(), "");
}

void C_SteamBrowserInstance::BrowserInstanceStartRequest(HTML_StartRequest_t *pCmd)
{
	bool bRes = OnStartRequest(pCmd->pchURL, pCmd->pchTarget, pCmd->pchPostData, pCmd->bIsRedirect);

	steamapicontext->SteamHTMLSurface()->AllowStartRequest(m_unBrowserHandle, bRes);
}

//-----------------------------------------------------------------------------
// Purpose: browser wants to start loading this url, do we let it?
//-----------------------------------------------------------------------------
bool C_SteamBrowserInstance::OnStartRequest(const char *url, const char *target, const char *pchPostData, bool bIsRedirect)
{
	if (!url || !Q_stricmp(url, "about:blank"))
		return true; // this is just webkit loading a new frames contents inside an existing page

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

	DevMsg("SteamworksBrowser: OnStartRequest - %s\n", url);

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: finished loading this page
//-----------------------------------------------------------------------------
void C_SteamBrowserInstance::BrowserInstanceFinishedRequest(HTML_FinishedRequest_t *pCmd)
{
	/*
	PostActionSignal(new KeyValues("OnFinishRequest", "url", pCmd->pchURL));
	if (pCmd->pchPageTitle && pCmd->pchPageTitle[0])
	PostActionSignal(new KeyValues("PageTitleChange", "title", pCmd->pchPageTitle));
	*/
	CUtlMap < CUtlString, CUtlString > mapHeaders;
	SetDefLessFunc(mapHeaders);
	// headers are no longer reported on loads

	OnFinishRequest(pCmd->pchURL, pCmd->pchPageTitle, mapHeaders);
}

void C_SteamBrowserInstance::OnFinishRequest(const char *url, const char *pageTitle, const CUtlMap < CUtlString, CUtlString > &headers)
{
	if (!url || !Q_stricmp(url, "about:blank"))
		return;

	DevMsg("Request finished!!\n");
	//	steamapicontext->SteamHTMLSurface()->
	//	m_unBrowserHandle
}

//-----------------------------------------------------------------------------
// Purpose: we have a new texture to update
//-----------------------------------------------------------------------------
void C_SteamBrowserInstance::BrowserInstanceNeedsPaint(HTML_NeedsPaint_t *pCallback)
{
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

	OnNeedsPaint(pCallback->pBGRA, pCallback->unWide, pCallback->unTall, 4);
}

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
	/*
	info->lastframewidth = width;
	info->lastframeheight = height;
	info->lastframepitch = pitch;

	info->readytocopyframe = true;
	*/

	m_bReadyToCopyFrame = true;

	//this->CopyLastFrame()
}

void C_SteamBrowserInstance::OnMouseMove(float x, float y)
{
	int goodX = (1280 * x) / 1;
	int goodY = (720 * y) / 1;
	steamapicontext->SteamHTMLSurface()->MouseMove(m_unBrowserHandle, goodX, goodY);
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

	steamapicontext->SteamHTMLSurface()->MouseDown(m_unBrowserHandle, goodButton);
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

	steamapicontext->SteamHTMLSurface()->MouseUp(m_unBrowserHandle, goodButton);
}

void C_SteamBrowserInstance::Update()
{
	//if (m_info->state == 1)
		this->OnProxyBind(null);
}

void C_SteamBrowserInstance::ResizeFrameFromRGB565(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth)
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

void C_SteamBrowserInstance::OnProxyBind(C_BaseEntity* pBaseEntity)
{
//	if (m_id == "images")
//		return;

	/*
	if ( pBaseEntity )
	DevMsg("WebTab: OnProxyBind: %i\n", pBaseEntity->entindex());
	else
	DevMsg("WebTab: OnProxyBind\n");
	*/

	if (m_iLastRenderedFrame < gpGlobals->framecount)
	{
		//if (!g_pAnarchyManager->GetCanvasManager()->IsPriorityCanvas(this))
		//g_pAnarchyManager->GetCanvasManager()->IncrementVisibleCanvasesCurrentFrame();

		if (g_pAnarchyManager->GetCanvasManager()->ShouldRender(this) && m_bIsDirty)
			Render();
	}
}

void C_SteamBrowserInstance::Render()
{
//	if (m_id == "images")
	//	return;
	//DevMsg("Rendering texture: %s\n", m_pTexture->GetName());
	//	DevMsg("Render Web Tab: %s\n", this->GetTexture()->Ge>GetId().c_str());
	//DevMsg("WebTab: Render: %s on %i\n", m_id.c_str(), gpGlobals->framecount);
	g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen()->SetEmbeddedInstance(this);
	m_pTexture->Download();

	m_iLastRenderedFrame = gpGlobals->framecount;

//	if (!g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(this))
//		g_pAnarchyManager->GetCanvasManager()->SetLastPriorityRenderedFrame(gpGlobals->framecount);
//	else
		g_pAnarchyManager->GetCanvasManager()->SetLastRenderedFrame(gpGlobals->framecount);
}

void C_SteamBrowserInstance::RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect)
{
	//if (m_info->state == 1)
		this->CopyLastFrame(pVTFTexture->ImageData(0, 0, 0), pSubRect->width, pSubRect->height, pSubRect->width * 4, 4);
}

C_InputListener* C_SteamBrowserInstance::GetInputListener()
{
	return g_pAnarchyManager->GetSteamBrowserManager()->GetInputListener();
}