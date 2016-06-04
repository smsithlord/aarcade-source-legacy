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
	m_pSelectedLibretroInstance = null;
}

C_LibretroManager::~C_LibretroManager()
{
	DevMsg("LibretroManager: Destructor\n");
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
	if (m_pSelectedLibretroInstance)
		m_pSelectedLibretroInstance->Update();
}

C_LibretroInstance* C_LibretroManager::CreateLibretroInstance()
{
	C_LibretroInstance* pLibretroInstance = new C_LibretroInstance();
	SelectLibretroInstance(pLibretroInstance);

	pLibretroInstance->Init();
	pLibretroInstance->LoadCore();
	return null;
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