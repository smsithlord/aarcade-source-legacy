#include "cbase.h"

//#include "aa_globals.h"
#include "c_steambrowsermanager.h"
#include "c_anarchymanager.h"
//#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_SteamBrowserManager::C_SteamBrowserManager()
{
	DevMsg("SteamBrowserManager: Constructor\n");
	m_bSoundEnabled = true;
	m_pSelectedSteamBrowserInstance = null;

	steamapicontext->SteamHTMLSurface()->Init();

	m_pInputListener = new C_InputListenerSteamBrowser();
}

C_SteamBrowserManager::~C_SteamBrowserManager()
{
	DevMsg("SteamBrowserManager: Destructor\n");

	// iterate over all web tabs and call their destructors
	for (auto it = m_steamBrowserInstances.begin(); it != m_steamBrowserInstances.end(); ++it)
	{
		C_SteamBrowserInstance* pSteamBrowserInstance = it->second;

		if (pSteamBrowserInstance == m_pSelectedSteamBrowserInstance)
		{
			this->SelectSteamBrowserInstance(null);
			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
		}

//		if (g_pAnarchyManager->GetInputManager()->GetInputCanvasTexture() == pSteamBrowserInstance->GetTexture())
		if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pSteamBrowserInstance)
		{
			g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
			//g_pAnarchyManager->GetInputManager()->SetInputListener(null);
			//g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(null);
		}

//		auto foundSteamBrowserInstance = m_steamBrowserInstances.find(pSteamBrowserInstance->GetId());
//		if (foundSteamBrowserInstance != m_steamBrowserInstances.end())
//			m_steamBrowserInstances.erase(foundSteamBrowserInstance);

		pSteamBrowserInstance->SelfDestruct();
	}

	m_steamBrowserInstances.clear();

	if ( m_pInputListener )
		delete m_pInputListener;
}

void C_SteamBrowserManager::Update()
{
	for (auto it = m_steamBrowserInstances.begin(); it != m_steamBrowserInstances.end(); ++it)
	{
		C_SteamBrowserInstance* pSteamBrowserInstance = it->second;
		pSteamBrowserInstance->Update();
	}

	//DevMsg("SteamBrowserManager: Update\n");
	//info->state = state;
//	if (m_pSelectedSteamBrowserInstance)
//		m_pSelectedSteamBrowserInstance->Update();
}

C_SteamBrowserInstance* C_SteamBrowserManager::CreateSteamBrowserInstance()
{
	C_SteamBrowserInstance* pSteamBrowserInstance = new C_SteamBrowserInstance();
	SelectSteamBrowserInstance(pSteamBrowserInstance);
	return pSteamBrowserInstance;
}

bool C_SteamBrowserManager::SelectSteamBrowserInstance(C_SteamBrowserInstance* pSteamBrowserInstance)
{
	m_pSelectedSteamBrowserInstance = pSteamBrowserInstance;
	return true;
}

void C_SteamBrowserManager::OnSteamBrowserInstanceCreated(C_SteamBrowserInstance* pSteamBrowserInstance)
{
	std::string id = pSteamBrowserInstance->GetId();
	m_steamBrowserInstances[id] = pSteamBrowserInstance;

	//pSteamBrowserInstance->GetInfo()->state = 1;
}

/*
C_SteamBrowserInstance* C_SteamBrowserManager::FindSteamBrowserInstance(CSysModule* pModule)
{
	auto foundSteamBrowserInstance = m_steamBrowserInstances.find(pModule);
	if (foundSteamBrowserInstance != m_steamBrowserInstances.end())
	{
		return m_steamBrowserInstances[pModule];
	}
	else
		return null;
}
*/

C_SteamBrowserInstance* C_SteamBrowserManager::FindSteamBrowserInstance(std::string id)
{
	auto foundSteamBrowserInstance = m_steamBrowserInstances.find(id);
	if (foundSteamBrowserInstance != m_steamBrowserInstances.end())
	{
		return foundSteamBrowserInstance->second;
			//return m_steamBrowserInstances[foundSteamBrowserInstance];
	}
	else
		return null;
}


void C_SteamBrowserManager::RunEmbeddedSteamBrowser()
{
	C_SteamBrowserInstance* pSteamBrowserInstance = this->CreateSteamBrowserInstance();
	pSteamBrowserInstance->Init("", "http://smarcade.net/dlcv2/view_youtube.php?id=CmRih_VtVAs&autoplay=1", null);

	// http://anarchyarcade.com/press.html
	// https://www.youtube.com/html5
	// http://smarcade.net/dlcv2/view_youtube.php?id=CmRih_VtVAs&autoplay=1
}

void C_SteamBrowserManager::DestroySteamBrowserInstance(C_SteamBrowserInstance* pInstance)
{
	if (pInstance == m_pSelectedSteamBrowserInstance)
	{
		this->SelectSteamBrowserInstance(null);
		g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
	}

	//if (g_pAnarchyManager->GetInputManager()->GetInputCanvasTexture() == pInstance->GetTexture())
	if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance)
	{
		g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
		//g_pAnarchyManager->GetInputManager()->SetInputListener(null);
		//g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(null);
	}

	auto foundSteamBrowserInstance = m_steamBrowserInstances.find(pInstance->GetId());
	if (foundSteamBrowserInstance != m_steamBrowserInstances.end())
		m_steamBrowserInstances.erase(foundSteamBrowserInstance);

	pInstance->SelfDestruct();
}