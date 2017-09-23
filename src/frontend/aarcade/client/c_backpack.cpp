#include "cbase.h"

#include "c_anarchymanager.h"
#include "aa_globals.h"
#include "c_backpack.h"
#include <algorithm>
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// We are basically going to LEGACY EXPORT the contents of this folder (and its VPK) to this
// library.db file.  Actually, we will be literally doing that.  But in addition, we'll also
// be scanning the folder for MDLs.  We cannot assume that the MDLs in this folder all already
// have legacy ITM files for them.  This might not even be a legacy addon at all.  It might be
// random loose files the user has in a backpack.  We don't know.

// VERY IMPORTANT NOTE:
// The LEGACY WORKSHOP functions that are used to add GEN2 stuff to the active user library
// will be REWORKED to now generate THIS type of addon library.db.  They will LITERALLY be
// the same function.
// GEN2 stuff will GENERATE these library.db files when they are CONSUMED, but will USE the library.db when ACTIVATED (rather than generating it EVERY time)

C_Backpack::C_Backpack()
{
	DevMsg("C_Backpack::Constructor\n");
	m_bPrepared = false;
	m_bActive = false;
	m_pDb = null;
	//m_bActive = false;
}

C_Backpack::~C_Backpack()
{
	DevMsg("C_Backpack::Destructor\n");

	// m_apps
	while (!m_apps.empty())
	{
		m_apps.begin()->second->deleteThis();
		m_apps.erase(m_apps.begin());
	}

	// m_models
	while (!m_models.empty())
	{
		m_models.begin()->second->deleteThis();
		m_models.erase(m_models.begin());
	}

	// m_items
	while (!m_items.empty())
	{
		m_items.begin()->second->deleteThis();
		m_items.erase(m_items.begin());
	}

	// m_types
	while (!m_types.empty())
	{
		m_types.begin()->second->deleteThis();
		m_types.erase(m_types.begin());
	}

	// m_instances
	// TODO??

	if (this->IsDbOpen())
		this->CloseDb();
}

void C_Backpack::CreateDb()
{
	std::string backpackFolderName = g_pAnarchyManager->GetBackpackManager()->ExtractBackpackFoldeNameFromPath(m_backpackFolder);
	//std::string backpackSearchPath = "pack" + backpackFolderName;

	std::string libraryFile = std::string("aarcade_user/custom/") + backpackFolderName + std::string("/library.db");
	g_pAnarchyManager->GetMetaverseManager()->CreateDb(libraryFile, &m_pDb);

	bool bNeedsDefault = g_pAnarchyManager->GetMetaverseManager()->IsEmptyDb(&m_pDb);
	if (bNeedsDefault)
		g_pAnarchyManager->GetMetaverseManager()->AddDefaultTables(&m_pDb);

	// if the db was sucessfully created, it is open.  So we need to close it if we aren't really using it yet.
	//sqlite3_close(m_pDb);
	//m_pDb = null;
}

void C_Backpack::OpenDb()
{
	if (this->IsDbOpen())
		return;

	std::string backpackFolderName = g_pAnarchyManager->GetBackpackManager()->ExtractBackpackFoldeNameFromPath(m_backpackFolder);
	//std::string backpackSearchPath = "pack" + backpackFolderName;

	std::string libraryFile = std::string("aarcade_user/custom/") + backpackFolderName + std::string("/library.db");
	g_pAnarchyManager->GetMetaverseManager()->CreateDb(libraryFile, &m_pDb);
}

void C_Backpack::CloseDb()
{
	if (!this->IsDbOpen())
		return;

	DevMsg("Closed backpack DB.\n");
	sqlite3_close(m_pDb);
	m_pDb = null;
}

void C_Backpack::LegacyAuditDb()
{
	DevMsg("Auditing backpack...\n");
	// Legacy Audit will:
	// 1. Use the TEMP search path to hold all paths from this backpack.
	// 2. Basically legacy export the contents of the searchpath into its library.db
	//	2a. Types
	//	2b. Apps
	//	2c. Items
	//	2d. Models
	//	2e. Instances

	// TODO:
	//	Detected things should be INTELLEGENTLY merged, not just loaded in.

	// The legacy version of audit will search for ITM & SET files, etc.  While the non-legacy version has a much more simple job.

	// IMPORTANT NOTE: Need to scan through ALL items first, to create their types, THEN load items after that.  Just like exporting from legacy.

	// 1. Use the TEMP search path to hold all paths from this backpack.
	std::string tempSearchPath = "AUDITTEMP";
	g_pFullFileSystem->RemoveSearchPaths(tempSearchPath.c_str());

	// Now add in all the search paths we DO need
	std::string backpackFolderName = g_pAnarchyManager->GetBackpackManager()->ExtractBackpackFoldeNameFromPath(m_backpackFolder);
	g_pFullFileSystem->AddSearchPath(m_backpackFolder.c_str(), tempSearchPath.c_str(), PATH_ADD_TO_TAIL);

	if (m_vpks.empty())
		this->vpkDetect();

	unsigned int max = m_vpks.size();
	std::string vpkPath;
	for (unsigned int i = 0; i < max; i++)
	{
		vpkPath = m_backpackFolder + m_vpks[i];
		g_pFullFileSystem->AddSearchPath(vpkPath.c_str(), tempSearchPath.c_str(), PATH_ADD_TO_TAIL);
	}

	// NEXT TIME, ON BATMAN: THE ANIMATED SERIES:
	// - IMPROVE THIS AUDITING SYSTEM TO BE MUCH MORE LIKE THE LEGACY EXPORT FUNCTIONS!!
	// - DON'T FORGET TO GET ALL TYPES AND RESOLVE THEM PROPERLY, LIKE LEGACY EXPORT DOES!!
	// - .app FILES ARE NOT DISTRIBUTED ON WORKSHOP, THEY DO NOT NEED TO BE SUPPORTED WHEN DOING LEGACY AUDITS OF BACKPACKS
	// - TYPES, APPS, ITEMS, MODELS, INSTANCES, *ALL* must be properly imported. (except for apps)
	// - WHILE YOU'RE AT IT.... do the revisions to the legacy export logic that turns group names into KEYWORDS for the stuff it exports, and support complex type names.
	// - Note that they NEED to be the keywords field because it is already coded into a lot of stuff already in Redux if you search the code.  You can call it what ever on the UI menus.

	// 2. Basically legacy export the contents of the searchpath into its library.db
	//	2a. Types
	//	2b. Apps

	//	2c & 2d. Items & Models
	unsigned int itemCount = 0;
	unsigned int modelCount = 0;


	DevMsg("Scanning backpack's library folder for content...\n");

	KeyValues* pItem;
	std::string loadLocalItemLegacyFilePath;
	std::string loadLocalItemLegacyFolderPath;
	FileFindHandle_t loadLocalItemLegacyFolderHandle;
	FileFindHandle_t loadLocalItemLegacyFileHandle;
	const char *pFoldername = g_pFullFileSystem->FindFirstEx("library\\*", tempSearchPath.c_str(), &loadLocalItemLegacyFolderHandle);
	while (pFoldername != NULL)
	{
		if (!Q_strcmp(pFoldername, ".") || !Q_strcmp(pFoldername, ".."))
		{
			pFoldername = g_pFullFileSystem->FindNext(loadLocalItemLegacyFolderHandle);
			continue;
		}

		loadLocalItemLegacyFolderPath = VarArgs("library\\%s", pFoldername);
		if (!g_pFullFileSystem->FindIsDirectory(loadLocalItemLegacyFolderHandle))
		{
			DevMsg("All items files must be within a subfolder!\n");
			pFoldername = g_pFullFileSystem->FindNext(loadLocalItemLegacyFolderHandle);
			continue;
		}

		const char *pFilename = g_pFullFileSystem->FindFirstEx(VarArgs("%s\\*.itm", loadLocalItemLegacyFolderPath.c_str()), tempSearchPath.c_str(), &loadLocalItemLegacyFileHandle);
		while (pFilename != NULL)
		{
			if (g_pFullFileSystem->FindIsDirectory(loadLocalItemLegacyFileHandle))
			{
				pFilename = g_pFullFileSystem->FindNext(loadLocalItemLegacyFileHandle);
				continue;
			}

			// WE'VE FOUND A FILE TO ATTEMPT TO LOAD!!!
			std::string foundName = VarArgs("%s\\%s", loadLocalItemLegacyFolderPath.c_str(), pFilename);

			// MAKE THE FILE PATH NICE
			char path_buffer[AA_MAX_STRING];
			Q_strcpy(path_buffer, foundName.c_str());
			V_FixSlashes(path_buffer);
			foundName = path_buffer;
			// FINISHED MAKING THE FILE PATH NICE

			bool bIsModel;
			bool bWasLoaded;	// If it wasn't ALREADY loaded, then WE need to DELETE this KV when we are done with it!
			std::string workshopId = "";
			std::string mountId = "";
			//std::string filePath = backpackFolder + loadLocalItemLegacyFolderPath;
			pItem = g_pAnarchyManager->GetMetaverseManager()->LoadLocalItemLegacy(bIsModel, bWasLoaded, foundName, "", workshopId, mountId, this, tempSearchPath, false);

			// now save pItem to the BACKPACK'S library.db, so we don't have to scan for models next time.
			// TODO: That.
			if (pItem)
			{
				if (bIsModel)
				{
					DevMsg("Model detected & saved to backpack library: %s\n", g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pItem)->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID)));
					g_pAnarchyManager->GetMetaverseManager()->SaveModel(pItem, m_pDb);
					pItem->deleteThis();
					modelCount++;
				}
				else
				{
					DevMsg("Item detected & saved to backpack library: %s\n", g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pItem)->GetString("file"));
					g_pAnarchyManager->GetMetaverseManager()->SaveItem(pItem, m_pDb);
					pItem->deleteThis();
					itemCount++;
				}
			}

			pFilename = g_pFullFileSystem->FindNext(loadLocalItemLegacyFileHandle);
		}

		g_pFullFileSystem->FindClose(loadLocalItemLegacyFileHandle);
		pFoldername = g_pFullFileSystem->FindNext(loadLocalItemLegacyFolderHandle);
		//break;
	}
	g_pFullFileSystem->FindClose(loadLocalItemLegacyFolderHandle);

	// Now save the library.db out
	// do work
	DevMsg("Detected %u models\n", modelCount);
	DevMsg("Detected %u items\n", itemCount);


	//	2e. Instances
	DevMsg("Backpack fodlername: %s\n", backpackFolderName.c_str());
	//std::string folder = g_pAnarchyManager->GetBackpackManager()->GetCustomFolder() + backpackFolderName;
	//DevMsg("Full folder: %s\n", folder.c_str());
	g_pAnarchyManager->ScanForLegacySaveRecursive("", tempSearchPath, "", "", this);

	// now start loading the items from it
	//g_pAnarchyManager->GetMetaverseManager()->LoadFirstLocalItemLegacy(true, fullPath, id, "");

	// Clear the TEMP search path now that we're finished with it
	g_pFullFileSystem->RemoveSearchPaths(tempSearchPath.c_str());
}

bool C_Backpack::IsDbOpen()
{
	return (m_pDb != null);
}

void C_Backpack::MergDb()
{
	DevMsg("C_Backpack::MergDb - Quick 'n Dirty Backpack Merg Db\n");

	// TYPES
	g_pAnarchyManager->GetMetaverseManager()->LoadAllLocalTypes(m_pDb, &m_types);

	auto typesIt = m_types.begin();
	while (typesIt != m_types.end())
	{
		DevMsg("Type from DB addon is: %s\n", typesIt->first.c_str());
		if (!g_pAnarchyManager->GetMetaverseManager()->GetLibraryType(typesIt->first))
		{
			g_pAnarchyManager->GetMetaverseManager()->AddType(typesIt->second);
		}
		else
		{
			typesIt->second->deleteThis();
			typesIt->second = null;
		}
		typesIt++;
	}
	m_types.clear();

	// ITEMS
	g_pAnarchyManager->GetMetaverseManager()->LoadAllLocalItems(m_pDb, &m_items);

	auto itemsIt = m_items.begin();
	while (itemsIt != m_items.end())
	{
		DevMsg("Item from DB addon is: %s\n", itemsIt->first.c_str());
		if (!g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(itemsIt->first))
			g_pAnarchyManager->GetMetaverseManager()->AddItem(itemsIt->second);
		else
		{
			itemsIt->second->deleteThis();
			itemsIt->second = null;
		}
		itemsIt++;
	}
	m_items.clear();

	// MODELS
	unsigned int numCabinets = 0;
	g_pAnarchyManager->GetMetaverseManager()->LoadAllLocalModels(numCabinets, m_pDb, &m_models);

	auto modelsIt = m_models.begin();
	while (modelsIt != m_models.end())
	{
		DevMsg("Model from addon DB is: %s\n", modelsIt->first.c_str());
		if (!g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(modelsIt->first))
			g_pAnarchyManager->GetMetaverseManager()->AddModel(modelsIt->second);
		else
		{
			modelsIt->second->deleteThis();
			modelsIt->second = null;
		}
		modelsIt++;
	}
	m_models.clear();

	// APPS
	/* DISABLED
	g_pAnarchyManager->GetMetaverseManager()->LoadAllLocalApps(m_pDb, &m_apps);

	auto appsIt = m_apps.begin();
	while (appsIt != m_apps.end())
	{
		DevMsg("App from DB addon is: %s\n", appsIt->first.c_str());
		if (!g_pAnarchyManager->GetMetaverseManager()->GetLibraryApp(appsIt->first))
		{
			//g_pAnarchyManager->GetMetaverseManager()->AddApp(appsIt->second);
		}
		else
		{
			appsIt->second->deleteThis();
			appsIt->second = null;
		}
		appsIt++;
	}
	m_apps.clear();
	*/

	// NOTE: Instances should NOT be cleared, this way LoadInstance can ask the backpack if it contains an instance ID BEFORE it bothers to Prepare and load the instance from it!
	// DO THIS NEXT!!

	// INSTANCES
	g_pAnarchyManager->GetMetaverseManager()->LoadAllLocalInstances(m_pDb, &m_instances);

	auto instancesIt = m_instances.begin();
	while (instancesIt != m_instances.end())
	{
		DevMsg("Instance from DB addon is: %s\n", instancesIt->first.c_str());
		if (!g_pAnarchyManager->GetInstanceManager()->GetInstance(instancesIt->first))
		{
			KeyValues* pInstanceKV = instancesIt->second;

			int generation = pInstanceKV->GetInt("generation", 3);
			int iLegacy = pInstanceKV->GetInt("legacy", 0);

			KeyValues* pInstanceInfoKV = pInstanceKV->FindKey("info/local", true);
			std::string instanceId = pInstanceInfoKV->GetString("id");
			std::string mapId = pInstanceInfoKV->GetString("map");
			std::string title = pInstanceInfoKV->GetString("title");
			if (title == "")
				title = "Unnamed (" + instanceId + ")";
			std::string file = "";
			std::string workshopIds = pInstanceInfoKV->GetString(VarArgs("platforms/%s/workshopIds", AA_PLATFORM_ID));
			std::string mountIds = pInstanceInfoKV->GetString(VarArgs("platforms/%s/mountIds", AA_PLATFORM_ID));
			//std::string backpackId = pInstanceInfoKV->GetString(VarArgs("platforms/%s/backpackIds", AA_PLATFORM_ID));
			std::string style = pInstanceInfoKV->GetString("style");
			g_pAnarchyManager->GetInstanceManager()->AddInstance(iLegacy, instanceId, mapId, title, file, workshopIds, mountIds, style);

			instancesIt->second->deleteThis();
			instancesIt->second = null;
		}
		else
		{
			instancesIt->second->deleteThis();
			instancesIt->second = null;
		}
		instancesIt++;
	}
	//m_instances.clear();
}

void C_Backpack::GetAllVPKs(std::vector<std::string>& allVPKs)
{
	unsigned int max = m_vpks.size();
	for (unsigned int i = 0; i < max; i++)
		allVPKs.push_back(m_vpks[i]);
}

void C_Backpack::GetAllFilesRecursive(std::vector<std::string>& allFiles, std::string path, const char* pFilename, FileFindHandle_t& fileFindHandle)
{
	while (pFilename != NULL)
	{
		if (Q_strcmp(pFilename, ".") && Q_strcmp(pFilename, ".."))
		{
			std::string file = path + std::string(pFilename);

			if (g_pFullFileSystem->FindIsDirectory(fileFindHandle))
			{
				std::string recursivePath = file + "\\";
				FileFindHandle_t recursiveFileFindHandle;
				const char *pRecursiveFilename = g_pFullFileSystem->FindFirstEx(VarArgs("%s*", recursivePath.c_str()), "TEMP", &recursiveFileFindHandle);
				if (pRecursiveFilename)
					this->GetAllFilesRecursive(allFiles, recursivePath, pRecursiveFilename, recursiveFileFindHandle);

				g_pFullFileSystem->FindClose(recursiveFileFindHandle);
			}
			else
				allFiles.push_back(file);
		}

		pFilename = g_pFullFileSystem->FindNext(fileFindHandle);
	}
}

bool C_Backpack::HasInstance(std::string id)
{
	auto it = m_instances.find(id);
	if (it != m_instances.end())
		return true;
	else
		return false;
}

void C_Backpack::GetAllFiles(std::vector<std::string>& allFiles)
{
	// 1. Create a temp search path.
	// 2. Add the backpack folder to it.
	g_pFullFileSystem->RemoveSearchPaths("TEMP");
	g_pFullFileSystem->AddSearchPath(m_backpackFolder.c_str(), "TEMP", PATH_ADD_TO_TAIL);

	// 3. Add all VPKs to it.
	std::string vpkPath;
	unsigned int max = m_vpks.size();
	for (unsigned int i = 0; i < max; i++)
	{
		vpkPath = m_backpackFolder + m_vpks[i];
		DevMsg("Adding %s to the TEMP search path...\n", vpkPath.c_str());
		g_pFullFileSystem->AddSearchPath(vpkPath.c_str(), "TEMP", PATH_ADD_TO_TAIL);
	}

	// 4. Search through the temp search path, adding to allFiles.
	// NOTE: We cannot search the root folder, due to a bug with mounted VPKs.  Instead, we need a hard-list of files & folders to scan through.
	std::vector<std::string> rootFolders;

	// legacy stuff (mostlY)
	rootFolders.push_back("apps");
	rootFolders.push_back("cfg");
	rootFolders.push_back("download");
	rootFolders.push_back("library_cache");
	rootFolders.push_back("screenshots");
	rootFolders.push_back("scripts");
	rootFolders.push_back("searchproviders");
	rootFolders.push_back("sound");
	rootFolders.push_back("maps");
	rootFolders.push_back("models");
	rootFolders.push_back("materials");
	rootFolders.push_back("library");
	rootFolders.push_back("resource");
	rootFolders.push_back("saves");

	max = rootFolders.size();
	for (unsigned int i = 0; i < max; i++)
	{
		FileFindHandle_t fileFindHandle;
		std::string rootFolder = rootFolders[i] + "\\";
		const char *pFilename = g_pFullFileSystem->FindFirstEx(VarArgs("%s*", rootFolder.c_str()), "TEMP", &fileFindHandle);
		this->GetAllFilesRecursive(allFiles, rootFolder, pFilename, fileFindHandle);
		g_pFullFileSystem->FindClose(fileFindHandle);
	}

	std::vector<std::string> rootFiles;

	// legacy stuff
	rootFiles.push_back("info.txt");

	// additional stuff needed for redux
	rootFiles.push_back("library.db");

	max = rootFiles.size();
	for (unsigned int i = 0; i < max; i++)
	{
		FileFindHandle_t fileFindHandle;
		const char *pFilename = g_pFullFileSystem->FindFirstEx(rootFiles[i].c_str(), "TEMP", &fileFindHandle);
		this->GetAllFilesRecursive(allFiles, "", pFilename, fileFindHandle);
		g_pFullFileSystem->FindClose(fileFindHandle);
	}

	// 5. Destroy the temp search path.
	g_pFullFileSystem->RemoveSearchPaths("TEMP");
}

void C_Backpack::Activate()
{
	// Activating will:
	// 1. Mount the backpack's root folder (automatic)
	// 2. Mount all VPK's found in the backpack's root folder
	// 3. Load the library.db
	// 4. Merg it with the active library.
	// 5. Unload the library.db

	DevMsg("C_Backpack::Activate\n");

	if (m_bActive)
		DevMsg("CRITICAL ALERT: Backpack was already active!\n");

	// 1. Mount the backpack's root folder (automatic)
	//DevMsg("Adding %s to the search paths.\n", m_backpackFolder.c_str());
	//g_pFullFileSystem->AddSearchPath(m_backpackFolder.c_str(), "GAME", PATH_ADD_TO_TAIL);

	// 2. Mount all VPK's found in the backpack's root folder
	if (m_vpks.empty())
		this->vpkDetect();

	std::string vpkPath;
	unsigned int max = m_vpks.size();
	for (unsigned int i = 0; i < max; i++)
	{
		vpkPath = m_backpackFolder + m_vpks[i];

		DevMsg("Adding VPK file %s to the search paths.\n", vpkPath.c_str());
		g_pFullFileSystem->AddSearchPath(vpkPath.c_str(), "GAME", PATH_ADD_TO_TAIL);	// only game assets are allowed in VPKs, and not BSP files. VPKs are optional.  Legacy VPKs have ITM and SETs in them too.
	}

	// 3. Load the library.db
	if (!this->IsDbOpen())
		this->CreateDb();

	// 4. Merg it with the active library.
	this->MergDb();

	// 5. Unload the library.db
	this->CloseDb();
}

void C_Backpack::vpkDetect()
{
	std::string vpkFile;
	FileFindHandle_t findHandle;
	const char *pFilename = g_pFullFileSystem->FindFirstEx(VarArgs("%s*.vpk", m_backpackFolder.c_str()), "", &findHandle);
	while (pFilename != NULL)
	{
		vpkFile = m_backpackFolder + std::string(pFilename);
		m_vpks.push_back(std::string(pFilename));	// remember VPKs associated with us
		pFilename = g_pFullFileSystem->FindNext(findHandle);
	}
	g_pFullFileSystem->FindClose(findHandle);
}

void C_Backpack::Prepare()
{
	// Preparing will:
	// 1. Load the library.db.
	// 2. Standby.

	//std::string backpackFolderName = g_pAnarchyManager->GetBackpackManager()->ExtractBackpackFoldeNameFromPath(m_backpackFolder);
	//std::string backpackSearchPath = "pack" + backpackFolderName;

	// Also add us to a custom search path that ONLY gets used when we are searching through the backpack contents
	//g_pFullFileSystem->AddSearchPath(m_backpackFolder.c_str(), backpackSearchPath.c_str(), PATH_ADD_TO_TAIL);

	// vpk's are remembered in m_vpks
	//g_pFullFileSystem->AddSearchPath(vpkFile.c_str(), backpackSearchPath.c_str(), PATH_ADD_TO_TAIL);
}

void C_Backpack::Release()
{
	// Releasing will:
	// 1. Unload the library.db.
	// 2. Clear all the m_items and stuff out.	// (this is done automatically right now after merging with active library, EXCEPT for instance stubs, which must always remain for fast "hasInstance" checking.

	//if (this->IsDbOpen())
		//this->CloseDb();
	//if (this->IsPrepared())
		//this->Unprepare();
}

void C_Backpack::Init(std::string id, std::string title, std::string backpackFolder)
{
	// Initing will:
	// 1. Set basic info
	// 1. Make sure the folder exists
	// 2. Make sure a library.db exists (generating it if needed)

	// Set basic info
	if (id == "")
		m_id = g_pAnarchyManager->GenerateUniqueId();
	else
		m_id = id;

	if (title == "")
		title = "Untitled Backpack";
	else
		m_title = title;

	m_backpackFolder = backpackFolder;

	// Make sure the folder exists
	g_pFullFileSystem->CreateDirHierarchy(m_backpackFolder.c_str());

	// Make sure the library.db exists
	if (!g_pFullFileSystem->FileExists(VarArgs("%slibrary.db", m_backpackFolder.c_str())))
	{
		this->CreateDb();
		this->LegacyAuditDb();	// TODO: Legacy audit is not ALWAYS needed.
		this->CloseDb();
	}
}