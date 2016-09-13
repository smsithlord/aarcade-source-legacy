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
#include <mutex>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_AwesomiumBrowserInstance::C_AwesomiumBrowserInstance()
{
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
}

C_AwesomiumBrowserInstance::~C_AwesomiumBrowserInstance()
{
	DevMsg("AwesomiumBrowserInstance: Destructor\n");
	
	if (m_pTexture)
	{
		m_pTexture->SetTextureRegenerator(null);
//		m_pTexture->DecrementReferenceCount();
//		m_pTexture->DeleteIfUnreferenced();
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
	//int iWidth = 1920;
	//int iHeight = 1080;

	//m_pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, iWidth, iHeight, IMAGE_FORMAT_BGR888, 1);
	if (m_bAlpha)
		m_pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, iWidth, iHeight, IMAGE_FORMAT_BGRA8888, 1);
	else
		m_pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, iWidth, iHeight, IMAGE_FORMAT_BGR888, 1);

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

/*
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

void C_AwesomiumBrowserInstance::OnProxyBind(C_BaseEntity* pBaseEntity)
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
		// just don't render the hud webview for now...
	//	if (m_id == "hud")
//			return;

		//if (!g_pAnarchyManager->GetCanvasManager()->IsPriorityCanvas(this))
		//g_pAnarchyManager->GetCanvasManager()->IncrementVisibleCanvasesCurrentFrame();

		bool bIsDirty = false;
		if (m_pWebView)
		{
			Awesomium::BitmapSurface* surface = static_cast<Awesomium::BitmapSurface*>(m_pWebView->surface());
			if (surface != 0)
				bIsDirty = surface->is_dirty();
		}

		if (g_pAnarchyManager->GetCanvasManager()->ShouldRender(this) && bIsDirty)//&& m_bIsDirty)
			Render();
	}
}

void C_AwesomiumBrowserInstance::Render()
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

C_InputListener* C_AwesomiumBrowserInstance::GetInputListener()
{
	return g_pAnarchyManager->GetAwesomiumBrowserManager()->GetInputListener();
}