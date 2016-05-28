#include "cbase.h"

//#include "aa_globals.h"
#include "c_webmanager.h"
//#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_WebManager::C_WebManager()
{
	DevMsg("WebManager: Constructor\n");
	m_iState = 0;	// uninitialized
	m_pWebBrowser = null;
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

	if (m_pWebBrowser)
		m_pWebBrowser->Update();

	//if (m_iState != 2)
	//	return;
}

C_WebTab* C_WebManager::GetWebTab(std::string id)
{
	auto foundWebTab = m_webTabs.find(id);
	if (foundWebTab != m_webTabs.end())
	{
		return m_webTabs[id];
	}
	else
		return null;
}

C_WebTab* C_WebManager::CreateWebTab(std::string url)
{
	DevMsg("WebManager: CreateWebTab\n");
	C_WebTab* pWebTab = new C_WebTab(url);
	m_webTabs[pWebTab->GetId()] = pWebTab;
	return pWebTab;
}