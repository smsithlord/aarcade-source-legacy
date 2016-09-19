#include "cbase.h"

#include "vtf/vtf.h"
#include "pixelwriter.h"

#include "c_websurfaceregen.h"
#include "c_anarchymanager.h"

// For file handing
#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CWebSurfaceRegen::CWebSurfaceRegen()
{
	DevMsg("WebSurfaceRege: Constructor\n");

//	m_pImage = null;
}

void CWebSurfaceRegen::Release()
{
	DevMsg("WebSurfaceRege: Release\n");
}

void CWebSurfaceRegen::RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect)
{
//	DevMsg("Tab ID: %s\n", m_pWebTab->GetId().c_str());
	//DevMsg("WebSurfaceRege: RegenerateTextureBits\n");
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