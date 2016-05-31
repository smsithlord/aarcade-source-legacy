#include "cbase.h"

//#include "aa_globals.h"
#include "c_webmanager.h"
#include "c_anarchymanager.h"
//#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_WebManager::C_WebManager()
{
	DevMsg("WebManager: Constructor\n");
	m_iState = 0;	// uninitialized
	m_iWebSurfaceWidth = 1280;
	m_iWebSurfaceHeight = 720;
	m_iVisibleWebTabsLastTick = -1;
	m_iVisibleWebTabsCurrentTick = 0;
	m_iLastRenderedTick = -1;
	m_pWebBrowser = null;
	m_pWebSurfaceRegen = null;
}

C_WebManager::~C_WebManager()
{
	DevMsg("WebManager: Destructor\n");
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
	m_iVisibleWebTabsLastTick = m_iVisibleWebTabsCurrentTick;
	m_iVisibleWebTabsCurrentTick = 0;

	// Update the browser, if it exists
	if (m_pWebBrowser)
		m_pWebBrowser->Update();
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

C_WebTab* C_WebManager::CreateWebTab(std::string url, std::string id)
{
	DevMsg("WebManager: CreateWebTab\n");

	std::string validId;
	if (id == "")
		validId = g_pAnarchyManager->GenerateUniqueId();
	else
		validId = id;	// FIXME: Might want to do a check to make sure a tab with this ID doesn't already exist.

	C_WebTab* pWebTab = new C_WebTab(url, validId);
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
	int iLastRenderedTick = pWebTab->GetLastRenderedTick();
	if (m_iLastRenderedTick != gpGlobals->tickcount && (iLastRenderedTick <= 0 || gpGlobals->tickcount - iLastRenderedTick >= m_iVisibleWebTabsLastTick))
		return true;

	return false;
}