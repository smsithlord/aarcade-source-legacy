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
	m_pFocusedSteamBrowserInstance = null;
	//surface()->GetWebkitHTMLUserAgentString()
	steamapicontext->SteamHTMLSurface()->Init();
//	steamapicontext->SteamHTMLSurface()->

	m_pInputListener = new C_InputListenerSteamBrowser();
}

C_SteamBrowserManager::~C_SteamBrowserManager()
{
	DevMsg("SteamBrowserManager: Destructor\n");
	this->CloseAllInstances();

	if (m_pInputListener)
		delete m_pInputListener;
}

void C_SteamBrowserManager::Update()
{
	/*
	for (auto it = m_steamBrowserInstances.begin(); it != m_steamBrowserInstances.end(); ++it)
	{
		C_SteamBrowserInstance* pSteamBrowserInstance = it->second;
		pSteamBrowserInstance->Update();
	}
	*/

	//DevMsg("SteamBrowserManager: Update\n");
	//info->state = state;
	//if (m_pSelectedSteamBrowserInstance)
		//m_pSelectedSteamBrowserInstance->Update();

	for (auto it = m_steamBrowserInstances.begin(); it != m_steamBrowserInstances.end(); ++it)
	{
		if (g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(it->second))
			it->second->Update();
	}


	/*
	for (auto it = m_steamBrowserInstances.begin(); it != m_steamBrowserInstances.end(); ++it)
	{
		C_SteamBrowserInstance* pSteamBrowserInstance = it->second;
		if (pSteamBrowserInstance != m_pSelectedSteamBrowserInstance)
			pSteamBrowserInstance->Update();
	}
	*/
}

void C_SteamBrowserManager::CloseAllInstances()
{
	// iterate over all web tabs and call their destructors
	for (auto it = m_steamBrowserInstances.begin(); it != m_steamBrowserInstances.end(); ++it)
	{
		C_SteamBrowserInstance* pSteamBrowserInstance = it->second;

		if (pSteamBrowserInstance->GetId() == "hud" || pSteamBrowserInstance->GetId()== "images")
		{
			DevMsg("ERROR: Steam Browser instance detected that is NOT of type Awesomium Browser!!\n");
			continue;
		}

		DevMsg("Removing 1 Steam instance...\n");

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
}

C_SteamBrowserInstance* C_SteamBrowserManager::CreateSteamBrowserInstance()
{
	C_SteamBrowserInstance* pSteamBrowserInstance = new C_SteamBrowserInstance();
	SelectSteamBrowserInstance(pSteamBrowserInstance);
	return pSteamBrowserInstance;
}

bool C_SteamBrowserManager::FocusSteamBrowserInstance(C_SteamBrowserInstance* pSteamBrowserInstance)
{
	if (pSteamBrowserInstance && pSteamBrowserInstance->GetHandle())
		steamapicontext->SteamHTMLSurface()->SetKeyFocus(pSteamBrowserInstance->GetHandle(), true);

	m_pFocusedSteamBrowserInstance = pSteamBrowserInstance;
	return true;
}

bool C_SteamBrowserManager::SelectSteamBrowserInstance(C_SteamBrowserInstance* pSteamBrowserInstance)
{
	m_pSelectedSteamBrowserInstance = pSteamBrowserInstance;
	return true;
}

void C_SteamBrowserManager::AddFreshSteamBrowserInstance(C_SteamBrowserInstance* pSteamBrowserInstance)
{
	std::string id = pSteamBrowserInstance->GetId();
	m_steamBrowserInstances[id] = pSteamBrowserInstance;
}

void C_SteamBrowserManager::OnSteamBrowserInstanceCreated(C_SteamBrowserInstance* pSteamBrowserInstance)
{
	//std::string id = pSteamBrowserInstance->GetId();
	//m_steamBrowserInstances[id] = pSteamBrowserInstance;












	// old unknown shit
	//m_steamBrowserInstanceIds[pSteamBrowserInstance->GetHandle()] = pSteamBrowserInstance->GetId();
	//pSteamBrowserInstance->GetInfo()->state = 1;
}

C_SteamBrowserInstance* C_SteamBrowserManager::FindSteamBrowserInstance(unsigned int unHandle)
{
	auto foundSteamBrowserInstance = m_steamBrowserInstances.begin();
	while (foundSteamBrowserInstance != m_steamBrowserInstances.end())
	{
		if (foundSteamBrowserInstance->second->GetHandle() == unHandle)
			return foundSteamBrowserInstance->second;
		else
			foundSteamBrowserInstance++;
	}

	return null;
}

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

C_SteamBrowserInstance* C_SteamBrowserManager::GetPendingSteamBrowserInstance()
{
	auto foundSteamBrowserInstance = m_steamBrowserInstances.begin();
	while (foundSteamBrowserInstance != m_steamBrowserInstances.end())
	{
		if (foundSteamBrowserInstance->second->GetHandle() == 0)
			return foundSteamBrowserInstance->second;
		else
			foundSteamBrowserInstance++;
	}

	return null;
}

void C_SteamBrowserManager::RunEmbeddedSteamBrowser()
{
	C_SteamBrowserInstance* pSteamBrowserInstance = this->CreateSteamBrowserInstance();

//	pSteamBrowserInstance->Init("", "https://www.netflix.com/watch/217258", null);
	pSteamBrowserInstance->Init("", "http://www.youtube.com/", null);

//	pSteamBrowserInstance->Init("", "file:///C:/Users/Owner/Desktop/wowvr/index.html", null);
	pSteamBrowserInstance->Focus();
	//g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(pSteamBrowserInstance);
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pSteamBrowserInstance);

	// http://anarchyarcade.com/press.html
	// https://www.youtube.com/html5
	// http://smarcade.net/dlcv2/view_youtube.php?id=CmRih_VtVAs&autoplay=1
}

void C_SteamBrowserManager::DestroySteamBrowserInstance(C_SteamBrowserInstance* pInstance)
{
	if (pInstance == m_pSelectedSteamBrowserInstance)
		this->SelectSteamBrowserInstance(null);

	if (pInstance == m_pFocusedSteamBrowserInstance)
		this->FocusSteamBrowserInstance(null);

	//if (g_pAnarchyManager->GetInputManager()->GetInputCanvasTexture() == pInstance->GetTexture())
	if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance)
	{
		g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);

		if (g_pAnarchyManager->GetInputManager()->GetInputMode())
			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
		//g_pAnarchyManager->GetInputManager()->SetInputListener(null);
		//g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(null);
	}

	auto foundSteamBrowserInstance = m_steamBrowserInstances.find(pInstance->GetId());
	if (foundSteamBrowserInstance != m_steamBrowserInstances.end())
		m_steamBrowserInstances.erase(foundSteamBrowserInstance);

	pInstance->SelfDestruct();
}

void C_SteamBrowserManager::GetAllInstances(std::vector<C_EmbeddedInstance*>& embeddedInstances)
{
	auto it = m_steamBrowserInstances.begin();
	while (it != m_steamBrowserInstances.end())
	{
		embeddedInstances.push_back(it->second);
		it++;
	}
}