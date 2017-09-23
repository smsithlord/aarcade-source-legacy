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
	m_pBrowserListener = new C_SteamBrowserListener();
	m_bSoundEnabled = true;
	m_pSelectedSteamBrowserInstance = null;
	m_pFocusedSteamBrowserInstance = null;

	// NOTE: NO STEAM ERROR MANIFESTS HERE.  Fails before devmsg can print.
	if (!steamapicontext->SteamHTMLSurface())
	{
		m_bSupported = false;
		DevMsg("CRITICAL ERROR: Failed to acquire the SteamHTMLSurface! Make sure Steam is running!\n");
		g_pAnarchyManager->ThrowEarlyError("Anarchy Arcade cannot connect to the Steamworks web browser.\nPlease restart Steam and try again.");
	}
	else if (!steamapicontext->SteamHTMLSurface()->Init())
	{
		m_bSupported = false;
		DevMsg("CRITICAL ERROR: Failed to initialize the Steamworks browser!\n");
		g_pAnarchyManager->ThrowEarlyError("Anarchy Arcade cannot connect to the Steamworks web browser.\nPlease restart Steam and try again.");
	}
	else
	{
		m_bSupported = true;
		m_pInputListener = new C_InputListenerSteamBrowser();
	}
}

C_SteamBrowserManager::~C_SteamBrowserManager()
{
	DevMsg("SteamBrowserManager: Destructor\n");
	this->CloseAllInstances();

	ISteamHTMLSurface* pHTMLSurface = steamapicontext->SteamHTMLSurface();
	if (!pHTMLSurface)
		DevMsg("ERROR: There was no SteamHTMLSurface to clean up!\n");
	else if (!pHTMLSurface->Shutdown())
		DevMsg("CRITICAL ERROR: Failed to shutdown the Steamworks browser!\n");

	if (m_pInputListener)
		delete m_pInputListener;
	
	if (m_pBrowserListener)
		delete m_pBrowserListener;
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

	unsigned int max = m_steamBrowserInstances.size();
	for (unsigned int i = 0; i < max; i++)
	{
		if (g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(m_steamBrowserInstances[i]))
			m_steamBrowserInstances[i]->Update();
	}

	/*
	for (auto it = m_steamBrowserInstances.begin(); it != m_steamBrowserInstances.end(); ++it)
	{
		if (g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(it->second))
			it->second->Update();
	}
	*/


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
	g_pAnarchyManager->GetCanvasManager()->SetDisplayInstance(null);

	unsigned int max = m_steamBrowserInstances.size();
	for (unsigned int i = 0; i < max; i++)
	{
		C_SteamBrowserInstance* pSteamBrowserInstance = m_steamBrowserInstances[i];
		DevMsg("Removing 1 Steam instance...\n");
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

bool C_SteamBrowserManager::IsAlreadyInInstances(C_SteamBrowserInstance* pSteamBrowserInstance)
{
	unsigned int max = m_steamBrowserInstances.size();
	for (unsigned int i = 0; i < max; i++)
	{
		if (m_steamBrowserInstances[i] == pSteamBrowserInstance)
			return true;
	}

	return false;
}

void C_SteamBrowserManager::AddFreshSteamBrowserInstance(C_SteamBrowserInstance* pSteamBrowserInstance)
{
	if (this->IsAlreadyInInstances(pSteamBrowserInstance))
		DevMsg("Warning: Fresh Steam browser instance already exists in the list!\n");
	else
		m_steamBrowserInstances.push_back(pSteamBrowserInstance);

	//std::string id = pSteamBrowserInstance->GetId();
	//m_steamBrowserInstances[id] = pSteamBrowserInstance;
}

void C_SteamBrowserManager::OnSteamBrowserInstanceCreated(C_SteamBrowserInstance* pSteamBrowserInstance)
{
	//std::string id = pSteamBrowserInstance->GetId();
	//m_steamBrowserInstances[id] = pSteamBrowserInstance;












	// old unknown shit
	//m_steamBrowserInstanceIds[pSteamBrowserInstance->GetHandle()] = pSteamBrowserInstance->GetId();
	//pSteamBrowserInstance->GetInfo()->state = 1;
}


C_SteamBrowserInstance* C_SteamBrowserManager::FindSteamBrowserInstanceByEntityIndex(int iEntityIndex)
{
	unsigned int max = m_steamBrowserInstances.size();
	for (unsigned int i = 0; i < max; i++)
	{
		if (m_steamBrowserInstances[i]->GetOriginalEntIndex() == iEntityIndex)
			return m_steamBrowserInstances[i];
	}

	return null;
}

C_SteamBrowserInstance* C_SteamBrowserManager::FindSteamBrowserInstance(unsigned int unHandle)
{
	unsigned int max = m_steamBrowserInstances.size();
	for (unsigned int i = 0; i < max; i++)
	{
		if (m_steamBrowserInstances[i]->GetHandle() == unHandle)// && !foundSteamBrowserInstance->second->IsDefunct())
			return m_steamBrowserInstances[i];
	}

	return null;
}

C_SteamBrowserInstance* C_SteamBrowserManager::FindSteamBrowserInstance(std::string id)
{
	unsigned int max = m_steamBrowserInstances.size();
	for (unsigned int i = 0; i < max; i++)
	{
		if (m_steamBrowserInstances[i]->GetId() == id)
			return m_steamBrowserInstances[i];
	}

	return null;
}

C_SteamBrowserInstance* C_SteamBrowserManager::GetPendingSteamBrowserInstance()
{
	unsigned int max = m_steamBrowserInstances.size();
	for (unsigned int i = 0; i < max; i++)
	{
		if (m_steamBrowserInstances[i]->GetHandle() == 0)
			return m_steamBrowserInstances[i];
	}

	return null;
}

void C_SteamBrowserManager::RunEmbeddedSteamBrowser()
{
	C_SteamBrowserInstance* pSteamBrowserInstance = this->CreateSteamBrowserInstance();

//	pSteamBrowserInstance->Init("", "https://www.netflix.com/watch/217258", null);
	pSteamBrowserInstance->Init("", "http://www.youtube.com/", "Manual Steamworks Browser Tab", null);

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
	/*
	if (pInstance == m_pSelectedSteamBrowserInstance)
	{
		DevMsg("Was the selected Steamworks browser instance.\n");
		this->SelectSteamBrowserInstance(null);
	}

	if (pInstance == m_pFocusedSteamBrowserInstance)
	{
		DevMsg("Was the focused Steamworks browser instance.\n");
		this->FocusSteamBrowserInstance(null);
	}

	//if (g_pAnarchyManager->GetCanvasManager()->GetDisplayInstance() == pInstance)
	//	g_pAnarchyManager->GetCanvasManager()->SetDisplayInstance(null);

	//if (g_pAnarchyManager->GetInputManager()->GetInputCanvasTexture() == pInstance->GetTexture())
	if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance)
	{
		DevMsg("Was the input manager embedded instance.\n");
		g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);

		if (g_pAnarchyManager->GetInputManager()->GetInputMode())
		{
			DevMsg("Was in input mode.\n");
			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
		}
		//g_pAnarchyManager->GetInputManager()->SetInputListener(null);
		//g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(null);
	}
	*/

	bool bWorked = false;
	unsigned int max = m_steamBrowserInstances.size();
	for (unsigned int i = 0; i < max; i++)
	{
		if (m_steamBrowserInstances[i]->GetId() == pInstance->GetId())
		{
			m_steamBrowserInstances.erase(m_steamBrowserInstances.begin() + i);
			bWorked = true;
		}
	}

	if (!bWorked)
		DevMsg("WARNING: Failed to remove Steam Browser instance!\n");

	pInstance->SelfDestruct();
}

unsigned int C_SteamBrowserManager::GetInstanceCount()
{
	return m_steamBrowserInstances.size();
}

// TODO: This function is kinda pointless now that m_steamBrowserInstances is a vector itself.  Could just return a pointer directly to it.
void C_SteamBrowserManager::GetAllInstances(std::vector<C_EmbeddedInstance*>& embeddedInstances)
{
	unsigned int max = m_steamBrowserInstances.size();
	for (unsigned int i = 0; i < max; i++)
		embeddedInstances.push_back(m_steamBrowserInstances[i]);
}

/*
C_SteamBrowserInstance* C_SteamBrowserManager::FindDefunctInstance(unsigned int unHandle)
{
	std::map<std::string, C_SteamBrowserInstance*>::iterator foundDefuncSteamBrowserInstance = m_defunctSteamBrowserInstances.begin();
	while (foundDefuncSteamBrowserInstance != m_defunctSteamBrowserInstances.end())
	{
		if (foundDefuncSteamBrowserInstance->second->GetHandle() == unHandle)
			return foundDefuncSteamBrowserInstance->second;
		else
			foundDefuncSteamBrowserInstance++;
	}

	return null;
}

void C_SteamBrowserManager::AddDefunctInstance(C_SteamBrowserInstance* pInstance)
{
	m_defunctSteamBrowserInstances[pInstance->GetId()] = pInstance;
}

bool C_SteamBrowserManager::DestroyDefunctInstance(C_SteamBrowserInstance* pInstance)
{
	DevMsg("Attempting to destroy defunct instance...\n");
	std::string id = pInstance->GetId();

	bool response;
	pInstance->DoDefunctDestruct(response);

	if (response)
	{
		DevMsg("Success!\n");
		std::map<std::string, C_SteamBrowserInstance*>::iterator it = m_defunctSteamBrowserInstances.find(id);
		if (it != m_defunctSteamBrowserInstances.end())
			m_defunctSteamBrowserInstances.erase(it);

		return true;
	}
	else
	{
		DevMsg("Failed!\n");
		return false;
	}
}
*/