#include "cbase.h"
#include "c_websurfaceproxy.h"

#include <string>
#include "Filesystem.h"
#include <KeyValues.h>
#include "c_anarchymanager.h"
//#include "c_simple_image_entity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

std::map<std::string, std::map<std::string, ITexture*>> CWebSurfaceProxy::s_simpleImages;

//int CWebSurfaceProxy::s_textureCount = 0;
CCanvasRegen* CWebSurfaceProxy::s_pCanvasRegen = null;

void CWebSurfaceProxy::OnSimpleImageRendered(std::string channel, std::string itemId, std::string field, ITexture* pTexture)
{
//	DevMsg("WebSurfaceProxy: OnSimpleImageRendered %s %s %s\n", channel.c_str(), itemId.c_str(), field.c_str());
	s_simpleImages[channel][itemId] = pTexture;

	if (field != "" && field != channel && field != "file")	// FIXME: more intellegent look-ahead before requesting images be rendered would speed stuff up a lot.
		s_simpleImages[field][itemId] = pTexture;
}

CWebSurfaceProxy::CWebSurfaceProxy()
{
	DevMsg("WebSurfaceProxy: Constructor\n");
	m_iState = 0;
	//m_id = "";
	m_pCurrentEmbeddedInstance = null;
	m_pMaterial = null;
	m_pCurrentTexture = null;
	m_pOriginalTexture = null;
	m_pMaterialTextureVar = null;
	m_pMaterialDetailBlendFactorVar = null;
	m_pEmbeddedInstance = null;
	m_originalId = "";
	m_iOriginalAutoCreate = 1;
	m_originalUrl = "";
	m_originalSimpleImageChannel = "";

	//g_pAnarchyManager->GetCanvasManager()->RegisterProxy(this);
}

CWebSurfaceProxy::~CWebSurfaceProxy()
{
	DevMsg("WebSurfaceProxy: Destructor\n");
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

	//g_pAnarchyManager->GetWebManager()->RegisterProxy(this);
	g_pAnarchyManager->GetCanvasManager()->RegisterProxy(this);

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
			pTexture = it2->second;
			if (pTexture)
			{
				usedTextures[pTexture] = true;
			//	pTexture->DecrementReferenceCount();	// FIXME: Disabled on 10/21/2016 for testing
			}

			/*
			if (usedTextures.find(it2->second) == usedTextures.end())
			{
				usedTextures[it2->second] = true;

				//do work
				pTexture = it2->second;

				if (pTexture)
				{
					//pTexture->SetTextureRegenerator(null);
					pTexture->DecrementReferenceCount();
					//pTexture->DeleteIfUnreferenced();
				}
			}
			else
			{
				//do work
				pTexture = it2->second;

				if (pTexture)
				{
					//pTexture->SetTextureRegenerator(null);
					pTexture->DecrementReferenceCount();
					//pTexture->DeleteIfUnreferenced();
				}
			}
			*/

			it2++;
		}

		//it->second.clear();
		it++;
	}

	auto usedIt = usedTextures.begin();
	while (usedIt != usedTextures.end())
	{
		pTexture = usedIt->first;
		if (pTexture)
		{
			pTexture->DecrementReferenceCount();	// FIXME: Disabled on 10/21/2016 for testing
			pTexture->SetTextureRegenerator(null);
			pTexture->DeleteIfUnreferenced();
		}

		usedIt++;
	}

	it = simpleImages.begin();
	while (it != simpleImages.end())
	{
		it->second.clear();
		it++;
	}

	simpleImages.clear();
	usedTextures.clear();
}

void CWebSurfaceProxy::StaticLevelShutdownPreEntity()
{
	// THIS SHOULD ONLY BE CALLED ONCE!!!!
	//ReleaseSimpleImages(s_simpleImages);
}

void CWebSurfaceProxy::StaticLevelShutdownPostEntity()
{
	// THIS SHOULD ONLY BE CALLED ONCE!!!!
	ReleaseSimpleImages(s_simpleImages);
}

void CWebSurfaceProxy::LevelShutdownPreEntity()
{
}

void CWebSurfaceProxy::ReleaseCurrent()
{
	if (m_pMaterialTextureVar && m_pCurrentTexture && m_pCurrentTexture != m_pOriginalTexture)
		m_pMaterialTextureVar->SetTextureValue(m_pOriginalTexture);
}

void CWebSurfaceProxy::ReleaseStuff()
{
	DevMsg("WebSurfaceProxy: Release Stuff\n");
	if (m_pMaterialTextureVar && m_pOriginalTexture && m_pCurrentTexture && m_pOriginalTexture != m_pCurrentTexture)
	{
		m_pMaterialTextureVar->SetTextureValue(m_pOriginalTexture);
	}

	//if ( m_pOriginalTexture )

	m_iState = 0;
	//m_id = "";
	m_pCurrentEmbeddedInstance = null;
	m_pMaterial = null;
	m_pCurrentTexture = null;
	m_pOriginalTexture = null;
	m_pMaterialTextureVar = null;
	m_pMaterialDetailBlendFactorVar = null;
	m_pEmbeddedInstance = null;
	m_originalId = "";
	m_iOriginalAutoCreate = 1;
	m_originalUrl = "";
	m_originalSimpleImageChannel = "";
}

void CWebSurfaceProxy::Release()
{
	DevMsg("WebSurfaceProxy: Release\n");	// FIXME: This is causing some fucked up issues.  wtf is going on bra, fix it.
//	if (g_pAnarchyManager->GetSuspendEmbedded())	// hmmm, does this ever get triggered when embeds ARE NOT suspended?
//		return;	// FIXME: disabled for resting on 10/4/2016 and again on 10/21/2016
//	else
//		DevMsg("Embededs ARE NOT suspended %s\n", this->m_originalId.c_str());	// well that'll answer that.

	/*
	if (m_pMaterialTextureVar && m_pMaterialTextureVar->IsDefined() && m_pMaterialTextureVar->IsTexture() && m_pOriginalTexture && m_pCurrentTexture && m_pOriginalTexture != m_pCurrentTexture)
		m_pMaterialTextureVar->SetTextureValue(m_pOriginalTexture);

	m_iState = 0;
	//m_id = "";
	m_pCurrentEmbeddedInstance = null;
	m_pMaterial = null;
	m_pCurrentTexture = null;
	m_pOriginalTexture = null;
	m_pMaterialTextureVar = null;
	m_pMaterialDetailBlendFactorVar = null;
	m_pEmbeddedInstance = null;
	m_originalId = "";
	m_iOriginalAutoCreate = 1;
	m_originalUrl = "";
	m_originalSimpleImageChannel = "";
	*/

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

	//if (g_pAnarchyManager->GetSuspendEmbedded())
	//	s_simpleImages.clear();
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
	//C_EmbeddedInstance* pSelectedEmebeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
	C_EmbeddedInstance* pSelectedEmebeddedInstance = null;
	
	if (m_pEmbeddedInstance)
		pSelectedEmebeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();// m_pEmbeddedInstance->GetParentSelectedEmbeddedInstance();

	C_AwesomiumBrowserInstance* pAwesomiumBrowserInstance = dynamic_cast<C_AwesomiumBrowserInstance*>(m_pEmbeddedInstance);

	bool bShouldBind = false;
	if (!m_pEmbeddedInstance)
	{
		if (m_iState == 0)
		{
			if (m_originalId != "")
			{
				// does a web tab for this id already exist?
				//C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->FindWebTab(m_originalId);
				//C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
				C_EmbeddedInstance* pEmbeddedInstance = (m_originalId == "images") ? g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("images") : g_pAnarchyManager->GetCanvasManager()->FindEmbeddedInstance(m_originalId);
				if (pEmbeddedInstance)
				{
					m_pEmbeddedInstance = pEmbeddedInstance;
					m_pCurrentEmbeddedInstance = pEmbeddedInstance;
					g_pAnarchyManager->GetCanvasManager()->SetMaterialEmbeddedInstanceId(m_pMaterial, pEmbeddedInstance->GetId());
					//g_pAnarchyManager->GetWebManager()->SetMaterialWebTabId(m_pMaterial, m_pWebTab->GetId());
				}
			}

			if (!m_pEmbeddedInstance)
			{
				// check if we should create a web tab
				if (m_iOriginalAutoCreate == 1 )	//&& m_iState == 0 )	// redundant
				{
					// create a web tab
					//m_pEmbeddedInstance = g_pAnarchyManager->GetWebManager()->CreateWebTab(m_originalUrl, m_originalId);
				//	m_pEmbeddedInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->CreateAwesomiumBrowserInstance(m_originalId, m_originalUrl, false);

					// try to get the object ID
					C_SteamBrowserInstance* pSteamBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->CreateSteamBrowserInstance();
					C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(pC_BaseEntity);
					pSteamBrowserInstance->Init(m_originalId, m_originalUrl, "Auto Steamworks Browser", null, (pShortcut) ? pShortcut->entindex() : -1);

					m_pEmbeddedInstance = pSteamBrowserInstance;
					m_pCurrentEmbeddedInstance = m_pEmbeddedInstance;
					//g_pAnarchyManager->GetWebManager()->SetMaterialWebTabId(m_pMaterial, m_pWebTab->GetId());
					m_iState = 1;	// initializing
				}  
			}

			if (m_pEmbeddedInstance)
				g_pAnarchyManager->GetCanvasManager()->SetMaterialEmbeddedInstanceId(m_pMaterial, m_pEmbeddedInstance->GetId());
				//g_pAnarchyManager->GetWebManager()->SetMaterialWebTabId(m_pMaterial, m_pEmbeddedInstance->GetId());
		}
	}
	else
	{
		if (pAwesomiumBrowserInstance && pAwesomiumBrowserInstance->GetState() == 2)
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
			ITexture* pTexture = m_pEmbeddedInstance->GetTexture();
			if (pTexture)
			{
				m_pMaterialTextureVar->SetTextureValue(pTexture);
				//			m_iState = 2;

				if (m_pMaterialDetailBlendFactorVar && ((pC_BaseEntity && g_pAnarchyManager->GetSelectedEntity() != pC_BaseEntity) || !pSelectedEmebeddedInstance || !g_pAnarchyManager->GetInputManager()->GetInputMode() || m_pEmbeddedInstance != pSelectedEmebeddedInstance))
					m_pMaterialDetailBlendFactorVar->SetFloatValue(0);
				else if (m_pMaterialDetailBlendFactorVar)
					m_pMaterialDetailBlendFactorVar->SetFloatValue(1);

				m_pEmbeddedInstance->OnProxyBind(pC_BaseEntity);
			}
		}
		else
		{
			if (m_pMaterialDetailBlendFactorVar && ((pC_BaseEntity && g_pAnarchyManager->GetSelectedEntity() != pC_BaseEntity) || !pSelectedEmebeddedInstance || !g_pAnarchyManager->GetInputManager()->GetInputMode()))
				m_pMaterialDetailBlendFactorVar->SetFloatValue(0);
			else if (m_pMaterialDetailBlendFactorVar)
				m_pMaterialDetailBlendFactorVar->SetFloatValue(1);

			bool bSwappedEmbeddedInstanceIn = false;
			// if a texture exists for this shortcut's item's id for this channel, then we're done already.
			// otherwise, we have to request the web tab to render us, which will be ignored 90% of the time, but thats OK.
			bool bTextureExists = false;
			C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(pC_BaseEntity);
			if (pShortcut)
			{
				std::string itemId = pShortcut->GetItemId();
				if (true)//itemId != "")
				{
					// we're swapping in the web browser tab even if no simple image is anywhere.
					if (m_originalSimpleImageChannel == "screen" || itemId == "" )
					{
						C_EmbeddedInstance* testerInstance = null;	// if initialized to null, this might work still!!!
						std::string tabTitle;

						if (itemId != "")
							testerInstance = g_pAnarchyManager->GetCanvasManager()->FindEmbeddedInstance("auto" + itemId);	// WARNING: If instances are never removed, this returns even dead instances!!

						tabTitle = (g_pAnarchyManager->GetSelectedEntity()) ? "auto" + static_cast<C_PropShortcutEntity*>(g_pAnarchyManager->GetSelectedEntity())->GetItemId() : "";

						C_EmbeddedInstance* selectedInstance = (g_pAnarchyManager->GetSelectedEntity()) ? g_pAnarchyManager->GetCanvasManager()->FindEmbeddedInstance(tabTitle) : null;

						C_EmbeddedInstance* displayInstance = g_pAnarchyManager->GetCanvasManager()->GetDisplayInstance();
						if (!displayInstance)
						{
							// if there's not a display instance, just grab the 1st instance found
							displayInstance = g_pAnarchyManager->GetCanvasManager()->GetFirstInstanceToDisplay();
						}

						// FIXME: This should be a required method of all embedded instances!!
					//	bool bInstanceTextureReady = true;
						//C_LibretroInstance* pLibretroInstance = (selectedInstance) ? dynamic_cast<C_LibretroInstance*>(selectedInstance) : null;
					//	if (pLibretroInstance)
						//	bInstanceTextureReady = (pLibretroInstance->GetInfo()->state == 5);

							//if (g_pAnarchyManager->GetSelectedEntity() && !testerInstance && selectedInstance && (pShortcut->GetSlave() || itemId == ""))//g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance())
						if (!testerInstance && (selectedInstance || displayInstance) && (pShortcut->GetSlave() || itemId == ""))
						{
							//	DevMsg("Swapped slave in!\n");
							testerInstance = (selectedInstance) ? selectedInstance : displayInstance;

							//if (m_pMaterialDetailBlendFactorVar && (!g_pAnarchyManager->GetInputManager()->GetInputMode() || m_originalSimpleImageChannel != "screen"))
							//if (true && m_pMaterialDetailBlendFactorVar)

							// FIXME: Improve this to make sure THIS instance is the input instance before thinking we need to draw the UI.
							if (m_pMaterialDetailBlendFactorVar && (!g_pAnarchyManager->GetInputManager()->GetInputMode() || !g_pAnarchyManager->GetSelectedEntity() || m_originalSimpleImageChannel != "screen"))
								m_pMaterialDetailBlendFactorVar->SetFloatValue(0);
							else if (m_pMaterialDetailBlendFactorVar)
								m_pMaterialDetailBlendFactorVar->SetFloatValue(1);
						}

						if (m_pMaterialTextureVar && testerInstance && testerInstance->GetTexture())
						{
							ITexture* pTesterTexture = testerInstance->GetTexture();
							//DevMsg("tester name: %s\n", pTesterTexture->GetName());
							m_pMaterialTextureVar->SetTextureValue(pTesterTexture);

							// instead, try to use same logic as ShouldRender
							//if (g_pAnarchyManager->GetCanvasManager()->ShouldRender(testerInstance, true))	// not enough to fix the issue.
							if (testerInstance->GetLastVisibleFrame() != gpGlobals->framecount)	// FIXME: This doesn't take into account that this slave web tab is the same as the original
								testerInstance->Update();

							bSwappedEmbeddedInstanceIn = true;
						}
					}

					if ( itemId != "")
					{
						// we still need to find the texture so we can process the image loading still.
						std::map<std::string, std::map<std::string, ITexture*>>::iterator it = s_simpleImages.find(m_originalSimpleImageChannel);
						if (it != s_simpleImages.end())
						{
							std::map<std::string, ITexture*>::iterator it2 = it->second.find(itemId);
							if (it2 != it->second.end())
							{
								if (!bSwappedEmbeddedInstanceIn && m_pMaterialTextureVar)
								{
									// we have found our texture.  swap it in and we're done.
									if (it2->second)
									{
										m_pMaterialTextureVar->SetTextureValue(it2->second);
									}
									else
										m_pMaterialTextureVar->SetTextureValue(m_pOriginalTexture);
								}

								bTextureExists = true;
							}
						}

						if (!bTextureExists)
						{
							C_AwesomiumBrowserInstance* pImagesBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("images");
							if (pImagesBrowserInstance && pImagesBrowserInstance->RequestLoadSimpleImage(m_originalSimpleImageChannel, itemId))
							{
								// If the request was accepted, then we need to get rdy to get the result.
								s_simpleImages[m_originalSimpleImageChannel][itemId] = null;
							}
						}
					}
				}
			}

			if (!bTextureExists && !bSwappedEmbeddedInstanceIn)
				m_pMaterialTextureVar->SetTextureValue(m_pOriginalTexture);	// Note that there is 1 case where bTextureExists can be true but the original texture still gets swapped in.
		}
	}

	///* show selected texture on broken cabinets
	if (!m_pEmbeddedInstance)
	{
		//C_WebTab* pSelectedWebTab = g_pAnarchyManager->GetWebManager()->GetSelectedWebTab();
		if (pSelectedEmebeddedInstance)
		{
			ITexture* pSelectedTexture = pSelectedEmebeddedInstance->GetTexture();
			m_pMaterialTextureVar->SetTextureValue(pSelectedTexture);
		}
	}
	//*/
	
	// even if we didn't find a new web tab this bind, continue acting as if the old one is still active.
	/*
	if (m_pCurrentEmbeddedInstance && m_pCurrentEmbeddedInstance != m_pEmbeddedInstance)
	{
		m_pCurrentEmbeddedInstance->OnProxyBind(pC_BaseEntity);
	}
	*/
}

void CWebSurfaceProxy::PrepareRefreshItemTextures(std::string itemId, std::string channel)
{
	if (!m_pMaterialTextureVar)
		return;

	auto channelIt = s_simpleImages.begin();
	while (channelIt != s_simpleImages.end())
	{
		if (channel == "ALL" || channelIt->first == channel)
		{
			auto itemIt = channelIt->second.find(itemId);
			if (itemIt != channelIt->second.end())
			{
				ITexture* pTexture = itemIt->second;
				if (pTexture && m_pMaterialTextureVar->GetTextureValue() == pTexture)
					m_pMaterialTextureVar->SetTextureValue(m_pOriginalTexture);
			}

			if (channel != "ALL")
				break;
		}

		channelIt++;
	}
}

void CWebSurfaceProxy::UnreferenceTexture(ITexture* pTexture)
{
	if (m_pMaterialTextureVar && m_pMaterialTextureVar->IsDefined() && m_pMaterialTextureVar->IsTexture() && m_pMaterialTextureVar->GetTextureValue() == pTexture )
			m_pMaterialTextureVar->SetTextureValue(m_pOriginalTexture);
}

void CWebSurfaceProxy::UnreferenceEmbeddedInstance(C_EmbeddedInstance* pEmbeddedInstance)
{
	if (m_pCurrentEmbeddedInstance == pEmbeddedInstance)
	{
		DevMsg("Unreferencing embedded instance from material proxy: m_pCurrentEmbeddedInstance\n");
		m_pCurrentEmbeddedInstance = null;
	}

	if (m_pEmbeddedInstance == pEmbeddedInstance)
	{
		DevMsg("Unreferencing embedded instance from material proxy: m_pEmbeddedInstance\n");
		m_pEmbeddedInstance = null;
	}
}

void CWebSurfaceProxy::RefreshItemTextures(std::string itemId, std::string channel)
{
	auto channelIt = s_simpleImages.begin();
	while (channelIt != s_simpleImages.end())
	{
		if (channel == "ALL" || channelIt->first == channel)
		{
			auto itemIt = channelIt->second.find(itemId);
			if (itemIt != channelIt->second.end())
			{
				ITexture* pTexture = itemIt->second;
				if (pTexture)
				{
					// if there's already a texture for this item, we're gonna have to replace it.  So that means releasing it first.
					// FIXME: do work...

					// aaaand theeeeen....
				}

				channelIt->second.erase(itemIt);
			}

			if (channel != "ALL")
				break;
		}

		channelIt++;
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