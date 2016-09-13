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

	m_pInputListener = new C_InputListenerLibretro();
}

C_LibretroManager::~C_LibretroManager()
{
	DevMsg("LibretroManager: Destructor\n");

	// iterate over all web tabs and call their destructors
	for (auto it = m_libretroInstances.begin(); it != m_libretroInstances.end(); ++it)
	{
		C_LibretroInstance* pInstance = it->second;

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

	if (m_pInputListener)
		delete m_pInputListener;
}

void C_LibretroManager::Update()
{
	for (auto it = m_libretroInstances.begin(); it != m_libretroInstances.end(); ++it)
	{
		C_LibretroInstance* pLibretroInstance = it->second;
		pLibretroInstance->Update();

	}

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

bool C_LibretroManager::SelectLibretroInstance(C_LibretroInstance* pLibretroInstance)
{
	m_pSelectedLibretroInstance = pLibretroInstance;
	return true;
}

void C_LibretroManager::OnLibretroInstanceCreated(C_LibretroInstance* pLibretroInstance)
{
	CSysModule* pModule = pLibretroInstance->GetInfo()->module;
	m_libretroInstances[pModule] = pLibretroInstance;

	uint uId = pLibretroInstance->GetInfo()->threadid;
	m_libretroInstancesModules[uId] = pModule;

	pLibretroInstance->GetInfo()->state = 1;
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

void C_LibretroManager::RunEmbeddedLibretro()
{
	// TEST: AUTO-CREATE AN INSTANCE, LOAD THE FFMPEG CORE, AND PLAY A MOVIE
	C_LibretroInstance* pLibretroInstance = this->CreateLibretroInstance();
	pLibretroInstance->Init();

	// load a core
	pLibretroInstance->LoadCore();

	// load a file
	pLibretroInstance->LoadGame();

	// tell the input manager that the libretro instance is active
	//C_InputListenerLibretro* pListener = this->GetInputListener();
	//g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(pLibretroInstance->GetTexture());
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pLibretroInstance);
}