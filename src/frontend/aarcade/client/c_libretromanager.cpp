#include "cbase.h"

//#include "aa_globals.h"
#include "c_libretromanager.h"
#include "c_anarchymanager.h"
//#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_LibretroManager::C_LibretroManager()
{
	DevMsg("LibretroManager: Constructor\n");
	m_bSoundEnabled = true;
	m_pSelectedLibretroInstance = null;
	m_pFocusedLibretroInstance = null;

	m_pInputListener = new C_InputListenerLibretro();
}

C_LibretroManager::~C_LibretroManager()
{
	DevMsg("LibretroManager: Destructor\n");
	this->CloseAllInstances();

	if (m_pInputListener)
		delete m_pInputListener;
}

void C_LibretroManager::CloseAllInstances()
{
	unsigned int count = 0;
	// iterate over all web tabs and call their destructors
	for (auto it = m_libretroInstances.begin(); it != m_libretroInstances.end(); ++it)
	{
		C_LibretroInstance* pInstance = it->second;

		if (pInstance->GetId() == "hud" || pInstance->GetId() == "images")
		{
			DevMsg("ERROR: Libretro Browser instance detected that is NOT of type Awesomium Browser!!\n");
			continue;
		}

		DevMsg("Removing 1 Libretro instance...\n");

		if (pInstance == m_pSelectedLibretroInstance)
		{
			this->SelectLibretroInstance(null);
			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
		}

		//if (g_pAnarchyManager->GetInputManager()->GetInputCanvasTexture() == pInstance->GetTexture())
		if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance)
		{
			g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
			//g_pAnarchyManager->GetInputManager()->SetInputListener(null);
			//g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(null);
		}

		//		auto foundSteamBrowserInstance = m_steamBrowserInstances.find(pSteamBrowserInstance->GetId());
		//		if (foundSteamBrowserInstance != m_steamBrowserInstances.end())
		//			m_steamBrowserInstances.erase(foundSteamBrowserInstance);

		pInstance->SelfDestruct();
	}

	m_libretroInstances.clear();
}

void C_LibretroManager::Update()
{
	/*
	for (auto it = m_libretroInstances.begin(); it != m_libretroInstances.end(); ++it)
	{
		C_LibretroInstance* pLibretroInstance = it->second;
		pLibretroInstance->Update();

	}
	*/

	for (auto it = m_libretroInstances.begin(); it != m_libretroInstances.end(); ++it)
	{
		if (g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(it->second))
			it->second->Update();
	}

//	if (m_pSelectedLibretroInstance)
	//	m_pSelectedLibretroInstance->Update();

	//DevMsg("LibretroManager: Update\n");
	//info->state = state;
//	if (m_pSelectedLibretroInstance)
//		m_pSelectedLibretroInstance->Update();
}

C_LibretroInstance* C_LibretroManager::CreateLibretroInstance()
{
	C_LibretroInstance* pLibretroInstance = new C_LibretroInstance();
	SelectLibretroInstance(pLibretroInstance);
	return pLibretroInstance;
}

void C_LibretroManager::DestroyLibretroInstance(C_LibretroInstance* pInstance)
{
	if (pInstance == m_pSelectedLibretroInstance)
	{
		this->SelectLibretroInstance(null);
		g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
	}

	//if (g_pAnarchyManager->GetInputManager()->GetInputCanvasTexture() == pInstance->GetTexture())
	if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance)
	{
		g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
		//g_pAnarchyManager->GetInputManager()->SetInputListener(null);
		//g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(null);
	}

	auto foundLibretroInstance = m_libretroInstances.find(pInstance->GetInfo()->module);
	if (foundLibretroInstance != m_libretroInstances.end())
		m_libretroInstances.erase(foundLibretroInstance);

	pInstance->SelfDestruct();
}

bool C_LibretroManager::FocusLibretroInstance(C_LibretroInstance* pLibretroInstance)
{
	m_pFocusedLibretroInstance = pLibretroInstance;
	return true;
}

bool C_LibretroManager::SelectLibretroInstance(C_LibretroInstance* pLibretroInstance)
{
	m_pSelectedLibretroInstance = pLibretroInstance;
	return true;
}

void C_LibretroManager::OnLibretroInstanceCreated(LibretroInstanceInfo_t* pInfo)//C_LibretroInstance* pLibretroInstance)
{
	// FIXME: If you un-click fast after selecting something, u might trigger a crash here...
	//LibretroInstanceInfo_t* pInfo = (pLibretroInstance) ? pLibretroInstance->GetInfo() : null;
	if (!pInfo || pInfo->close || !pInfo->libretroinstance)
	{
		DevMsg("Extinct libretro instance ignored.\n");
		return;
	}

	CSysModule* pModule = pInfo->module;
	m_libretroInstances[pModule] = pInfo->libretroinstance;

	uint uId = pInfo->threadid;
	m_libretroInstancesModules[uId] = pModule;

	pInfo->state = 3;
}

C_LibretroInstance* C_LibretroManager::FindLibretroInstance(CSysModule* pModule)
{
	auto foundLibretroInstance = m_libretroInstances.find(pModule);
	if (foundLibretroInstance != m_libretroInstances.end())
	{
		return m_libretroInstances[pModule];
	}
	else
		return null;
}

C_LibretroInstance* C_LibretroManager::FindLibretroInstance(uint uId)
{
	auto foundId = m_libretroInstancesModules.find(uId);
	if (foundId != m_libretroInstancesModules.end())
	{
		CSysModule* pModule = m_libretroInstancesModules[uId];

		auto foundWebTab = m_libretroInstances.find(pModule);
		if (foundWebTab != m_libretroInstances.end())
			return m_libretroInstances[pModule];
	}

	return null;
}

C_LibretroInstance* C_LibretroManager::FindLibretroInstance(std::string id)
{
	//typedef std::map<std::string, std::map<std::string, std::string>>::iterator it_type;
	auto foundLibretroInstance = m_libretroInstances.begin();
	while (foundLibretroInstance != m_libretroInstances.end())
	{
		if (foundLibretroInstance->second->GetId() == id)
			return foundLibretroInstance->second;
		else
			foundLibretroInstance++;
	}

	return null;
}

void C_LibretroManager::RunEmbeddedLibretro(std::string file)
{
	// TEST: AUTO-CREATE AN INSTANCE, LOAD THE FFMPEG CORE, AND PLAY A MOVIE
	C_LibretroInstance* pLibretroInstance = this->CreateLibretroInstance();
	pLibretroInstance->Init();

	// load a core
	pLibretroInstance->LoadCore();

	// tell the input manager that the libretro instance is active
	//C_InputListenerLibretro* pListener = this->GetInputListener();
	//g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(pLibretroInstance->GetTexture());
	//pLibretroInstance->Select();
	//g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pLibretroInstance);

	pLibretroInstance->Focus();
	//g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(pSteamBrowserInstance);
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pLibretroInstance);

	// load a file
	pLibretroInstance->SetGame(file);
}

unsigned int C_LibretroManager::GetInstanceCount()
{
	unsigned int count = 0;

	auto it = m_libretroInstances.begin();
	while (it != m_libretroInstances.end())
	{
		count++;
		it++;
	}

	return count;
}