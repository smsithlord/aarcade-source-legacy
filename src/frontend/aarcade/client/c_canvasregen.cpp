#include "cbase.h"

#include "vtf/vtf.h"
#include "pixelwriter.h"

#include "c_canvasregen.h"
//#include "c_anarchymanager.h"

// For file handing
#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CCanvasRegen::CCanvasRegen()
{
	DevMsg("CanvasRegen: Constructor\n");
	m_pEmbeddedInstance = null;

//	m_pImage = null;
}

void CCanvasRegen::Release()
{
	//DevMsg("CanvasRegen: Release\n");
}

void CCanvasRegen::NotifyInstanceAboutToDie(C_EmbeddedInstance* pInstance)
{
	// only need to handle the case of this being the active instance.
	if (m_pEmbeddedInstance == pInstance)
		m_pEmbeddedInstance = null;
}

void CCanvasRegen::SetEmbeddedInstance(C_EmbeddedInstance* pInstance)
{
	m_pEmbeddedInstance = pInstance;
}

void CCanvasRegen::RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect)
{
//	DevMsg("Tab ID: %s\n", m_pWebTab->GetId().c_str());
	//DevMsg("WebSurfaceRege: RegenerateTextureBits\n");
	if (m_pEmbeddedInstance)
		m_pEmbeddedInstance->RegenerateTextureBits(pTexture, pVTFTexture, pSubRect);

	//return;
	/*
	if( !m_pImage )
		return;

	C_DynamicImageWebView* pDynamicImageWebView = C_AnarchyManager::GetSelf()->GetWebViewManager()->GetDynamicImageWebView();

	if( m_pImage->status == 0 )
	{
		// Put our request into the Webview to download us
		pDynamicImageWebView->PushRequest(m_pImage);
	}
	else if( m_pImage->status == 1 )
		pDynamicImageWebView->Render(pVTFTexture->ImageData(0, 0, 0));

	DevMsg("Regenerate Texture Bits\n");
	*/
}