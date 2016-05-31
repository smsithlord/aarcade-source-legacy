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
	m_iVisibleWebTabsLastFrame = -1;
	m_iVisibleWebTabsCurrentFrame = 0;
	m_iLastRenderedFrame = -1;
	m_pWebBrowser = null;
	m_pWebSurfaceRegen = null;
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
	int iLastRenderedFrame = pWebTab->GetLastRenderedFrame();
	if (m_iLastRenderedFrame != gpGlobals->framecount && (iLastRenderedFrame <= 0 || m_iVisibleWebTabsLastFrame <= 1 || gpGlobals->framecount - iLastRenderedFrame >= m_iVisibleWebTabsLastFrame))
		return true;

	return false;
}