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
	m_iWebSurfaceWidth = 1280;
	m_iWebSurfaceHeight = 720;
	//m_iWebSurfaceWidth = 1920;
	//m_iWebSurfaceHeight = 1080;
	m_bHudPriority = true;// false;
	m_bSelectedPriority = true;// false;
	m_iVisibleWebTabsLastFrame = -1;
	m_iVisiblePriorityWebTabsLastFrame = -1;
	m_iVisibleWebTabsCurrentFrame = 0;
	m_iVisiblePriorityWebTabsCurrentFrame = 0;
	m_iLastRenderedFrame = -1;
	m_iLastPriorityRenderedFrame = -1;
	m_pWebBrowser = null;
	m_pWebSurfaceRegen = null;
	m_pSelectedWebTab = null;
	m_pFocusedWebTab = null;
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

	delete m_pWebSurfaceRegen;

	CWebSurfaceProxy* pProxy;
	while (!m_webSurfaceProxies.empty())
	{
		pProxy = m_webSurfaceProxies[m_webSurfaceProxies.size() - 1];
		pProxy->Release();
		m_webSurfaceProxies.pop_back();
	}
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

	m_iVisiblePriorityWebTabsLastFrame = m_iVisiblePriorityWebTabsCurrentFrame;
	m_iVisiblePriorityWebTabsCurrentFrame = 0;

	// Update the browser, if it exists
	if (m_pWebBrowser)
		m_pWebBrowser->Update();

	// assume that the hud and selected web tabs are priority
	if (m_pSelectedWebTab && m_pSelectedWebTab->GetId() != "images")
	{
		m_pSelectedWebTab->OnProxyBind();
		g_pAnarchyManager->GetWebManager()->IncrementVisiblePriorityWebTabsCurrentFrame();
	}
	
	if (m_pHudWebTab && (m_pHudWebTab != m_pSelectedWebTab || !m_pSelectedWebTab || m_pSelectedWebTab->GetId() == "images") && g_pAnarchyManager->GetInputManager()->GetInputMode())
	{
		m_pHudWebTab->OnProxyBind();
		g_pAnarchyManager->GetWebManager()->IncrementVisiblePriorityWebTabsCurrentFrame();
	}

	// FIXME: the selected web tab always renders
	/*
	if (m_pSelectedWebTab)
	{
		m_pSelectedWebTab->OnProxyBind();

		if (!m_bSelectedPriority)
			g_pAnarchyManager->GetWebManager()->IncrementVisibleWebTabsCurrentFrame();
	}
	*/

	// FIXME: the hud web tab always renders
	//if (m_pHudWebTab && m_pHudWebTab != m_pSelectedWebTab && m_pSelectedWebTab)
	/*
	if (m_pHudWebTab && g_pAnarchyManager->GetInputManager()->GetInputMode())
	{
		m_pHudWebTab->OnProxyBind();

		if ( !m_bHudPriority )
			g_pAnarchyManager->GetWebManager()->IncrementVisibleWebTabsCurrentFrame();
	}
	*/
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

void C_WebManager::FocusWebTab(C_WebTab* pWebTab)
{
	if (m_pFocusedWebTab)
		this->UnfocusWebTab(m_pFocusedWebTab);

	m_pFocusedWebTab = pWebTab;
	g_pAnarchyManager->GetWebManager()->GetWebBrowser()->FocusWebTab(pWebTab);
}

void C_WebManager::UnfocusWebTab(C_WebTab* pWebTab)
{
	if (m_pFocusedWebTab != pWebTab)
		return;

	g_pAnarchyManager->GetWebManager()->GetWebBrowser()->UnfocusWebTab(pWebTab);
	m_pFocusedWebTab = null;
}

void C_WebManager::SelectWebTab(C_WebTab* pWebTab)
{
	g_pAnarchyManager->GetInputManager()->SetInputListener(g_pAnarchyManager->GetWebManager(), LISTENER_WEB_MANAGER);

	m_pSelectedWebTab = pWebTab;

	// give focus to the newly selected web tab
	this->FocusWebTab(pWebTab);
	//m_pWebBrowser->OnSelectWebTab(pWebTab);

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
	this->UnfocusWebTab(pWebTab);
	//m_pWebBrowser->OnDeselectWebTab(pWebTab);

	g_pAnarchyManager->GetInputManager()->SetInputListener(null, LISTENER_NONE);

	if (pWebTab->GetId().find("auto") == 0)
		this->RemoveWebTab(pWebTab);

	DevMsg("done removing web tab\n");
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

void C_WebManager::LevelShutdownPreEntity()
{
	// close all webtabs except for important ones
	C_WebTab* victim;
	std::map<std::string, C_WebTab*>::iterator it = m_webTabs.begin();
	while (it != m_webTabs.end())
	{
		victim = it->second;
		it++;

		if (victim != m_pHudWebTab)
			RemoveWebTab(victim);
	}

	unsigned int i;
	unsigned int numProxies = m_webSurfaceProxies.size();
	for (i = 0; i < numProxies; i++)
		m_webSurfaceProxies[i]->LevelShutdownPreEntity();
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

void C_WebManager::OnActivateInputMode(bool bFullscreenMode)
{
	// notify the HUD that input mode has been activated (so it can update its "input lock/pin" button mostly)
	std::vector<std::string> args;
	args.push_back(VarArgs("%i", (bFullscreenMode)));
	args.push_back(VarArgs("%i", (g_pAnarchyManager->GetInputManager()->GetWasForceInputMode())));	// FIXME: There will probably be other ways for the HUD to be pinned, such as pressing ESC to bring it up.
	
	std::string mapName = VarArgs("%s", g_pAnarchyManager->MapName());
	args.push_back(VarArgs("%i", (mapName != "(null)")));

	this->DispatchJavaScriptMethod(m_pHudWebTab, "arcadeHud", "onActivateInputMode", args);
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
	C_WebTab* pWebTab = CreateWebTab("asset://ui/startup.html", "hud", true);
	//g_pAnarchyManager->GetWebManager()->SelectWebTab(pWebTab);
//	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true);
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

bool C_WebManager::IsPriorityWebTab(C_WebTab* pWebTab)
{
	return (pWebTab == m_pHudWebTab && this->GetHudPriority()) || (pWebTab == m_pSelectedWebTab && this->GetSelectedPriority());
}

bool C_WebManager::ShouldRender(C_WebTab* pWebTab)
{
	// don't render more than 1 web tab per frame
	if (m_iLastRenderedFrame == gpGlobals->framecount || m_iLastPriorityRenderedFrame == gpGlobals->framecount)
		return false;

	bool bIsPriorityWebTab = this->IsPriorityWebTab(pWebTab);
	if (bIsPriorityWebTab)
	{
//		DevMsg("num priority web views: %i\n", m_iVisiblePriorityWebTabsLastFrame);
		int iLastRenderedFrame = pWebTab->GetLastRenderedFrame();
		//int iLastPriorityRenderedFrame = pWebTab->GetLastRenderedFrame();
		//SetLastPriorityRenderedFrame
		// we are a priority web tab

		/*
		// don't render if there are still regular web tabs waiting to render
		if (gpGlobals->framecount - m_iLastRenderedFrame >= m_iVisibleWebTabsLastFrame)
			return false;
		*/

		// always render the 1st time
		if (iLastRenderedFrame <= 0)
			return true;

		// render if we're the only visible (priority) web tab
	//	if (m_iVisiblePriorityWebTabsLastFrame <= 1)
	//		return true;

		// render if we've waited long enough for all other (priority) web views to render

		// Need to wait an extra tick if there are any non-priority web tabs waiting.
		int iExtraOne = 0;
		if (m_iVisibleWebTabsLastFrame > 0)
			iExtraOne = 1;

		if (gpGlobals->framecount - iLastRenderedFrame >= m_iVisiblePriorityWebTabsLastFrame + iExtraOne)
			return true;
	}
	else
	{
		//DevMsg("num priority web views: %i\n", m_iVisiblePriorityWebTabsLastFrame);
		int iLastRenderedFrame = pWebTab->GetLastRenderedFrame();

	//	int frameCount = m_iLastPriorityRenderedFrame;// gpGlobals->framecount - m_iLastPriorityRenderedFrame;
		// we are a regular web tab

		// don't render if there are still priority web tabs waiting to render
	//	if (m_iVisiblePriorityWebTabsLastFrame > 0 && frameCount - m_iLastPriorityRenderedFrame >= m_iVisiblePriorityWebTabsLastFrame)
		//	return false;

		// always render the 1st time
		if (iLastRenderedFrame <= 0)
			return true;

		// render if we're the only visible (regular) web tab
	//	if (m_iVisibleWebTabsLastFrame <= 1)
	//		return true;

		// render if we've waited long enough for all other (regular) web views to render
		//if (frameCount - iLastRenderedFrame >= m_iVisibleWebTabsLastFrame)
		if (m_iVisiblePriorityWebTabsLastFrame > 0 && m_iLastPriorityRenderedFrame - iLastRenderedFrame >= m_iVisiblePriorityWebTabsLastFrame + m_iVisibleWebTabsLastFrame - 1)
			return true;
		else if (m_iVisiblePriorityWebTabsLastFrame <= 0 && gpGlobals->framecount - iLastRenderedFrame >= m_iVisibleWebTabsLastFrame)
			return true;
	}

//	if (m_iLastRenderedFrame != gpGlobals->framecount && (iLastRenderedFrame <= 0 || m_iVisibleWebTabsLastFrame <= 1 || ((pWebTab == m_pHudWebTab || pWebTab == m_pSelectedWebTab) && m_iVisiblePriorityWebTabsLastFrame <= 1) || (((pWebTab == m_pHudWebTab || pWebTab == m_pSelectedWebTab) && m_iVisiblePriorityWebTabsLastFrame <= 1) || gpGlobals->framecount - iLastRenderedFrame >= m_iVisibleWebTabsLastFrame)))
//		return true;

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
	//m_pSelectedWebTab->MousePress(code);
	m_pHudWebTab->MousePress(code);
}

void C_WebManager::OnMouseRelease(vgui::MouseCode code)
{
	// these events are always for the selected web tab
	//m_pSelectedWebTab->MouseRelease(code);
	m_pHudWebTab->MouseRelease(code);
}

void C_WebManager::OnKeyCodePressed(vgui::MouseCode code, bool bShiftState, bool bCtrlState, bool bAltState)
{
	// these events are always for the selected web tab
//	m_pSelectedWebTab->KeyCodePress(code, bShiftState, bCtrlState, bAltState);
	m_pFocusedWebTab->KeyCodePress(code, bShiftState, bCtrlState, bAltState);
}

void C_WebManager::OnKeyCodeReleased(vgui::MouseCode code, bool bShiftState, bool bCtrlState, bool bAltState)
{
	// these events are always for the selected web tab
	//m_pSelectedWebTab->KeyCodeRelease(code, bShiftState, bCtrlState, bAltState);
	m_pFocusedWebTab->KeyCodeRelease(code, bShiftState, bCtrlState, bAltState);
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

void C_WebManager::DispatchJavaScriptMethods(C_WebTab* pWebTab)
{
	m_pWebBrowser->DispatchJavaScriptMethods(pWebTab);
}

void C_WebManager::RegisterProxy(CWebSurfaceProxy* pProxy)
{
	m_webSurfaceProxies.push_back(pProxy);
}