#include "cbase.h"

#include "aa_globals.h"
#include "c_mountmanager.h"
#include "c_anarchymanager.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_MountManager::C_MountManager()
{
	DevMsg("MountManager: Constructor\n");
}

C_MountManager::~C_MountManager()
{
	DevMsg("MountManager: Destructor\n");
}

void C_MountManager::Init()
{
	DevMsg("MountManager: Init\n");

	// Get all Steam library folders
	HKEY key;
	if (RegOpenKey(HKEY_CURRENT_USER, TEXT("Software\\Valve\\Steam"), &key) == ERROR_SUCCESS)
	{
		char value[AA_MAX_STRING];
		DWORD value_length = AA_MAX_STRING;
		DWORD flags = REG_SZ;
		RegQueryValueEx(key, "SteamPath", NULL, &flags, (LPBYTE)&value, &value_length);
		RegCloseKey(key);

		V_FixSlashes(value);
		/*
		if (value[3] == '\\')
		{
			std::string buf = value;
			buf = buf.substr(0, 2) + buf.substr(3);
			Q_strcpy(value, buf.c_str());
		}
		*/

		// primary
		m_libraryPaths.push_back(VarArgs("%s\\SteamApps\\common\\", value));

		// primary mod
		//if (g_pFullFileSystem->FindFirstEx(VarArgs("%s\\SteamApps\\sourcemods", value), "", &pHandle))
		if (g_pFullFileSystem->IsDirectory(VarArgs("%s\\SteamApps\\sourcemods", value)))
			m_libraryPaths.push_back(VarArgs("%s\\SteamApps\\sourcemods\\", value));
		//g_pFullFileSystem->FindClose(pHandle);

		// load %value/config/config.vdf to find other library paths
		KeyValues* pConfigKV = new KeyValues("InstallConfigStore");
		//pConfigKV->UsesEscapeSequences(true);
		if (pConfigKV->LoadFromFile(g_pFullFileSystem, VarArgs("%s\\config\\config.vdf", value), ""))
		{
			// For now, try searching until a search path is not found.
			// However, search paths may not be sequential, there might be numbers missing.
			// If that turns out to be the case, just check for the 1st 100 paths or something.

			std::string buf;
			std::string base = "Software/Valve/Steam/BaseInstallFolder_";
			KeyValues* pKey = pConfigKV->FindKey(VarArgs("%s1", base.c_str()));
			for (int i = 2; pKey; i++)
			{
				buf = pKey->GetString();
				if (buf.at(3) == '\\')
					buf = buf.substr(0, 2) + buf.substr(3);

				DevMsg("here it found: %s\n", buf.c_str());
				m_libraryPaths.push_back(VarArgs("%s\\SteamApps\\common\\", buf.c_str()));

				if (g_pFullFileSystem->IsDirectory(VarArgs("%s\\SteamApps\\sourcemods", buf.c_str())))
					m_libraryPaths.push_back(VarArgs("%s\\SteamApps\\sourcemods\\", buf.c_str()));
				
				pKey = pConfigKV->FindKey(VarArgs("%s%i", base.c_str(), i));
			}
		}
		pConfigKV->deleteThis();
	}

	/*
	for (unsigned int i = 0; i < m_libraryPaths.size(); i++)
	{
		DevMsg("Path: %s\n", m_libraryPaths[i].c_str());
	}
	*/
}

bool C_MountManager::LoadMountsFromKeyValues(std::string filename)
{
	unsigned int mountCount = 0;
	KeyValues* pKv = new KeyValues("mounts");
	if (pKv->LoadFromFile(g_pFullFileSystem, filename.c_str(), "MOD"))
	{
		for (KeyValues *pMountKv = pKv->GetFirstSubKey(); pMountKv; pMountKv = pMountKv->GetNextKey())
		{
			C_Mount* pMount = new C_Mount();

			std::string id = pMountKv->GetString("id");
			std::string title = pMountKv->GetString("title");
			std::string base = pMountKv->GetString("base");
			std::vector<std::string> paths;
			for (KeyValues *sub = pMountKv->FindKey("paths", true)->GetFirstSubKey(); sub; sub = sub->GetNextKey())
				paths.push_back(sub->GetString());

			pMount->Init(id, title, base, paths);

			//		if (pKv->GetBool("active"))
			//	{
			if (pMount->Activate())
				mountCount++;

			m_mounts[id] = pMount;
			//		}
		}
	}

	pKv->deleteThis();

	std::string num = VarArgs("%u", mountCount);
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Mounting Source Engine Games", "mounts", "0", num, num);

	return false;
}

// NOTE: THe file should be a FULL file path, not a relative one.
C_Mount* C_MountManager::FindOwningMount(std::string file)
{
	//DevMsg("And here file is: %s\n", file.c_str());
	std::map<std::string, C_Mount*>::iterator it = m_mounts.begin();
	while (it != m_mounts.end())
	{
		if (it->second->DoesOwn(file))
			return it->second;

		it++;
	}

	return null;
}