#include "cbase.h"

//#include "aa_globals.h"
#include "c_webmanager.h"
#include "c_anarchymanager.h"
#include "c_inputslate.h"
#include "materialsystem/IMaterialVar.h"
//#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_WebManager::C_WebManager()
{
	DevMsg("WebManager: Constructor\n");
	m_iState = 0;	// uninitialized
	//m_iWebSurfaceWidth = 1280;
	//m_iWebSurfaceHeight = 720;
	m_iWebSurfaceWidth = 1920;
	m_iWebSurfaceHeight = 1080;
	m_bHudPriority = false;
	m_bSelectedPriority = false;
	m_iVisibleWebTabsLastFrame = -1;
	m_iVisibleWebTabsCurrentFrame = 0;
	m_iLastRenderedFrame = -1;
	m_pWebBrowser = null;
	m_pWebSurfaceRegen = null;
	m_pSelectedWebTab = null;
	m_pHudWebTab = null;
	m_bHudReady = false;
}

C_WebManager::~C_WebManager()
{
	DevMsg("WebManager: Destructor\n");

	// iterate over all web tabs and call their destructors
	for (std::map<std::string, C_WebTab*>::iterator it = m_webTabs.begin(); it != m_webTabs.end(); ++it)
	{
		delete it->second;
		it->second = null;
	}

	if (m_pWebBrowser)
		delete m_pWebBrowser;
}

void C_WebManager::Init()
{
	DevMsg("WebManager: Init\n");
	m_iState = 1;	// initializing
	
	// Do work (possibly async)
	m_pWebBrowser = new C_WebBrowser();
	m_pWebBrowser->Init();

	//m_iState = 2;	// ready
}

void C_WebManager::Update()
{
	//DevMsg("WebManager: Update\n");

	// Update the visible web tab estimates
	m_iVisibleWebTabsLastFrame = m_iVisibleWebTabsCurrentFrame;
	m_iVisibleWebTabsCurrentFrame = 0;

	// Update the browser, if it exists
	if (m_pWebBrowser)
		m_pWebBrowser->Update();

	// FIXME: the selected web tab always renders
	if (m_pSelectedWebTab)
	{
		m_pSelectedWebTab->OnProxyBind();

		if (!m_bSelectedPriority)
			g_pAnarchyManager->GetWebManager()->IncrementVisibleWebTabsCurrentFrame();
	}

	// FIXME: the hud web tab always renders
	if (m_pHudWebTab && m_pHudWebTab != m_pSelectedWebTab && m_pSelectedWebTab)
	{
		m_pHudWebTab->OnProxyBind();

		if ( !m_bHudPriority )
			g_pAnarchyManager->GetWebManager()->IncrementVisibleWebTabsCurrentFrame();
	}
}

C_WebTab* C_WebManager::FindWebTab(std::string id)
{
	auto foundWebTab = m_webTabs.find(id);
	if (foundWebTab != m_webTabs.end())
	{
		return m_webTabs[id];
	}
	else
		return null;
}

C_WebTab* C_WebManager::FindWebTab(IMaterial* pMaterial)
{
	auto foundId = m_materialWebTabIds.find(pMaterial);
	if (foundId != m_materialWebTabIds.end())
	{
		std::string id = m_materialWebTabIds[pMaterial];

		auto foundWebTab = m_webTabs.find(id);
		if (foundWebTab != m_webTabs.end())
			return m_webTabs[id];
	}
	
	return null;
}

void C_WebManager::SetMaterialWebTabId(IMaterial* pMaterial, std::string id)
{
	m_materialWebTabIds[pMaterial] = id;
}

void C_WebManager::SelectWebTab(C_WebTab* pWebTab)
{
	g_pAnarchyManager->GetInputManager()->SetInputListener(g_pAnarchyManager->GetWebManager(), LISTENER_WEB_MANAGER);

	m_pSelectedWebTab = pWebTab;
	m_pWebBrowser->OnSelectWebTab(pWebTab);

	/*
	IMaterial* pMaterial = vgui::CInputSlate::s_pMaterial;
	ITexture* pOriginalTexture = vgui::CInputSlate::s_pOriginalTexture;
	if (pMaterial)
	{
		if (pOriginalTexture)
		{
			bool found;
			IMaterialVar* pMaterialVar = vgui::CInputSlate::s_pMaterial->FindVar("$basetexture", &found, false);
			pMaterialVar->SetTextureValue(pOriginalTexture);
			vgui::CInputSlate::s_pOriginalTexture = null;
		}

		bool found;
		IMaterialVar* pMaterialVar = vgui::CInputSlate::s_pMaterial->FindVar("$basetexture", &found, false);
		vgui::CInputSlate::s_pOriginalTexture = pMaterialVar->GetTextureValue();
		pMaterialVar->SetTextureValue(pWebTab->GetTexture());
	}
	*/
}

void C_WebManager::DeselectWebTab(C_WebTab* pWebTab)
{
	m_pSelectedWebTab = null;
	pWebTab->MouseMove(0.5, 0.5);
	m_pWebBrowser->OnDeselectWebTab(pWebTab);

	g_pAnarchyManager->GetInputManager()->SetInputListener(null, LISTENER_NONE);
	/*
	// update the live webview texture
	IMaterial* pMaterial = vgui::CInputSlate::s_pMaterial;
	ITexture* pOriginalTexture = vgui::CInputSlate::s_pOriginalTexture;
	if (pMaterial && pOriginalTexture)
	{
		bool found;
		IMaterialVar* pMaterialVar = vgui::CInputSlate::s_pMaterial->FindVar("$basetexture", &found, false);
		pMaterialVar->SetTextureValue(pOriginalTexture);
		vgui::CInputSlate::s_pOriginalTexture = null;
	}
	*/
}

void C_WebManager::OnBrowserInitialized()
{
	m_pHudWebTab = CreateHudWebTab();
}

void C_WebManager::OnHudWebTabReady()// Created()
{
	m_bHudReady = true;
	g_pAnarchyManager->OnWebManagerReady();
}

void C_WebManager::OnLoadingWebTabReady()
{
	/*
	C_LoadingManager* pLoadingManager = g_pAnarchyManager->GetLoadingManager();
	if (pLoadingManager)
		pLoadingManager->OnWebTabReady();
	*/






	//g_pAnarchyManager->OnLoadingManagerReady();
}

void C_WebManager::RemoveWebTab(C_WebTab* pWebTab)
{
	m_pWebBrowser->RemoveWebView(pWebTab);

	std::string textureName = "websurface_";
	textureName += pWebTab->GetId();

	ITexture* pTexture = materials->FindTexture(textureName.c_str(), TEXTURE_GROUP_VGUI);
	if (pTexture)
	{
		pTexture->DecrementReferenceCount();
		pTexture->DeleteIfUnreferenced();
	}

	std::map<std::string, C_WebTab*>::iterator it = m_webTabs.find(pWebTab->GetId());
	m_webTabs.erase(it);

	delete pWebTab;
	
//	m_webTabs
//	m_materialWebTabIds
}

C_WebTab* C_WebManager::CreateHudWebTab()
{
	//C_WebTab* pWebTab = CreateWebTab("asset://ui/hud.html", "hud", true);
	C_WebTab* pWebTab = CreateWebTab("asset://ui/loading.html", "hud", true);
	return pWebTab;
}

C_WebTab* C_WebManager::CreateWebTab(std::string url, std::string id, bool bAlpha)
{
	DevMsg("WebManager: CreateWebTab\n");

	std::string validId;
	if (id == "")
		validId = g_pAnarchyManager->GenerateUniqueId();
	else
		validId = id;	// FIXME: Might want to do a check to make sure a tab with this ID doesn't already exist.

	C_WebTab* pWebTab = new C_WebTab(url, validId, bAlpha);
	m_webTabs[validId] = pWebTab;

	return pWebTab;
}

CWebSurfaceRegen* C_WebManager::GetOrCreateWebSurfaceRegen()
{
	if (!m_pWebSurfaceRegen)
		m_pWebSurfaceRegen = new CWebSurfaceRegen;
	
	return m_pWebSurfaceRegen;
}


bool C_WebManager::ShouldRender(C_WebTab* pWebTab)
{
	//DevMsg("WebManager: ShouldRender\n");
	//if (pWebTab == m_pHudWebTab)//|| pWebTab == g_pAnarchyManager->GetWebManager()->GetSelectedWebTab())
	//{
	//	if (m_pSelectedWebTab)
//			return true;
	//	else
	//		return false;
//	}

	/*
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance();
	if (pLibretroInstance)
	{
		LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();
		if (info->processingaudio)
			return false;
	}
	*/

	int iLastRenderedFrame = pWebTab->GetLastRenderedFrame();
	//DevMsg("Last Frame: %i\n", m_iVisibleWebTabsLastFrame);
	//if (m_iLastRenderedFrame != gpGlobals->framecount && (pWebTab == m_pHudWebTab || iLastRenderedFrame <= 0 || m_iVisibleWebTabsLastFrame <= 1 || gpGlobals->framecount - iLastRenderedFrame >= m_iVisibleWebTabsLastFrame))
	if (m_iLastRenderedFrame != gpGlobals->framecount && (iLastRenderedFrame <= 0 || m_iVisibleWebTabsLastFrame <= 1 || gpGlobals->framecount - iLastRenderedFrame >= m_iVisibleWebTabsLastFrame))
		return true;

	return false;
}

void C_WebManager::OnMouseMove(float fXAmount, float fYAmount)
{
	// these events are always for the selected web tab
	m_pSelectedWebTab->MouseMove(fXAmount, fYAmount);
}

void C_WebManager::OnMousePress(vgui::MouseCode code)
{
	// these events are always for the selected web tab
	m_pSelectedWebTab->MousePress(code);
}

void C_WebManager::OnMouseRelease(vgui::MouseCode code)
{
	// these events are always for the selected web tab
	m_pSelectedWebTab->MouseRelease(code);
}

void C_WebManager::OnKeyCodePressed(vgui::MouseCode code, bool bShiftState, bool bCtrlState, bool bAltState)
{
	// these events are always for the selected web tab
	m_pSelectedWebTab->KeyCodePress(code, bShiftState, bCtrlState, bAltState);
}

void C_WebManager::OnKeyCodeReleased(vgui::MouseCode code, bool bShiftState, bool bCtrlState, bool bAltState)
{
	// these events are always for the selected web tab
	m_pSelectedWebTab->KeyCodeRelease(code, bShiftState, bCtrlState, bAltState);
}

void C_WebManager::DispatchJavaScriptMethod(C_WebTab* pWebTab, std::string objectName, std::string objectMethod, std::vector<std::string> methodArguments)
{
	m_pWebBrowser->DispatchJavaScriptMethod(pWebTab, objectName, objectMethod, methodArguments);
	/*
	for (auto arg : args)
	{
		DevMsg("Argument: %s\n", arg->text.c_str());
	}
	*/
}

//void DispatchJavaScriptMethodBatch(C_WebTab* pWebTab, std::vector<MethodBatch_t*> batch);