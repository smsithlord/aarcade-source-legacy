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

std::map<std::string, std::map<std::string, ITexture*>> CWebSurfaceProxy::s_simpleImages;

//int CWebSurfaceProxy::s_textureCount = 0;
CWebSurfaceRegen* CWebSurfaceProxy::s_pWebSurfaceRegen = null;

void CWebSurfaceProxy::OnSimpleImageRendered(std::string channel, std::string itemId, std::string field, ITexture* pTexture)
{
	DevMsg("WebSurfaceProxy: OnSimpleImageRendered\n");
	s_simpleImages[channel][itemId] = pTexture;

	if (field != "" && field != channel && field != "file")	// FIXME: more intellegent look-ahead before requesting images be rendered would speed stuff up a lot.
		s_simpleImages[field][itemId] = pTexture;
}

CWebSurfaceProxy::CWebSurfaceProxy()
{
	DevMsg("WebSurfaceProxy: Constructor\n");
	m_iState = 0;
	m_id = "";
	m_pCurrentWebTab = null;
	m_pMaterial = null;
	m_pCurrentTexture = null;
	m_pOriginalTexture = null;
	m_pMaterialTextureVar = null;
	m_pMaterialDetailBlendFactorVar = null;
	m_pWebTab = null;
	m_originalId = "";
	m_iOriginalAutoCreate = 1;
	m_originalUrl = "";
	m_originalSimpleImageChannel = "";
}

CWebSurfaceProxy::~CWebSurfaceProxy()
{
	//DevMsg("WebSurfaceProxy: Destructor\n");
//	if( m_pOriginalTexture != null )	// FIXME When would this ever not exist for this type of proxy?
	//{
//		m_pOriginalTexture->SetTextureRegenerator(null);
	//}
	// FIXME: Do we need to reduce the texture reference or delete the m_pTexture as well?
	// FIXME: Does this get called every map load? Should all the textureregens in the m_pImageInstances get NULL'd too?
}

bool CWebSurfaceProxy::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	DevMsg("WebSurfaceProxy: Init\n");

	m_pMaterial = pMaterial;

	// set all the original stuff
	bool found;
	IMaterialVar* pMaterialVar = m_pMaterial->FindVar("$basetexture", &found, false);

	if( !found )
	{
		DevMsg("ERROR: No $basetexture found!\n");
		return false;
	}

	g_pAnarchyManager->GetWebManager()->RegisterProxy(this);

	m_pMaterialTextureVar = pMaterialVar;
	m_pOriginalTexture = pMaterialVar->GetTextureValue();
//	m_pOriginalTexture->SetTextureRegenerator(s_pWebSurfaceRegen);

	bool bFoundDetailBlendFactorVar;
	m_pMaterialDetailBlendFactorVar = m_pMaterial->FindVar("$detailblendfactor", &bFoundDetailBlendFactorVar);

	pMaterialVar = m_pMaterial->FindVar("id", &found, false);
	m_originalId = (found) ? pMaterialVar->GetStringValue() : "";

	pMaterialVar = m_pMaterial->FindVar("url", &found, false);
	m_originalUrl = (found) ? pMaterialVar->GetStringValue() : "";

	/*DevMsg("Original URL is: %s\n", m_originalUrl.c_str());*/

	pMaterialVar = m_pMaterial->FindVar("autocreate", &found, false);
	m_iOriginalAutoCreate = (found) ? pMaterialVar->GetIntValue() : 0;

	//pMaterialVar = m_pMaterial->FindVar("simpleimage", &found, false);
	//m_bOriginalSimpleImage = (found) ? (pMaterialVar->GetIntValue() != 0) : false;

	pMaterialVar = m_pMaterial->FindVar("simpleimagechannel", &found, false);
	m_originalSimpleImageChannel = (found) ? pMaterialVar->GetStringValue() : "";
	return true;
}

void ReleaseSimpleImages(std::map<std::string, std::map<std::string, ITexture*>>& simpleImages)
{
	// set all owned textures's regenerator to null
	ITexture* pTexture;
	std::map<ITexture*, bool> usedTextures;
	std::map<std::string, std::map<std::string, ITexture*>>::iterator it = simpleImages.begin();
	std::map<std::string, ITexture*>::iterator it2;
	while (it != simpleImages.end())
	{
		it2 = it->second.begin();
		while (it2 != it->second.end())
		{
			if (usedTextures.find(it2->second) == usedTextures.end())
			{
				usedTextures[it2->second] = true;

				//do work
				pTexture = it2->second;

				if (pTexture)
				{
					pTexture->SetTextureRegenerator(null);

					pTexture->DecrementReferenceCount();
					pTexture->DeleteIfUnreferenced();
				}
			}

			it2++;
		}

		it->second.clear();
		it++;
	}

	simpleImages.clear();
	usedTextures.clear();
}

void CWebSurfaceProxy::LevelShutdownPreEntity()
{
	ReleaseSimpleImages(s_simpleImages);
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
	//if (m_pOriginalTexture)
	//{
		//m_pOriginalTexture->DecrementReferenceCount();
		//m_pOriginalTexture->SetTextureRegenerator(null);
	//}

	// FIXME: Do we need to delete the class-scope members too? YOUT HINK I KNO W? SHIIIIT

//	delete this;

	//m_pMaterialTextureVar->SetTextureValue(m_pOriginalTexture);

	// release the simple images
	//ReleaseSimpleImages(s_simpleImages);
}

//ITexture* CWebSurfaceProxy::CreateTexture(C_BaseEntity* pEntity = null)
//{
	//DevMsg("WebSurfaceProxy: CreateTexture\n");
	//return null;
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
//}

void CWebSurfaceProxy::OnBind(C_BaseEntity *pC_BaseEntity)
{
	bool bShouldBind = false;
	if (!m_pWebTab )
	{
		if (m_iState == 0)
		{
			if (m_originalId != "")
			{
				// does a web tab for this id already exist?
				C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->FindWebTab(m_originalId);
				if (pWebTab)
				{
					m_pWebTab = pWebTab;
					m_pCurrentWebTab = m_pWebTab;
					//g_pAnarchyManager->GetWebManager()->SetMaterialWebTabId(m_pMaterial, m_pWebTab->GetId());
				}
			}

			if (!m_pWebTab)
			{
				// check if we should create a web tab
				if (m_iOriginalAutoCreate == 1 && m_iState == 0)
				{
					// create a web tab
					m_pWebTab = g_pAnarchyManager->GetWebManager()->CreateWebTab(m_originalUrl, m_originalId);
					m_pCurrentWebTab = m_pWebTab;
					//g_pAnarchyManager->GetWebManager()->SetMaterialWebTabId(m_pMaterial, m_pWebTab->GetId());
					m_iState = 1;	// initializing
				}
			}

			if ( m_pWebTab)
				g_pAnarchyManager->GetWebManager()->SetMaterialWebTabId(m_pMaterial, m_pWebTab->GetId());
		}
	}
	else
	{
		if (m_pWebTab->GetState() == 2)
			m_iState = 2;

		/*
		if (m_pMaterialDetailBlendFactorVar && (!g_pAnarchyManager->GetWebManager()->GetSelectedWebTab() || !g_pAnarchyManager->GetInputManager()->GetInputMode() || m_pWebTab != g_pAnarchyManager->GetWebManager()->GetSelectedWebTab()))
			m_pMaterialDetailBlendFactorVar->SetFloatValue(0);
		else if (m_pMaterialDetailBlendFactorVar)
			m_pMaterialDetailBlendFactorVar->SetFloatValue(1);
		*/

		// a regular proxy will need to grab the web tab's texture before it binds
		if (m_originalSimpleImageChannel == "")
		{
			ITexture* pTexture = m_pWebTab->GetTexture();
			if (pTexture)
			{
				m_pMaterialTextureVar->SetTextureValue(pTexture);
				//			m_iState = 2;

				if (m_pMaterialDetailBlendFactorVar && ((pC_BaseEntity && g_pAnarchyManager->GetSelectedEntity() != pC_BaseEntity) || !g_pAnarchyManager->GetWebManager()->GetSelectedWebTab() || !g_pAnarchyManager->GetInputManager()->GetInputMode() || m_pWebTab != g_pAnarchyManager->GetWebManager()->GetSelectedWebTab()))
					m_pMaterialDetailBlendFactorVar->SetFloatValue(0);
				else if (m_pMaterialDetailBlendFactorVar)
					m_pMaterialDetailBlendFactorVar->SetFloatValue(1);

				m_pWebTab->OnProxyBind(pC_BaseEntity);
			}
		}
		else
		{
		//	if (m_pMaterialDetailBlendFactorVar && pC_BaseEntity && g_pAnarchyManager->GetSelectedEntity() != pC_BaseEntity )
			if (m_pMaterialDetailBlendFactorVar && ((pC_BaseEntity && g_pAnarchyManager->GetSelectedEntity() != pC_BaseEntity) || !g_pAnarchyManager->GetWebManager()->GetSelectedWebTab() || !g_pAnarchyManager->GetInputManager()->GetInputMode()))
				m_pMaterialDetailBlendFactorVar->SetFloatValue(0);
			else if (m_pMaterialDetailBlendFactorVar)
				m_pMaterialDetailBlendFactorVar->SetFloatValue(1);

			C_BaseEntity* pSelectedEntity = g_pAnarchyManager->GetSelectedEntity();
			C_WebTab* pSelectedWebTab = g_pAnarchyManager->GetWebManager()->GetSelectedWebTab();
			if (pSelectedEntity && pSelectedEntity == pC_BaseEntity && m_originalSimpleImageChannel == "screen" && pSelectedWebTab)
			{
				ITexture* pTexture = pSelectedWebTab->GetTexture();
				if (pTexture)
					m_pMaterialTextureVar->SetTextureValue(pTexture);	// no need to increment reference, because the selected web tab is always priority
				else
				{
					DevMsg("ERROR: Web tab has no texture!\n");
					m_pMaterialTextureVar->SetTextureValue(m_pOriginalTexture);
				}
			}
			else
			{
				// if a texture exists for this shortcut's item's id for this channel, then we're done already.
				// otherwise, we have to request the web tab to render us, which will be ignored 90% of the time, but thats OK.
				bool bTextureExists = false;
				C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(pC_BaseEntity);
				if (pShortcut)
				{
					std::string itemId = pShortcut->GetItemId();
					if (itemId != "")
					{
						//std::map<std::string, std::map<std::string, ITexture*>>::iterator it = s_simpleImages.find(m_originalId);
						std::map<std::string, std::map<std::string, ITexture*>>::iterator it = s_simpleImages.find(m_originalSimpleImageChannel);
						if (it != s_simpleImages.end())
						{
							std::map<std::string, ITexture*>::iterator it2 = it->second.find(itemId);
							if (it2 != it->second.end())
							{
								// we have found our texture.  swap it in and we're done.
								if (it2->second)	// only use it if it's non-null
									m_pMaterialTextureVar->SetTextureValue(it2->second);
								else
									m_pMaterialTextureVar->SetTextureValue(m_pOriginalTexture);

								bTextureExists = true;
							}
						}

						if (!bTextureExists)
						{
							if (m_pWebTab->RequestLoadSimpleImage(m_originalSimpleImageChannel, itemId))
							{
								// If the request was accepted, then we need to get rdy to get the result.
								s_simpleImages[m_originalSimpleImageChannel][itemId] = null;
							}
						}
					}
				}

				if (!bTextureExists)
					m_pMaterialTextureVar->SetTextureValue(m_pOriginalTexture);	// Note that there is 1 case where bTextureExists can be true but the original texture still gets swapped in.
			}
		}
	}

	if (!m_pWebTab)
	{
		C_WebTab* pSelectedWebTab = g_pAnarchyManager->GetWebManager()->GetSelectedWebTab();
		if (pSelectedWebTab)
		{
			ITexture* pSelectedTexture = pSelectedWebTab->GetTexture();
			m_pMaterialTextureVar->SetTextureValue(pSelectedTexture);
		}
	}

	// even if we didn't find a new web tab this bind, continue acting as if the old one is still active.
	if (m_pCurrentWebTab && m_pCurrentWebTab != m_pWebTab)
	{
		m_pCurrentWebTab->OnProxyBind(pC_BaseEntity);
	}
}
	//DevMsg("WebSurfaceProxy: OnBind\n");
	// The objective is to fill these two values
	//DynamicImage* pImage = null;
	//ITexture* pTexture = null;

	// If we don't have a main texture for this proxy yet, create it now.
	// TODO: Tests to confirm what the above comment means. (When does that happen??)
//	if( !m_pTexture )
//	{
//		m_pTexture = this->CreateTexture();
//	}

//	C_LiveView* pLiveView = null;
	// Check if the proxy is being used on a simple image entity
//	if( pC_BaseEntity )
	//{
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
//	}

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
//}