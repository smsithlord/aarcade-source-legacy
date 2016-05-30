#include "cbase.h"
#include "c_websurfaceproxy.h"
#include "c_websurfaceregen.h"

#include <string>
#include "Filesystem.h"
#include <KeyValues.h>
#include "c_anarchymanager.h"
//#include "c_simple_image_entity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

int CWebSurfaceProxy::s_textureCount = 0;
CWebSurfaceRegen* CWebSurfaceProxy::s_pWebSurfaceRegen = null;

CWebSurfaceProxy::CWebSurfaceProxy()
{
	DevMsg("WebSurfaceProxy: Constructor\n");
	m_pTexture = null;
	m_pLoadingTexture = null;
	//m_image.status = 0;
//	m_image.ownedBy = null;		// FIXME: It is possible that these pointers should be set in the INIT message instead of the constructor. Look into it then update this comment.
	//m_channel = 0;
}

CWebSurfaceProxy::~CWebSurfaceProxy()
{
	DevMsg("WebSurfaceProxy: Destructor\n");
	if( m_pTexture != null )
	{
		m_pTexture->SetTextureRegenerator( null );
	}
	// FIXME: Do we need to reduce the texture reference or delete the m_pTexture as well?
	// FIXME: Does this get called every map load? Should all the textureregens in the m_pImageInstances get NULL'd too?
}

bool CWebSurfaceProxy::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	DevMsg("WebSurfaceProxy: Init\n");

	m_pMaterial = pMaterial;

	bool found;
	IMaterialVar* pMaterialVar = m_pMaterial->FindVar("$basetexture", &found, false);

	if( !found )
	{
		DevMsg("ERROR: No $basetexture found!\n");
		return false;
	}

	// If we haven't created the regen yet, do so now.
	if( !s_pWebSurfaceRegen )
		s_pWebSurfaceRegen = new CWebSurfaceRegen();

	m_pLoadingTexture = pMaterialVar->GetTextureValue();
	m_pLoadingTexture->SetTextureRegenerator(s_pWebSurfaceRegen);

	m_pMaterialTextureVar = pMaterialVar;

	/*
	pMaterialVar = m_pMaterial->FindVar("file", &found, false);
	if( found )
		m_image.file = pMaterialVar->GetStringValue();
	else
		m_image.file = "none";

	pMaterialVar = m_pMaterial->FindVar("channel", &found, false);
	if( found )
		m_channel = pMaterialVar->GetIntValue();
	else
		m_channel = 0;
	*/

	return true;
}

void CWebSurfaceProxy::Release()
{
	DevMsg("WebSurfaceProxy: Release\n");

	/*
	// Iterate through all our SimpleImages
	for(std::map<int, DynamicImage*>::iterator it = m_pImageInstances.begin(); it != m_pImageInstances.end(); it++)
		it->second->texture->DecrementReferenceCount();
	*/

	// Release our Loading Image
	if( m_pTexture )
		m_pTexture->DecrementReferenceCount();

	// FIXME: Do we need to delete the class-scope members too? YOUT HINK I KNO W? SHIIIIT

	delete this;
}

ITexture* CWebSurfaceProxy::CreateTexture(C_BaseEntity* pEntity = null)
{
	//DevMsg("WebSurfaceProxy: CreateTexture\n");
	return null;
	/*
	// Create the texture for this proxy instance
	C_DynamicImageWebView* pDynamicImageWebView = C_AnarchyManager::GetSelf()->GetWebViewManager()->GetDynamicImageWebView();
	int width = pDynamicImageWebView->GetWidth();
	int height = pDynamicImageWebView->GetHeight();

	std::string textureName = "simple_image_";
	textureName += C_AnarchyManager::GetSelf()->GenerateHash(m_pMaterial->GetName());

	if( pEntity )
	{
		textureName += "_";
		textureName += VarArgs("%i", pEntity->entindex());
	}

	ITexture* pTexture = g_pMaterialSystem->CreateProceduralTexture( textureName.c_str(), TEXTURE_GROUP_VGUI, width, height, IMAGE_FORMAT_BGR888, 1 );
	pTexture->SetTextureRegenerator(s_pDynamicRegen);

	return pTexture;
	*/
}

void CWebSurfaceProxy::OnBind(C_BaseEntity *pC_BaseEntity)
{
	//DevMsg("WebSurfaceProxy: OnBind\n");
	// The objective is to fill these two values
	//DynamicImage* pImage = null;
	//ITexture* pTexture = null;

	// If we don't have a main texture for this proxy yet, create it now.
	// TODO: Tests to confirm what the above comment means. (When does that happen??)
	if( !m_pTexture )
	{
		m_pTexture = this->CreateTexture();
	}

//	C_LiveView* pLiveView = null;
	// Check if the proxy is being used on a simple image entity
	if( pC_BaseEntity )
	{
		/*
		C_PropSimpleImageEntity* pPropSimpleImageEntity = null;
		pPropSimpleImageEntity = dynamic_cast<C_PropSimpleImageEntity*>(pC_BaseEntity);
		if( pPropSimpleImageEntity )
		{
			if( m_pImageInstances.find(pC_BaseEntity->entindex()) == m_pImageInstances.end() )
			{
				// We need to create a new SimpleImage and ITexture for this entity instance
				pImage = new DynamicImage;
				//pImage->file = pPropSimpleImageEntity->GetImageURL(m_channel);
				pImage->status = 0;
				pImage->texture = this->CreateTexture(pC_BaseEntity);
				pTexture = pImage->texture;

				// Store the pointer for next time
				m_pImageInstances[pC_BaseEntity->entindex()] = pImage;
			}
			else
			{
				pImage = m_pImageInstances[pC_BaseEntity->entindex()];
				pTexture = pImage->texture;
			}

			// Now that we have a pImage, check if we need to flag it as active on an entity
			if( pPropSimpleImageEntity == pPropSimpleImageEntity->GetActiveEntity() && m_channel == 1 )
			{
				pLiveView = dynamic_cast<C_LiveView*>(C_AnarchyManager::GetSelf()->GetWebViewManager()->GetLiveViewDetailsWebView());
			}
		}
		*/
	}

	// If proxy is not on an entity, then use the default Simple Image and texture
	/*
	if( !pImage )
	{
		pImage = &m_image;
		pImage->texture = m_pTexture;

		pTexture = pImage->texture;
	}
	*/

	//g_pAnarchyManager->GetWebManager()->Update();	// This gets handled elsewhere

	/*
	if( pImage->status == 0 )
	{
		s_pDynamicRegen->SetDynamicImage(pImage);
		pTexture->Download();

		// Set the loading texture
		m_pTextureVar->SetTextureValue(m_pLoadingTexture);
	}
	else if( pImage->status == 1 )
		// Set the loading texture
		m_pTextureVar->SetTextureValue(m_pLoadingTexture);
	else if( pImage->status == 2 )
	{
		// Set the custom texture
		m_pTextureVar->SetTextureValue(pTexture);
	}

	// Check if this SimpleImage is owned by a LiveView
	if( pLiveView && pLiveView->IsEarlyReady() )
	{
		// Swap in the LiveView's texture
		m_pTextureVar->SetTextureValue(pLiveView->GetTexture());

		// Call the update logic on our owner. It will re-render the texture if needed
		pLiveView->Update();
	}
	*/
}

IMaterial *CWebSurfaceProxy::GetMaterial()
{
	DevMsg("WebSurfaceProxy: GetMaterial\n");
	return m_pMaterial;
}