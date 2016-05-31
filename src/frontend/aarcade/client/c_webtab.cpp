#include "cbase.h"

#include "c_webtab.h"
#include "c_anarchymanager.h"
//#include "aa_globals.h"
//#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_WebTab::C_WebTab(std::string url, std::string id)
{
	DevMsg("WebTab: Constructor\n");
	m_iState = 0;

	m_id = id;
	m_initialUrl = url;
	m_iLastRenderTick = -1;
	m_iState = 1;	// initializing

	// create the texture (each WebTab has its own texture)
	std::string textureName = "websurface_";
	textureName += m_id;

	int iWidth = g_pAnarchyManager->GetWebManager()->GetWebSurfaceWidth();
	int iHeight = g_pAnarchyManager->GetWebManager()->GetWebSurfaceHeight();
	m_pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, iWidth, iHeight, IMAGE_FORMAT_BGR888, 1);

	// get the regen and assign it
	CWebSurfaceRegen* pRegen = g_pAnarchyManager->GetWebManager()->GetOrCreateWebSurfaceRegen();
	m_pTexture->SetTextureRegenerator(pRegen);

	// create the web view
	g_pAnarchyManager->GetWebManager()->GetWebBrowser()->CreateWebView(this);
}

C_WebTab::~C_WebTab()
{
	DevMsg("WebTab: Destructor\n");
}

void C_WebTab::OnProxyBind(C_BaseEntity* pBaseEntity)
{
	/*
	if ( pBaseEntity )
		DevMsg("WebTab: OnProxyBind: %i\n", pBaseEntity->entindex());
	else
		DevMsg("WebTab: OnProxyBind\n");
	*/

	if (m_iLastRenderTick < gpGlobals->tickcount)
	{
		// render the web tab to its texture
		Render();

		m_iLastRenderTick = gpGlobals->tickcount;
	}
}

void C_WebTab::Render()
{
	//DevMsg("WebTab: Render: %s\n", m_id.c_str());
	g_pAnarchyManager->GetWebManager()->GetOrCreateWebSurfaceRegen()->SetWebTab(this);
	m_pTexture->Download();
}

void C_WebTab::RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect)
{
//	DevMsg("WebTab: RegenerateTextureBits\n");
	g_pAnarchyManager->GetWebManager()->GetWebBrowser()->RegenerateTextureBits(this, pTexture, pVTFTexture, pSubRect);
}