#include "cbase.h"

#include "c_anarchymanager.h"
#include "aa_globals.h"
#include "c_metaversemanager.h"
#include "filesystem.h"
#include "vgui/IInput.h"
#include <algorithm>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_MetaverseManager::C_MetaverseManager()
{
	DevMsg("MetaverseManager: Constructor\n");
	m_pWebTab = null;
	m_pPreviousSearchInfo = null;
	m_pPreviousModelSearchInfo = null;
	m_pPreviousAppSearchInfo = null;
	m_pSpawningObject = null;
	m_pSpawningObjectEntity = null;
	m_iSpawningRotationAxis = 1;
	m_spawningAngles.x = 0;
	m_spawningAngles.y = 0;
	m_spawningAngles.z = 0;

	m_bHasDisconnected = false;

	m_bHostSessionNow = false;
	m_pLocalUser = null;

	m_fPresenceLastSynced = 0;

	m_pImportSteamGamesKV = null;
	m_pImportSteamGamesSubKV = null;

	m_libraryBrowserContextCategory = "items";
	m_libraryBrowserContextId = "";
	m_libraryBrowserContextSearch = "";
	m_libraryBrowserContextFilter = "";
	
	m_pPreviousLoadLocalItemLegacyBackpack = null;

	m_defaultFields.push_back("title");
	m_defaultFields.push_back("description");
	m_defaultFields.push_back("file");
	m_defaultFields.push_back("type");
	m_defaultFields.push_back("app");
	m_defaultFields.push_back("reference");
	m_defaultFields.push_back("preview");
	m_defaultFields.push_back("download");
	m_defaultFields.push_back("stream");
	m_defaultFields.push_back("screen");
	m_defaultFields.push_back("marquee");
	m_defaultFields.push_back("model");

	//m_uProcessBatchSize = 1;
	//m_uProcessCurrentCycle = 0;

	m_uNumSteamGamesToImport = 0;
	m_uNumSteamGamesToImported = 0;
	
	m_db = null;
}

C_MetaverseManager::~C_MetaverseManager()
{
	DevMsg("MetaverseManager: Destructor\n");

	m_mapScreenshots.clear();

	if (m_previousLoadLocalAppFilePath != "")
	{
		g_pFullFileSystem->FindClose(m_previousLoadLocalAppFileHandle);
		m_previousLoadLocalAppFilePath = "";
	}

	if (m_pPreviousSearchInfo)
		m_pPreviousSearchInfo->deleteThis();

	if (m_pPreviousModelSearchInfo)
		m_pPreviousModelSearchInfo->deleteThis();

	if (m_pPreviousAppSearchInfo)
		m_pPreviousAppSearchInfo->deleteThis();

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

	// screenshots
	while (!m_mapScreenshots.empty())
	{
		m_mapScreenshots.begin()->second->deleteThis();
		m_mapScreenshots.erase(m_mapScreenshots.begin());
	}

	if (m_pImportSteamGamesKV)
	{
		m_pImportSteamGamesKV->deleteThis();
		m_pImportSteamGamesKV = null;
		m_pImportSteamGamesSubKV = null;
	}

	sqlite3_close(m_db);
	// TODO: error objects for sqlite must also be cleaned up
}

bool C_MetaverseManager::CreateDb(std::string libraryFile, sqlite3** pDb)
{
	// create or open the library.db
	DevMsg("Opening (or creating) SQL DB at: %s\n", libraryFile.c_str());

	sqlite3* db;
	int rc = sqlite3_open(libraryFile.c_str(), &db);
	if (!db)
	{
		DevMsg("Critical error opening the specified SQLite3 database!\n");
		return false;
	}

	*pDb = db;
	return true;
}

bool C_MetaverseManager::IsEmptyDb(sqlite3** pDb)
{
	if (!pDb)
		pDb = &m_db;

	// no maps means empty library
	// TODO: Improve this check
	// confirm that default stuff exists
	bool bNeedsDefault = true;

	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(*pDb, "SELECT * from  types", -1, &stmt, NULL);
	if (rc == SQLITE_OK)
	{
		bNeedsDefault = false;
		/*
		int length;
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			length = sqlite3_column_bytes(stmt, 1);

			if (length == 0)
			{
				DevMsg("WARNING: Zero-byte KeyValues skipped.\n");
				continue;
			}

			//KeyValues* pMap = new KeyValues("map");

			//CUtlBuffer buf(0, length, 0);
			//buf.CopyBuffer(sqlite3_column_blob(stmt, 1), length);
			//pMap->ReadAsBinary(buf);

			// TODO: Look up any alias here first!! (or maybe later.  later is probably OK.  later is probably better. way later.  later during library functions.)
			//pMap = this->GetActiveKeyValues(pMap);
			//if (pMap)
			//	bNeedsDefault = false;

			break;
		}
		*/
		sqlite3_finalize(stmt);
	}

	return bNeedsDefault;
}

void C_MetaverseManager::AddDefaultTables(sqlite3** pDb)
{
	if (!pDb)
		pDb = &m_db;

	// create the tables
	char *error;
	const char *sqlCreateAppsTable = "CREATE TABLE apps (id STRING PRIMARY KEY, value BLOB);";
	int rc = sqlite3_exec(*pDb, sqlCreateAppsTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(*pDb));
		sqlite3_free(error);
	}

	const char *sqlCreateItemsTable = "CREATE TABLE items (id STRING PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(*pDb, sqlCreateItemsTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(*pDb));
		sqlite3_free(error);
	}

	const char *sqlCreateMapsTable = "CREATE TABLE maps (id STRING PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(*pDb, sqlCreateMapsTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(*pDb));
		sqlite3_free(error);
	}

	const char *sqlCreateModelsTable = "CREATE TABLE models (id STRING PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(*pDb, sqlCreateModelsTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(*pDb));
		sqlite3_free(error);
	}

	const char *sqlCreatePlatformsTable = "CREATE TABLE platforms (id STRING PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(*pDb, sqlCreatePlatformsTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(*pDb));
		sqlite3_free(error);
	}

	const char *sqlCreateTypesTable = "CREATE TABLE types (id STRING PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(*pDb, sqlCreateTypesTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(*pDb));
		sqlite3_free(error);
	}

	const char *sqlCreateInstancesTable = "CREATE TABLE instances (id STRING PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(*pDb, sqlCreateInstancesTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(*pDb));
		sqlite3_free(error);
	}

	const char *sqlCreateVersionTable = "CREATE TABLE version (id INTEGER PRIMARY KEY, value INTEGER);";
	rc = sqlite3_exec(*pDb, sqlCreateVersionTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(*pDb));
		sqlite3_free(error);
	}

	// now save our AA_LIBRARY_VERSION number to the DB for future proofing
	sqlite3_stmt *stmt = NULL;
	rc = sqlite3_prepare(*pDb, VarArgs("INSERT INTO version (id, value) VALUES(0, %i)", AA_LIBRARY_VERSION), -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: prepare failed: %s\n", sqlite3_errmsg(*pDb));

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
		DevMsg("FATAL ERROR: execution failed: %s\n", sqlite3_errmsg(*pDb));

	sqlite3_finalize(stmt);
}

addDefaultLibraryContext_t* C_MetaverseManager::GetAddDefaultLibraryContext()
{
	auto it = m_addDefaultLibraryContexts.begin();
	if( it != m_addDefaultLibraryContexts.end() )
		return it->first;
	
	return null;
}

void C_MetaverseManager::SetAddDefaultLibraryToDbIterativeContext(addDefaultLibraryContext_t* pContext)
{
	auto it = m_addDefaultLibraryContexts.find(pContext);
	if (it == m_addDefaultLibraryContexts.end())
		m_addDefaultLibraryContexts[pContext] = true;
}

bool C_MetaverseManager::DeleteAddDefaultLibraryContext(addDefaultLibraryContext_t* pContext)
{
	auto it = m_addDefaultLibraryContexts.find(pContext);
	if (it != m_addDefaultLibraryContexts.end())
	{
		m_addDefaultLibraryContexts.erase(it);
		delete pContext;
		return true;
	}

	return false;
}

/* STATES
	0 - uninitialized
	1 - finished
	2 - adding apps
	3 - adding cabinets
	4 - adding maps
	5 - adding models
	6 - adding types
*/
void C_MetaverseManager::AddDefaultLibraryToDbIterative(addDefaultLibraryContext_t* pContext)
{
	sqlite3** pDb = (pContext->pDb) ? pContext->pDb : &m_db;

	// get ready for apps
	if (pContext->state == 0)
	{
		pContext->pFilename = g_pFullFileSystem->FindFirstEx("defaultLibrary\\apps\\*.txt", "MOD", &pContext->handle);
		pContext->state = 2;
	}

	if (pContext->state == 2)
	{
		// 2 APPS
		while (pContext->pFilename != NULL && g_pFullFileSystem->FindIsDirectory(pContext->handle))
			pContext->pFilename = g_pFullFileSystem->FindNext(pContext->handle);

		if (pContext->pFilename != NULL)
		{
			pContext->kv = new KeyValues("app");
			if (pContext->kv->LoadFromFile(g_pFullFileSystem, VarArgs("defaultLibrary\\apps\\%s", pContext->pFilename), "MOD"))
			{
				KeyValues* active = this->GetActiveKeyValues(pContext->kv);
				this->SaveSQL(pDb, "apps", active->GetString("info/id"), pContext->kv);
			}
			pContext->numApps++;
			pContext->kv->deleteThis();
			pContext->pFilename = g_pFullFileSystem->FindNext(pContext->handle);
		}
		else
		{
			g_pFullFileSystem->FindClose(pContext->handle);

			// get ready for cabinets
			pContext->pFilename = g_pFullFileSystem->FindFirstEx("defaultLibrary\\cabinets\\*.txt", "MOD", &pContext->handle);
			pContext->state = 3;
		}
	}

	if (pContext->state == 3)
	{
		// 3 CABINETS
		while (pContext->pFilename != NULL && g_pFullFileSystem->FindIsDirectory(pContext->handle))
			pContext->pFilename = g_pFullFileSystem->FindNext(pContext->handle);

		if (pContext->pFilename != NULL)
		{
			pContext->kv = new KeyValues("model");
			if (pContext->kv->LoadFromFile(g_pFullFileSystem, VarArgs("defaultLibrary\\cabinets\\%s", pContext->pFilename), "MOD"))
			{
				KeyValues* active = this->GetActiveKeyValues(pContext->kv);

				//std::string id = active->GetString("info/id");
				//if (id.find_first_of(".") != std::string::npos)
					//id = g_pAnarchyManager->GenerateUniqueId();

				//DevMsg("Saving cabinet w/ title %s and id %s\n", active->GetString("title"), active->GetString("info/id"));
				// generate an ID on the file name instead of trying to load the one saved in the txt file cuz of how shiddy KV's are at loading from txt files.
				std::string id = g_pAnarchyManager->GenerateLegacyHash(active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID)));
				active->SetString("info/id", id.c_str());
				this->SaveSQL(pDb, "models", id.c_str(), pContext->kv);
			}
			pContext->numCabinets++;
			pContext->kv->deleteThis();
			pContext->pFilename = g_pFullFileSystem->FindNext(pContext->handle);
		}
		else
		{
			g_pFullFileSystem->FindClose(pContext->handle);

			// get ready for maps
			pContext->pFilename = g_pFullFileSystem->FindFirstEx("defaultLibrary\\maps\\*.txt", "MOD", &pContext->handle);
			pContext->state = 4;
		}
	}

	if (pContext->state == 4)
	{
		// 4 MAPS
		while (pContext->pFilename != NULL && g_pFullFileSystem->FindIsDirectory(pContext->handle))
			pContext->pFilename = g_pFullFileSystem->FindNext(pContext->handle);

		if (pContext->pFilename != NULL)
		{
			pContext->kv = new KeyValues("map");
			if (pContext->kv->LoadFromFile(g_pFullFileSystem, VarArgs("defaultLibrary\\maps\\%s", pContext->pFilename), "MOD"))
			{
				KeyValues* active = this->GetActiveKeyValues(pContext->kv);
				this->SaveSQL(pDb, "maps", active->GetString("info/id"), pContext->kv);
			}
			pContext->numMaps++;
			pContext->kv->deleteThis();
			pContext->pFilename = g_pFullFileSystem->FindNext(pContext->handle);
		}
		else
		{
			g_pFullFileSystem->FindClose(pContext->handle);

			// get ready for models
			pContext->pFilename = g_pFullFileSystem->FindFirstEx("defaultLibrary\\models\\*.txt", "MOD", &pContext->handle);
			pContext->state = 5;
		}
	}

	if (pContext->state == 5)
	{
		// 5 MODELS
		while (pContext->pFilename != NULL && g_pFullFileSystem->FindIsDirectory(pContext->handle))
			pContext->pFilename = g_pFullFileSystem->FindNext(pContext->handle);

		if (pContext->pFilename != NULL)
		{
			pContext->kv = new KeyValues("model");
			if (pContext->kv->LoadFromFile(g_pFullFileSystem, VarArgs("defaultLibrary\\models\\%s", pContext->pFilename), "MOD"))
			{
				KeyValues* active = this->GetActiveKeyValues(pContext->kv);
				this->SaveSQL(pDb, "models", active->GetString("info/id"), pContext->kv);
			}
			pContext->numModels++;
			pContext->kv->deleteThis();
			pContext->pFilename = g_pFullFileSystem->FindNext(pContext->handle);
		}
		else
		{
			g_pFullFileSystem->FindClose(pContext->handle);

			// get ready for types
			pContext->pFilename = g_pFullFileSystem->FindFirstEx("defaultLibrary\\types\\*.txt", "MOD", &pContext->handle);
			pContext->state = 6;
		}
	}

	if (pContext->state == 6)
	{
		// 6 TYPES
		while (pContext->pFilename != NULL && g_pFullFileSystem->FindIsDirectory(pContext->handle))
			pContext->pFilename = g_pFullFileSystem->FindNext(pContext->handle);

		if (pContext->pFilename != NULL)
		{
			pContext->kv = new KeyValues("type");
			if (pContext->kv->LoadFromFile(g_pFullFileSystem, VarArgs("defaultLibrary\\types\\%s", pContext->pFilename), "MOD"))
			{
				KeyValues* active = this->GetActiveKeyValues(pContext->kv);
				this->SaveSQL(pDb, "types", active->GetString("info/id"), pContext->kv);
			}
			pContext->numTypes++;
			pContext->kv->deleteThis();
			pContext->pFilename = g_pFullFileSystem->FindNext(pContext->handle);
		}
		else
		{
			g_pFullFileSystem->FindClose(pContext->handle);

			// finished
			pContext->state = 1;
		}
	}
}

void C_MetaverseManager::AddDefaultLibraryToDb(unsigned int& numApps, unsigned int& numCabinets, unsigned int& numModels, unsigned int& numTypes, sqlite3** pDb)
{
	// OBSOLETE!!
	if (!pDb)
		pDb = &m_db;

	// NOW LOAD IN ALL DEFAULTLIBRARY KEYVALUES & ADD THEM TO THE LIBRARY (and copy scraper.js files into the right folder)
	FileFindHandle_t handle;
	const char *pFilename;
	KeyValues* kv;

	// APPS
	pFilename = g_pFullFileSystem->FindFirstEx("defaultLibrary\\apps\\*.txt", "MOD", &handle);
	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(handle))
		{
			pFilename = g_pFullFileSystem->FindNext(handle);
			continue;
		}

		kv = new KeyValues("app");
		if (kv->LoadFromFile(g_pFullFileSystem, VarArgs("defaultLibrary\\apps\\%s", pFilename), "MOD"))
		{
			KeyValues* active = this->GetActiveKeyValues(kv);
			this->SaveSQL(pDb, "apps", active->GetString("info/id"), kv);
			numApps++;
		}
		kv->deleteThis();

		pFilename = g_pFullFileSystem->FindNext(handle);
	}
	g_pFullFileSystem->FindClose(handle);

	// CABINETS
	pFilename = g_pFullFileSystem->FindFirstEx("defaultLibrary\\cabinets\\*.txt", "MOD", &handle);
	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(handle))
		{
			pFilename = g_pFullFileSystem->FindNext(handle);
			continue;
		}

		kv = new KeyValues("model");
		if (kv->LoadFromFile(g_pFullFileSystem, VarArgs("defaultLibrary\\cabinets\\%s", pFilename), "MOD"))
		{
			KeyValues* active = this->GetActiveKeyValues(kv);
			DevMsg("Saving cabinet w/ title %s and id %s\n", active->GetString("title"), active->GetString("info/id"));
			this->SaveSQL(pDb, "models", active->GetString("info/id"), kv);
			numCabinets++;
		}
		kv->deleteThis();

		pFilename = g_pFullFileSystem->FindNext(handle);
	}
	g_pFullFileSystem->FindClose(handle);

	// MAPS
	pFilename = g_pFullFileSystem->FindFirstEx("defaultLibrary\\maps\\*.txt", "MOD", &handle);
	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(handle))
		{
			pFilename = g_pFullFileSystem->FindNext(handle);
			continue;
		}

		kv = new KeyValues("map");
		if (kv->LoadFromFile(g_pFullFileSystem, VarArgs("defaultLibrary\\maps\\%s", pFilename), "MOD"))
		{
			KeyValues* active = this->GetActiveKeyValues(kv);
			this->SaveSQL(pDb, "maps", active->GetString("info/id"), kv);
		}
		kv->deleteThis();

		pFilename = g_pFullFileSystem->FindNext(handle);
	}
	g_pFullFileSystem->FindClose(handle);

	// MODELS
	pFilename = g_pFullFileSystem->FindFirstEx("defaultLibrary\\models\\*.txt", "MOD", &handle);
	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(handle))
		{
			pFilename = g_pFullFileSystem->FindNext(handle);
			continue;
		}

		kv = new KeyValues("model");
		if (kv->LoadFromFile(g_pFullFileSystem, VarArgs("defaultLibrary\\models\\%s", pFilename), "MOD"))
		{
			KeyValues* active = this->GetActiveKeyValues(kv);
			this->SaveSQL(pDb, "models", active->GetString("info/id"), kv);
			numModels++;
		}
		kv->deleteThis();

		pFilename = g_pFullFileSystem->FindNext(handle);
	}
	g_pFullFileSystem->FindClose(handle);

	// TYPES
	pFilename = g_pFullFileSystem->FindFirstEx("defaultLibrary\\types\\*.txt", "MOD", &handle);
	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(handle))
		{
			pFilename = g_pFullFileSystem->FindNext(handle);
			continue;
		}

		kv = new KeyValues("type");
		if (kv->LoadFromFile(g_pFullFileSystem, VarArgs("defaultLibrary\\types\\%s", pFilename), "MOD"))
		{
			KeyValues* active = this->GetActiveKeyValues(kv);
			this->SaveSQL(pDb, "types", active->GetString("info/id"), kv);
			numTypes++;
		}
		kv->deleteThis();

		pFilename = g_pFullFileSystem->FindNext(handle);
	}
	g_pFullFileSystem->FindClose(handle);

	// SCRAPERS
	// TODO: work
}

void C_MetaverseManager::Init()
{
	this->CreateDb("aarcade_user/library.db", &m_db);
}

int C_MetaverseManager::ExtractLibraryVersion(sqlite3** pDb)
{
	if (!pDb)
		pDb = &m_db;

	int value = -1;

	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(*pDb, "SELECT * from version WHERE id = 0", -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed: %s\n", sqlite3_errmsg(*pDb));
	else if (sqlite3_step(stmt) == SQLITE_ROW)
		value = sqlite3_column_int(stmt, 1);

	sqlite3_finalize(stmt);
	return value;
}

/*
void C_MetaverseManager::OnWebTabCreated(C_WebTab* pWebTab)
{
	m_pWebTab = pWebTab;
}
*/

unsigned int C_MetaverseManager::GetLibraryDbSize()
{
	return g_pFullFileSystem->Size("library.db", "DEFAULT_WRITE_PATH");
}

bool C_MetaverseManager::CreateLibraryBackup()
{
	g_pAnarchyManager->AddToastMessage("Libary Backup Created");

	// make sure the backups folder exists
	g_pFullFileSystem->CreateDirHierarchy("backups", "DEFAULT_WRITE_PATH");

	// find a suitable filename
	unsigned int backupNumber = 0;
	std::string backupFile = "backups/library-auto" + std::string(VarArgs("%u", backupNumber)) + ".db";

	while (g_pFullFileSystem->FileExists(backupFile.c_str(), "DEFAULT_WRITE_PATH"))
	{
		backupNumber++;
		backupFile = "backups/library-auto" + std::string(VarArgs("%u", backupNumber)) + ".db";
	}

	// copy the library.db file to it
	CUtlBuffer buf;
	if (g_pFullFileSystem->ReadFile("library.db", "DEFAULT_WRITE_PATH", buf))
		g_pFullFileSystem->WriteFile(backupFile.c_str(), "DEFAULT_WRITE_PATH", buf);
	buf.Purge();

	return true;
}

bool C_MetaverseManager::ConvertLibraryVersion(unsigned int uOld, unsigned int uTarget)
{
	// 1. Make a backup of library.db to aarcade_user/backups/library-auto0.db
	// 2. The library is already loaded, so it just needs to be converted right now.
	// 3. The oldest conversion needed right now is from version -1/0 to version 1.

	// always create a backup of library.db

	this->CreateLibraryBackup();

	// now determine which conversion to do
	if (uOld == 0 && uTarget == 1)
	{
		// VERSION 0 TO VERSION 1:
		/*
		- Instances need to re-structure their info node to be info/local/FIELD
		- The platforms key is now stored under the info/local key as well, it is no longer a sibling to generation.
		- Instances converted from Version 0 to Version 1 are no longer considered or tagged as Legacy because Version 0 stuff tried to resolve item & model ID's upon addon consumption rather than on instance load.  Their info needs to be restructured still, but leave their resolved item & model ID's alone.
		- Library conversion should be done IMMEDIATELY upon any outdated library.db that gets loaded.
		- library.db files NEED to have their Library Version saved to their header, or be assumed Version 0.
		- Library Version should NOT change EVER, but ESPECIALLY after Redux is out of beta.
		- Conversion between Library Versions should be ABNORMAL behavior and only really needed by beta testers who don't want to lose their saves.
		*/

		// Alright, a backup has been made.  Time to start converting.
		std::vector<std::string> badInstanceIds;
		std::vector<std::string> instanceIds;
		sqlite3* pDb = m_db;
		//unsigned int count = 0;

		sqlite3_stmt *stmtSelAll = NULL;
		int rc = sqlite3_prepare(pDb, "SELECT * from instances", -1, &stmtSelAll, NULL);
		if (rc != SQLITE_OK)
			DevMsg("prepare failed: %s\n", sqlite3_errmsg(pDb));

		while (sqlite3_step(stmtSelAll) == SQLITE_ROW)
			instanceIds.push_back(std::string((const char*)sqlite3_column_text(stmtSelAll, 0)));
		sqlite3_finalize(stmtSelAll);

		for (unsigned int i = 0; i < instanceIds.size(); i++)
		{
			sqlite3_stmt *stmtSel = NULL;
			int rc = sqlite3_prepare(pDb, VarArgs("SELECT * from instances WHERE id = \"%s\"", instanceIds[i].c_str()), -1, &stmtSel, NULL);
			if (rc != SQLITE_OK)
			{
				DevMsg("prepare failed: %s\n", sqlite3_errmsg(pDb));
				sqlite3_finalize(stmtSel);
				continue;
			}

			if (sqlite3_step(stmtSel) != SQLITE_ROW)
			{
				DevMsg("warning: did now find a row. skipping.\n");
				sqlite3_finalize(stmtSel);
				continue;
			}

			std::string rowId = std::string((const char*)sqlite3_column_text(stmtSel, 0));
			//		DevMsg("Row ID is: %s\n", rowId.c_str());
			if (rowId == "")
			{
				sqlite3_finalize(stmtSel);
				continue;
			}

			int length = sqlite3_column_bytes(stmtSel, 1);
			if (length == 0)
			{
				DevMsg("WARNING: Zero-byte KeyValues detected as bad.\n");
				badInstanceIds.push_back(rowId);
				sqlite3_finalize(stmtSel);
				continue;
			}

			KeyValues* pInstance = new KeyValues("instance");

			CUtlBuffer buf(0, length, 0);
			buf.CopyBuffer(sqlite3_column_blob(stmtSel, 1), length);
			pInstance->ReadAsBinary(buf);

			// done with the select statement now
			sqlite3_finalize(stmtSel);
			buf.Purge();

			// instance is loaded & ready to convert
			KeyValues* oldInfoKV = pInstance->FindKey("info");
			if (!oldInfoKV)
			{
				// bogus save detected, clear it out... later
				badInstanceIds.push_back(rowId);
				pInstance->deleteThis();
				continue;
			}

			//KeyValues* infoKV = pInstance->FindKey("info/local", true);

			// copy subkeys from oldInfo to info
			for (KeyValues *sub = oldInfoKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			{
				if (sub->GetFirstSubKey())
					continue;	// don't copy some garbage values that show up.

				pInstance->SetString(VarArgs("info/local/%s", sub->GetName()), sub->GetString());
			}

			/*
			generation
			info (parent)
			title
			map
			style
			creator
			id
			platforms (parent) (sometimes)
			objects (parent) (sometimes)
			*/

			DevMsg("Converted instance w/ id %s\n", rowId.c_str());// and info : \n", rowId.c_str());
			//for (KeyValues *sub = pInstance->FindKey("info/local")->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			//	DevMsg("\t%s: %s\n", sub->GetName(), sub->GetString());

			//// Remove the platforms tab cuz it shouldn't be there.
			// Copy workshopId and mountId from the platform keys, if needed
			KeyValues* platformsKV = pInstance->FindKey(VarArgs("platforms/%s", AA_PLATFORM_ID));
			if (platformsKV)
			{
				for (KeyValues *sub = platformsKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
				{
					pInstance->SetString(VarArgs("info/local/platforms/%s/%s", AA_PLATFORM_ID, sub->GetName()), sub->GetString());
				}
			}

			std::vector<KeyValues*> targetSubKeys;
			// remove everything in the info key besides "local"
			for (KeyValues *sub = pInstance->FindKey("info")->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			{
				if (Q_strcmp(sub->GetName(), "local"))
					targetSubKeys.push_back(sub);
			}

			KeyValues* pOldInfoKV = pInstance->FindKey("info");
			for (unsigned int j = 0; j < targetSubKeys.size(); j++)
			{
				pOldInfoKV->RemoveSubKey(targetSubKeys[j]);
				//pInstance->SetString("info", "");
			}
			targetSubKeys.clear();

			// then remove the old platforms key
			if (pInstance->FindKey("platforms"))
			{
				pInstance->RemoveSubKey(pInstance->FindKey("platforms"));
				//pInstance->SetString("platforms", "");
				platformsKV = null;
			}

			// done removing stuff.

			//DevMsg("And now...\n");
			//for (KeyValues *sub = pInstance->FindKey("info/local")->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			//	DevMsg("\t%s: %s\n", sub->GetName(), sub->GetString());

			// Now save the instance back out to the library.db
			sqlite3_stmt *stmtInst = NULL;
			rc = sqlite3_prepare(pDb, VarArgs("REPLACE INTO instances VALUES(\"%s\", ?)", rowId.c_str()), -1, &stmtInst, NULL);
			if (rc != SQLITE_OK)
			{
				DevMsg("FATAL ERROR: prepare failed: %s\n", sqlite3_errmsg(pDb));
				sqlite3_finalize(stmtInst);
			}
			else
			{
				// SQLITE_STATIC because the statement is finalized before the buffer is freed:
				CUtlBuffer instBuf;
				pInstance->WriteAsBinary(instBuf);

				int size = instBuf.Size();
				rc = sqlite3_bind_blob(stmtInst, 1, instBuf.Base(), size, SQLITE_STATIC);
				if (rc != SQLITE_OK)
					DevMsg("FATAL ERROR: bind failed: %s\n", sqlite3_errmsg(pDb));
				else
				{
					rc = sqlite3_step(stmtInst);
					if (rc != SQLITE_DONE)
					{
						if (rc == SQLITE_ERROR)
							DevMsg("FATAL ERROR: execution failed: %s\n", sqlite3_errmsg(pDb));
						else
							DevMsg("Weird other error!!\n");
					}
				}
				sqlite3_finalize(stmtInst);
				instBuf.Purge();
			}
			pInstance->deleteThis();
		}
		instanceIds.clear();

		// remove bad instances
		DevMsg("Removing %u bogus instances from old library.\n", badInstanceIds.size());
		for (unsigned int i = 0; i < badInstanceIds.size(); i++)
		{
			/*
			DELETE
			FROM
			artists_backup
			WHERE
			artistid = 1;
			*/

			sqlite3_stmt *stmtDel = NULL;
			rc = sqlite3_prepare(pDb, VarArgs("DELETE FROM instances WHERE id = \"%s\"", badInstanceIds[i].c_str()), -1, &stmtDel, NULL);
			if (rc != SQLITE_OK)
				DevMsg("FATAL ERROR: prepare failed: %s\n", sqlite3_errmsg(pDb));

			rc = sqlite3_step(stmtDel);
			if (rc != SQLITE_DONE)
				DevMsg("FATAL ERROR: execution failed: %s\n", sqlite3_errmsg(pDb));

			sqlite3_finalize(stmtDel);
		}
		badInstanceIds.clear();

		//DevMsg("ERROR: Could not find old info key!  Contents include:\n");
		//for (KeyValues *sub = pInstance->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		//	DevMsg("\t%s: %s\n", sub->GetName(), sub->GetString());

		// Now add the version table and set it to 1
		// now save our AA_LIBRARY_VERSION number to the DB for future proofing
		char *error;
		const char *sqlCreateVersionTable = "CREATE TABLE version (id INTEGER PRIMARY KEY, value INTEGER);";
		rc = sqlite3_exec(pDb, sqlCreateVersionTable, NULL, NULL, &error);
		if (rc != SQLITE_OK)
		{
			DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(pDb));
			sqlite3_free(error);
		}

		sqlite3_stmt *stmt3 = NULL;
		rc = sqlite3_prepare(pDb, "INSERT INTO version (id, value) VALUES(0, 1)", -1, &stmt3, NULL);
		if (rc != SQLITE_OK)
			DevMsg("FATAL ERROR: prepare failed: %s\n", sqlite3_errmsg(pDb));

		rc = sqlite3_step(stmt3);
		if (rc != SQLITE_DONE)
			DevMsg("FATAL ERROR: execution failed: %s\n", sqlite3_errmsg(pDb));

		sqlite3_finalize(stmt3);

		return true;
	}
	//else if (uOld == 1 && uTarget == 2)
	//{
		// VERSION 1 TO VERSION 2:
		/*
		- There is now a default NODE type
		- The platforms key is now stored under the info/local key as well, it is no longer a sibling to generation.
		- Instances converted from Version 0 to Version 1 are no longer considered or tagged as Legacy because Version 0 stuff tried to resolve item & model ID's upon addon consumption rather than on instance load.  Their info needs to be restructured still, but leave their resolved item & model ID's alone.
		- Library conversion should be done IMMEDIATELY upon any outdated library.db that gets loaded.
		- library.db files NEED to have their Library Version saved to their header, or be assumed Version 0.
		- Library Version should NOT change EVER, but ESPECIALLY after Redux is out of beta.
		- Conversion between Library Versions should be ABNORMAL behavior and only really needed by beta testers who don't want to lose their saves.
		*/
	//	return true;
	//}
	else
		DevMsg("ERROR: Unknown library conversion values!\n");

	return false;
}

void C_MetaverseManager::Update()
{
	if (m_bHasDisconnected && !g_pAnarchyManager->IsPaused() && !engine->IsPaused())
	{
		this->RestartNetwork();
		return;
	}

	// handle any pending user updates
	auto it = m_pendingUserUpdates.begin();
	if (it != m_pendingUserUpdates.end())
	{
		if (this->ProcessUserSessionUpdate(it->second))
			m_pendingUserUpdates.erase(it);
	}

	if (m_bHostSessionNow)
		this->HostSessionNow();

	// thats all we do if we're paused.
	if (g_pAnarchyManager->IsPaused() || engine->IsPaused() )
		return;

	// FIXME: all this spawning object stuff should be done in instance manager, not here....
	if (m_pSpawningObjectEntity)
	{
		// Figure out where to place it
		C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
		Vector forward;
		pPlayer->EyeVectors(&forward);

		trace_t tr;
		UTIL_TraceLine(pPlayer->EyePosition(),
			pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH, MASK_NPCSOLID,
			pPlayer, COLLISION_GROUP_NONE, &tr);

		// No hit? We're done.
		if (tr.fraction == 1.0)
			return;

		VMatrix entToWorld;
		Vector xaxis;
		Vector yaxis;

		if (tr.plane.normal.z == 0.0f)
		{
			yaxis = Vector(0.0f, 0.0f, 1.0f);
			CrossProduct(yaxis, tr.plane.normal, xaxis);
			entToWorld.SetBasisVectors(tr.plane.normal, xaxis, yaxis);
		}
		else
		{
			Vector ItemToPlayer;
			VectorSubtract(pPlayer->GetAbsOrigin(), Vector(tr.endpos.x, tr.endpos.y, tr.endpos.z), ItemToPlayer);

			xaxis = Vector(ItemToPlayer.x, ItemToPlayer.y, ItemToPlayer.z);

			CrossProduct(tr.plane.normal, xaxis, yaxis);
			if (VectorNormalize(yaxis) < 1e-3)
			{
				xaxis.Init(0.0f, 0.0f, 1.0f);
				CrossProduct(tr.plane.normal, xaxis, yaxis);
				VectorNormalize(yaxis);
			}
			CrossProduct(yaxis, tr.plane.normal, xaxis);
			VectorNormalize(xaxis);

			entToWorld.SetBasisVectors(xaxis, yaxis, tr.plane.normal);
		}

		QAngle angles;
		MatrixToAngles(entToWorld, angles);

//		UTIL_SetOrigin(m_pSpawningObjectEntity, tr.endpos);
//		m_pSpawningObjectEntity->SetAbsAngles(angles);

		// WORKING!! Rotate based on how much player has turned since last frame
		/* disabled for now cuz of transform menu
		if (g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity())
		{
			if (vgui::input()->IsKeyDown(KEY_E))
			{
				int x, y;
				vgui::input()->GetCursorPos(x, y);

				switch (m_iSpawningRotationAxis)
				{
					case 0:
						m_spawningAngles.x += x - ScreenWidth() / 2;
						break;

					case 1:
						m_spawningAngles.y += x - ScreenWidth() / 2;
						break;

					case 2:
						m_spawningAngles.z += x - ScreenWidth() / 2;
						break;
				}
			}

			angles.x += m_spawningAngles.x;
			angles.y += m_spawningAngles.y;
			angles.z += m_spawningAngles.z;
		}
		*/

		// add in the current transform
		transform_t* pTransform = g_pAnarchyManager->GetInstanceManager()->GetTransform();
		tr.endpos.x += pTransform->offX;
		tr.endpos.y += pTransform->offY;
		tr.endpos.z += pTransform->offZ;

		angles.x += pTransform->rotP;
		angles.y += pTransform->rotY;
		angles.z += pTransform->rotR;

		// FIXME: that stuff should be done server-side so collision boxes are updated properly...
		engine->ServerCmd(VarArgs("setcabpos %i %f %f %f %f %f %f \"%s\";\n", m_pSpawningObjectEntity->entindex(), tr.endpos.x, tr.endpos.y, tr.endpos.z, angles.x, angles.y, angles.z, ""), false);

		float currentScale = m_pSpawningObjectEntity->GetModelScale();
		if (abs(pTransform->scale - currentScale) > 0.04)
			g_pAnarchyManager->GetMetaverseManager()->SetObjectScale(g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity(), pTransform->scale);
	}

	//C_AwesomiumBrowserInstance* pNetworkBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetNetworkAwesomiumBrowserInstance();
	if (g_pAnarchyManager->GetConnectedUniverse() && g_pAnarchyManager->GetConnectedUniverse()->connected) // && pNetworkBrowserInstance
	{
		//uint64 currentTime = g_pAnarchyManager->GetTimeNumber();
		//uint64 presenceSyncInterval = 300;

		float fCurrentTime = engine->Time();
		float fPresenceSyncInterval = 0.5;

		if (m_fPresenceLastSynced == 0 || fCurrentTime - m_fPresenceLastSynced >= fPresenceSyncInterval)
		{
			m_fPresenceLastSynced = fCurrentTime;
			//g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("network");
			this->PerformLocalPlayerUpdate();
		}
	}

	if (m_avatarDeathList.size() > 0)
	{
		C_DynamicProp* pVictimProp = m_avatarDeathList.back();
		m_avatarDeathList.pop_back();

		engine->ServerCmd(VarArgs("removeobject %i;\n", pVictimProp->entindex()), false);
	}
}

void C_MetaverseManager::PerformLocalPlayerUpdate()
{
	C_AwesomiumBrowserInstance* pNetworkBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetNetworkAwesomiumBrowserInstance();
	if (!pNetworkBrowserInstance)
		return;

	/*
	0 - instance
	1 - say
	2 - bodyOrigin
	3 - bodyAngles
	4 - headOrigin
	5 - headAngles
	6 - item
	7 - object
	8 - mouseX
	9 - mouseY
	10 - webURL
	*/

	//user_t* pUser = this->GetInstanceUser(cvar->FindVar("aamp_client_id"));

	if (!m_pLocalUser)
		return;

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	QAngle playerBodyAngles = pPlayer->GetAbsAngles();
	Vector playerBodyOrigin = pPlayer->GetAbsOrigin();
	QAngle playerHeadAngles = pPlayer->EyeAngles();
	Vector playerHeadOrigin;
	pPlayer->EyeVectors(&playerHeadOrigin);

	char buf[AA_MAX_STRING];
	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", playerBodyOrigin.x, playerBodyOrigin.y, playerBodyOrigin.z);
	std::string bodyOrigin = buf;

	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", playerBodyAngles.x, playerBodyAngles.y, playerBodyAngles.z);
	std::string bodyAngles = buf;

	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", playerHeadOrigin.x, playerHeadOrigin.y, playerHeadOrigin.z);
	std::string headOrigin = buf;

	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", playerHeadAngles.x, playerHeadAngles.y, playerHeadAngles.z);
	std::string headAngles = buf;

	std::string object;
	std::string item;
	std::string model;
	std::string webUrl;
	std::string mouseX = "0";
	std::string mouseY = "0";

	C_PropShortcutEntity* pSelectedShortcut = null;
	C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetCanvasManager()->GetDisplayInstance();//g_pAnarchyManager->GetCanvasManager()->GetFirstInstanceToDisplay()
	if (pEmbeddedInstance)
		pSelectedShortcut = dynamic_cast<C_PropShortcutEntity*>(C_BaseEntity::Instance(pEmbeddedInstance->GetOriginalEntIndex()));

	if (!pSelectedShortcut)
	{
		// If no shortcut determined from display instance, try the selected entity
		pSelectedShortcut = dynamic_cast<C_PropShortcutEntity*>(g_pAnarchyManager->GetSelectedEntity());
	}

	if (pSelectedShortcut)
	{
		if (!pEmbeddedInstance)
			pEmbeddedInstance = g_pAnarchyManager->GetCanvasManager()->FindEmbeddedInstance("auto" + pSelectedShortcut->GetItemId());


		object = pSelectedShortcut->GetObjectId();
		item = pSelectedShortcut->GetItemId();
		model = pSelectedShortcut->GetModelId();

		if (pEmbeddedInstance)
		{
			webUrl = pEmbeddedInstance->GetURL();

			float fMouseX;
			float fMouseY;
			pEmbeddedInstance->GetLastMouse(fMouseX, fMouseY);

			mouseX = VarArgs("%.10f", fMouseX);
			mouseY = VarArgs("%.10f", fMouseY);
		}
		else
		{
			webUrl = "";
			mouseX = "0";
			mouseY = "0";
		}

	}

	std::vector<std::string> args;
	args.push_back(g_pAnarchyManager->GetInstanceId());
	args.push_back(m_say);
	args.push_back(bodyOrigin);
	args.push_back(bodyAngles);
	args.push_back(headOrigin);
	args.push_back(headAngles);
	args.push_back(item);
	args.push_back(object);
	args.push_back(mouseX);
	args.push_back(mouseY);
	args.push_back(webUrl);

	// update our local user object too, so we can be treated just like all other users in the menus...
	m_pLocalUser->say = m_say;
	m_pLocalUser->bodyAngles = playerBodyAngles;
	m_pLocalUser->bodyOrigin = playerBodyOrigin;
	m_pLocalUser->headAngles = playerHeadAngles;
	m_pLocalUser->headOrigin = playerHeadOrigin;
	m_pLocalUser->itemId = item;
	m_pLocalUser->sessionId = g_pAnarchyManager->GetConnectedUniverse()->session;
	m_pLocalUser->objectId = object;
	m_pLocalUser->mouseX = mouseX;
	m_pLocalUser->mouseY = mouseY;
	m_pLocalUser->webUrl = webUrl;

	pNetworkBrowserInstance->DispatchJavaScriptMethod("aampNetwork", "localUserUpdate", args);
}

//void C_MetaverseManager::ImportSteamGame(std::string name, std::string appid)
void C_MetaverseManager::ImportSteamGames(KeyValues* kv)
{
	// 1st save this out
	kv->SaveToFile(g_pFullFileSystem, "steamGames.key", "DEFAULT_WRITE_PATH");
	m_pImportSteamGamesKV = kv;
}

void C_MetaverseManager::StartImportSteamGames()
{
	unsigned int count = 0;
	for (KeyValues *sub = m_pImportSteamGamesKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		count++;

	m_uNumSteamGamesToImport = count;
	m_uNumSteamGamesToImported = 0;

	m_pImportSteamGamesSubKV = m_pImportSteamGamesKV->GetFirstSubKey();
	this->ImportNextSteamGame();
}

void C_MetaverseManager::ImportNextSteamGame()
{
	// now loop through it and add any missing games to the user library
	std::string appid;
	std::string name;
	std::string detailsurl;
	std::string screenurl;
	std::string marqueeurl;

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");

	//for (KeyValues *sub = kv->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	if ( m_pImportSteamGamesSubKV )
	{
		appid = m_pImportSteamGamesSubKV->GetName();
		appid = appid.substr(2);
		name = m_pImportSteamGamesSubKV->GetString();
		//DevMsg("Adding game %s w/ appid %s\n", name.c_str(), appid.c_str());

		// 1. try to find an item with this fileid
		// 2. if not found, create an item for it
		// 3. victory bowl

		// build the search info
		KeyValues* pSearchInfo = new KeyValues("search");	// this gets deleted by the metaverse manager!!
		pSearchInfo->SetString("file", appid.c_str());

		KeyValues* item = this->FindLibraryItem(pSearchInfo);
		if (!item)
		{
			//DevMsg("Adding Steam game w/ id %s - %s\n", appid.c_str(), name.c_str());
			item = new KeyValues("item");
			item->SetInt("generation", 3);
			item->SetInt("local/info/created", 0);
			item->SetString("local/info/owner", "local");
			item->SetInt("local/info/removed", 0);
			item->SetString("local/info/remover", "");
			item->SetString("local/info/alias", "");
			item->SetString("local/info/id", g_pAnarchyManager->GenerateUniqueId());
			item->SetString("local/title", name.c_str());
			item->SetString("local/description", "");
			item->SetString("local/file", appid.c_str());

			item->SetString("local/type", this->ResolveLegacyType("pc").c_str());
			item->SetString("local/app", this->ResolveLegacyApp("Steam").c_str());

			detailsurl = "http://store.steampowered.com/app/" + appid;
			item->SetString("local/reference", detailsurl.c_str());

			item->SetString("local/preview", "");
			item->SetString("local/download", "");
			item->SetString("local/stream", "");

			screenurl = "http://cdn.akamai.steamstatic.com/steam/apps/" + appid + "/page_bg_generated.jpg";	// not the best or most reliable url for a screenshot, but is a standard format.
			item->SetString("local/screen", screenurl.c_str());

			marqueeurl = "http://cdn.akamai.steamstatic.com/steam/apps/" + appid + "/header.jpg";
			item->SetString("local/marquee", marqueeurl.c_str());

			this->AddItem(item);

			// JUST SAVE THE ITEM OUT NOW.  this way steamGames.key doesn't have to be iterated through every time AArcade starts.
			g_pAnarchyManager->GetMetaverseManager()->SaveItem(item);

			m_uNumSteamGamesToImported++;
			m_pImportSteamGamesSubKV = m_pImportSteamGamesSubKV->GetNextKey();
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Processing Steam Games", "importsteamgames", "0", VarArgs("%u", m_uNumSteamGamesToImport), "+", "importNextSteamGameCallback");
		}
		else
		{
			m_pImportSteamGamesSubKV = m_pImportSteamGamesSubKV->GetNextKey();
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Processing Steam Games", "importsteamgames", "0", VarArgs("%u", m_uNumSteamGamesToImport), "+", "importNextSteamGameCallback");
		}
	}
	else
	{
		m_pImportSteamGamesKV->deleteThis();
		m_pImportSteamGamesKV = null;
		m_pImportSteamGamesSubKV = null;

		//pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Processing Steam Games", "importsteamgames", "0", VarArgs("%u", m_uNumSteamGamesToImport), "+0", "");
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Finished Importing Steam Games", "importsteamgamescomplete", "", "", VarArgs("%u", m_uNumSteamGamesToImported), "");
		g_pAnarchyManager->AddToastMessage(VarArgs("Steam Games Imported (%u)", m_uNumSteamGamesToImported));

		std::vector<std::string> args;
		pHudBrowserInstance->DispatchJavaScriptMethod("eventListener", "doneImportingSteamGames", args);
	}
}

void C_MetaverseManager::AddType(KeyValues* pType)
{
	KeyValues* active = this->GetActiveKeyValues(pType);
	if (active)
	{
		std::string id = VarArgs("%s", active->GetString("info/id"));
		m_types[id] = pType;
	}
}

void C_MetaverseManager::AddItem(KeyValues* pItem)
{
	KeyValues* active = this->GetActiveKeyValues(pItem);
	if (active)
	{
		std::string id = VarArgs("%s", active->GetString("info/id"));
		m_items[id] = pItem;
	}
}

void C_MetaverseManager::AddModel(KeyValues* pModel)
{
	KeyValues* active = this->GetActiveKeyValues(pModel);
	if (active)
	{
		std::string id = VarArgs("%s", active->GetString("info/id"));
		m_models[id] = pModel;
	}
}

void C_MetaverseManager::DeleteSQL(sqlite3** pDb, const char* tableName, const char* id)
{
	if (!pDb)
		pDb = &m_db;

	char *error;
	sqlite3_stmt *stmt = NULL;
	const char *sqlDeleteInstance = VarArgs("DELETE from %s where id=\"%s\";", tableName, id);
	int rc = sqlite3_exec(*pDb, sqlDeleteInstance, NULL, NULL, &error);

	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(*pDb));
		sqlite3_free(error);
	}
}

void C_MetaverseManager::SaveSQL(sqlite3** pDb, const char* tableName, const char* id, KeyValues* kv)
{
	if (!pDb)
		pDb = &m_db;

	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(*pDb, VarArgs("REPLACE INTO %s VALUES(\"%s\", ?)", tableName, id), -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: prepare failed: %s\n", sqlite3_errmsg(*pDb));

	// SQLITE_STATIC because the statement is finalized before the buffer is freed:
	CUtlBuffer buf;
	kv->WriteAsBinary(buf);

	int size = buf.Size();
	rc = sqlite3_bind_blob(stmt, 1, buf.Base(), size, SQLITE_STATIC);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: bind failed: %s\n", sqlite3_errmsg(*pDb));
	else
	{
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
			DevMsg("FATAL ERROR: execution failed: %s\n", sqlite3_errmsg(*pDb));
	}
	sqlite3_finalize(stmt);
	buf.Purge();
}

void C_MetaverseManager::SaveItem(KeyValues* pItem, sqlite3* pDb)
{
	if (!pDb)
		pDb = m_db;

	KeyValues* active = this->GetActiveKeyValues(pItem);
	/*
	pItem->FindKey("current");
	if (!active)
		active = pItem->FindKey("local");
		*/

	// FIXME: ALWAYS SAVING TO ACTIVE, BUT WHEN OTHER KEY SLOTS GET USED, WILL HAVE TO SAVE TO USE LOGIC TO DETERMINE WHICH SUB-KEY WE'RE SAVING.
	// NOTE: We're only using the item to get the ID and to update its modified time.
	active->SetString("info/modified", VarArgs("%llu", g_pAnarchyManager->GetTimeNumber()));	// save as string because KeyValue's have no uint64 data type.

	// FIXME: SAVING TO .KEY FILES DISABLED FOR MYSQL MIGRATION!!
	// And dodged a bullet there, because when KV's get saved & loaded from disk, they forget their types, and ID's that start with 0 get screwed.
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(pDb, VarArgs("REPLACE INTO items VALUES(\"%s\", ?)", active->GetString("info/id")), -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: prepare failed: %s\n", sqlite3_errmsg(pDb));

	// SQLITE_STATIC because the statement is finalized before the buffer is freed:
	CUtlBuffer buf;
	pItem->WriteAsBinary(buf);

	int size = buf.Size();
	rc = sqlite3_bind_blob(stmt, 1, buf.Base(), size, SQLITE_STATIC);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: bind failed: %s\n", sqlite3_errmsg(pDb));
	else
	{
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
			DevMsg("FATAL ERROR: execution failed: %s\n", sqlite3_errmsg(pDb));
	}
	sqlite3_finalize(stmt);
	buf.Purge();
	//return rc;
	//pItem->SaveToFile(g_pFullFileSystem, VarArgs("library/items/%s.key", active->GetString("info/id")), "DEFAULT_WRITE_PATH");
	//DevMsg("Saved item %s to library/items/%s.key\n", active->GetString("title"), active->GetString("info/id"));
}

void C_MetaverseManager::SaveApp(KeyValues* pApp, sqlite3* pDb)
{
	if (!pDb)
		pDb = m_db;

	KeyValues* active = this->GetActiveKeyValues(pApp);

	// FIXME: ALWAYS SAVING TO ACTIVE, BUT WHEN OTHER KEY SLOTS GET USED, WILL HAVE TO SAVE TO USE LOGIC TO DETERMINE WHICH SUB-KEY WE'RE SAVING.
	// NOTE: We're only using the app to get the ID and to update its modified time.
	active->SetString("info/modified", VarArgs("%llu", g_pAnarchyManager->GetTimeNumber()));	// save as string because KeyValue's have no uint64 data type.

	// FIXME: SAVING TO .KEY FILES DISABLED FOR MYSQL MIGRATION!!
	// And dodged a bullet there, because when KV's get saved & loaded from disk, they forget their types, and ID's that start with 0 get screwed.
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(pDb, VarArgs("REPLACE INTO apps VALUES(\"%s\", ?)", active->GetString("info/id")), -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: prepare failed: %s\n", sqlite3_errmsg(pDb));

	// SQLITE_STATIC because the statement is finalized before the buffer is freed:
	CUtlBuffer buf;
	pApp->WriteAsBinary(buf);

	int size = buf.Size();
	rc = sqlite3_bind_blob(stmt, 1, buf.Base(), size, SQLITE_STATIC);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: bind failed: %s\n", sqlite3_errmsg(pDb));
	else
	{
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
			DevMsg("FATAL ERROR: execution failed: %s\n", sqlite3_errmsg(pDb));
	}
	sqlite3_finalize(stmt);
	buf.Purge();
}

void C_MetaverseManager::SaveModel(KeyValues* pItem, sqlite3* pDb)
{
	if (!pDb)
		pDb = m_db;

	KeyValues* active = this->GetActiveKeyValues(pItem);
	/*
	pItem->FindKey("current");
	if (!active)
	active = pItem->FindKey("local");
	*/

	// FIXME: ALWAYS SAVING TO ACTIVE, BUT WHEN OTHER KEY SLOTS GET USED, WILL HAVE TO SAVE TO USE LOGIC TO DETERMINE WHICH SUB-KEY WE'RE SAVING.
	// NOTE: We're only using the item to get the ID and to update its modified time.
	active->SetString("info/modified", VarArgs("%llu", g_pAnarchyManager->GetTimeNumber()));	// save as string because KeyValue's have no uint64 data type.

	// FIXME: SAVING TO .KEY FILES DISABLED FOR MYSQL MIGRATION!!
	// And dodged a bullet there, because when KV's get saved & loaded from disk, they forget their types, and ID's that start with 0 get screwed.
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(pDb, VarArgs("REPLACE INTO models VALUES(\"%s\", ?)", active->GetString("info/id")), -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: prepare failed: %s\n", sqlite3_errmsg(pDb));

	// SQLITE_STATIC because the statement is finalized before the buffer is freed:
	CUtlBuffer buf;
	pItem->WriteAsBinary(buf);

	int size = buf.Size();
	rc = sqlite3_bind_blob(stmt, 1, buf.Base(), size, SQLITE_STATIC);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: bind failed: %s\n", sqlite3_errmsg(pDb));
	else
	{
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
			DevMsg("FATAL ERROR: execution failed: %s\n", sqlite3_errmsg(pDb));
	}
	sqlite3_finalize(stmt);
	buf.Purge();
}

void C_MetaverseManager::SaveType(KeyValues* pType, sqlite3* pDb)
{
	if (!pDb)
		pDb = m_db;

	KeyValues* active = this->GetActiveKeyValues(pType);

	// FIXME: ALWAYS SAVING TO ACTIVE, BUT WHEN OTHER KEY SLOTS GET USED, WILL HAVE TO SAVE TO USE LOGIC TO DETERMINE WHICH SUB-KEY WE'RE SAVING.
	// NOTE: We're only using the item to get the ID and to update its modified time.
	active->SetString("info/modified", VarArgs("%llu", g_pAnarchyManager->GetTimeNumber()));	// save as string because KeyValue's have no uint64 data type.

	// FIXME: SAVING TO .KEY FILES DISABLED FOR MYSQL MIGRATION!!
	// And dodged a bullet there, because when KV's get saved & loaded from disk, they forget their types, and ID's that start with 0 get screwed.
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(pDb, VarArgs("REPLACE INTO types VALUES(\"%s\", ?)", active->GetString("info/id")), -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: prepare failed: %s\n", sqlite3_errmsg(pDb));

	// SQLITE_STATIC because the statement is finalized before the buffer is freed:
	CUtlBuffer buf;
	pType->WriteAsBinary(buf);

	int size = buf.Size();
	rc = sqlite3_bind_blob(stmt, 1, buf.Base(), size, SQLITE_STATIC);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: bind failed: %s\n", sqlite3_errmsg(pDb));
	else
	{
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
			DevMsg("FATAL ERROR: execution failed: %s\n", sqlite3_errmsg(pDb));
	}
	sqlite3_finalize(stmt);
	buf.Purge();
}
/*
void C_MetaverseManager::SaveMap(KeyValues* pMap, sqlite3* pDb)
{
	if (!pDb)
		pDb = m_db;

	KeyValues* active = this->GetActiveKeyValues(pType);

	// FIXME: ALWAYS SAVING TO ACTIVE, BUT WHEN OTHER KEY SLOTS GET USED, WILL HAVE TO SAVE TO USE LOGIC TO DETERMINE WHICH SUB-KEY WE'RE SAVING.
	// NOTE: We're only using the item to get the ID and to update its modified time.
	active->SetString("info/modified", VarArgs("%llu", g_pAnarchyManager->GetTimeNumber()));	// save as string because KeyValue's have no uint64 data type.

	// FIXME: SAVING TO .KEY FILES DISABLED FOR MYSQL MIGRATION!!
	// And dodged a bullet there, because when KV's get saved & loaded from disk, they forget their types, and ID's that start with 0 get screwed.
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(pDb, VarArgs("REPLACE INTO types VALUES(\"%s\", ?)", active->GetString("info/id")), -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: prepare failed: %s\n", sqlite3_errmsg(pDb));

	// SQLITE_STATIC because the statement is finalized before the buffer is freed:
	CUtlBuffer buf;
	pType->WriteAsBinary(buf);

	int size = buf.Size();
	rc = sqlite3_bind_blob(stmt, 1, buf.Base(), size, SQLITE_STATIC);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: bind failed: %s\n", sqlite3_errmsg(pDb));
	else
	{
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
			DevMsg("FATAL ERROR: execution failed: %s\n", sqlite3_errmsg(pDb));
	}
	sqlite3_finalize(stmt);
	buf.Purge();
}
*/

// Create the item and return it, but don't save it or add it to the library.
bool C_MetaverseManager::CreateItem(int iLegacy, std::string itemId, KeyValues* pItemKV, std::string title, std::string description, std::string file, std::string type, std::string app, std::string reference, std::string preview, std::string download, std::string stream, std::string screen, std::string marquee, std::string model)
{
	// TODO NEXT TIME:
	// MAKE USE OF THIS CREATE ITEM METHOD EVERYWHERE POSSIBLE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if (!pItemKV)
	{
		DevMsg("No item given to CreateItem! Aborting.\n");
		return false;
	}
	
	if (itemId == "")
		itemId = g_pAnarchyManager->GenerateUniqueId();

	pItemKV->SetInt("generation", 3);
	pItemKV->SetInt("legacy", iLegacy);

	KeyValues* pActiveKV = this->GetActiveKeyValues(pItemKV);

	// add standard info
	pActiveKV->SetString("info/id", itemId.c_str());
	pActiveKV->SetString("info/created", VarArgs("%llu", g_pAnarchyManager->GetTimeNumber()));
	pActiveKV->SetString("info/owner", "local");
	//pActiveKV->SetString("info/removed", "");
	//pActiveKV->SetString("info/remover", "");
	//pActiveKV->SetString("info/alias", "");

	pActiveKV->SetString("title", title.c_str());
	pActiveKV->SetString("description", description.c_str());
	pActiveKV->SetString("file", file.c_str());
	pActiveKV->SetString("type", type.c_str());	//AA_DEFAULT_TYPEID
	pActiveKV->SetString("app", app.c_str());
	pActiveKV->SetString("reference", reference.c_str());
	pActiveKV->SetString("preview", preview.c_str());
	pActiveKV->SetString("download", download.c_str());
	pActiveKV->SetString("stream", stream.c_str());
	pActiveKV->SetString("screen", screen.c_str());
	pActiveKV->SetString("marquee", marquee.c_str());
	pActiveKV->SetString("model", model.c_str());

	return pItemKV;
}

bool C_MetaverseManager::DeleteApp(std::string appId)
{
	bool bSuccess = false;
	KeyValues* pAppKV = g_pAnarchyManager->GetMetaverseManager()->GetLibraryApp(appId);
	if (pAppKV)
	{
		auto it = m_apps.find(appId);
		m_apps.erase(it);
		this->DeleteSQL(null, "apps", appId.c_str());

		g_pAnarchyManager->AddToastMessage("Open-With App Profile Deleted");
	}

	return bSuccess;
}

void C_MetaverseManager::SmartMergItemKVs(KeyValues* pItemA, KeyValues* pItemB, bool bPreferFirst)
{
	KeyValues* pKeeperItem = (bPreferFirst) ? pItemA : pItemB;
	KeyValues* pMergerItem = (bPreferFirst) ? pItemB : pItemA;

	KeyValues* pActiveKeeper = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pKeeperItem);
	KeyValues* pActiveMerger = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pMergerItem);

	if (Q_strcmp(pActiveKeeper->GetString("info/id"), pActiveMerger->GetString("info/id")))
		DevMsg("WARNING: SmargMerg-ing 2 items with different IDs: %s %s\n", pActiveKeeper->GetString("info/id"), pActiveMerger->GetString("info/id"));

	// pKeeperItem
	// - Contains the most user-customized versions of fields.
	// - Fields with local file paths are probably important.

	// Compare "app values.
	// If the Primary uses an app, keep it.  Otherwise, use the one from Secondary.
	if (!Q_strcmp(pActiveKeeper->GetString("app"), "") && Q_strcmp(pActiveMerger->GetString("app"), ""))
		pActiveKeeper->SetString("app", pActiveMerger->GetString("app"));

	// Compare "file" values.
	// If Primary uses a non-internet location, keep it.
	bool bSecondaryFileLocationIsGood = true;
	if (!Q_strcmp(pActiveMerger->GetString("file"), "") || !Q_strcmp(pActiveMerger->GetString("file"), pActiveMerger->GetString("title")) || !Q_stricmp(pActiveMerger->GetString("file"), pActiveKeeper->GetString("file")))
		bSecondaryFileLocationIsGood = false;

	if (bSecondaryFileLocationIsGood)
	{
		std::string primaryFileLocation = pActiveKeeper->GetString("file");

		bool bPrimaryFileLocationIsGood = true;
		if (!Q_strcmp(pActiveKeeper->GetString("file"), "") || !Q_strcmp(pActiveKeeper->GetString("file"), pActiveKeeper->GetString("title")) || primaryFileLocation.find("http://") == 0 || primaryFileLocation.find("https://") == 0 || primaryFileLocation.find("www.") == 0)
			bPrimaryFileLocationIsGood = false;

		if (!bPrimaryFileLocationIsGood)
		{
			bool bSecondaryFileLocationIsAWebImage = false;

			if (Q_strcmp(pActiveKeeper->GetString("file"), ""))
			{
				std::string imageTypes = "|jpg|jpeg|gif|png|bmp|tga|tbn|";

				// Find out if Secondary's "file" is a web image.
				std::string secondaryFileLocationExtension = pActiveMerger->GetString("file");
				if (secondaryFileLocationExtension.find("http://") == 0 || secondaryFileLocationExtension.find("https://") == 0 || secondaryFileLocationExtension.find("www.") == 0)
				{
					size_t found = secondaryFileLocationExtension.find_last_of(".");
					if (found != std::string::npos)
					{
						secondaryFileLocationExtension = secondaryFileLocationExtension.substr(found + 1);

						if (imageTypes.find(VarArgs("|%s|", secondaryFileLocationExtension.c_str())) != std::string::npos)
						{
							bSecondaryFileLocationIsAWebImage = true;
						}
					}
				}
			}

			if (bSecondaryFileLocationIsAWebImage)
			{
				// So, we have a "file" property about to be replaced with a web image.  We need to determine if we should replace the screen and marquee URLs now along with it.

				bool bPrimaryScreensLocationIsALocalImage = false;
				std::string primaryScreensLocation = pActiveKeeper->GetString("screen");

				if (primaryScreensLocation.find(":") == 1)
					bPrimaryScreensLocationIsALocalImage = true;

				if (!bPrimaryScreensLocationIsALocalImage)
				{
					if (!Q_strcmp(pActiveKeeper->GetString("screen"), "") || !Q_strcmp(pActiveKeeper->GetString("screen"), pActiveKeeper->GetString("file")))
					{
						pActiveKeeper->SetString("screen", pActiveMerger->GetString("file"));

						// Clear Primary's "screenslocation" as well so the image refreshes next time.
						// First,  delete the cached version of the image so it can be re-downloaded.
						/*
						if (Q_strcmp(pActiveKeeper->GetString("screen"), "") && g_pFullFileSystem->FileExists(pActiveKeeper->GetString("screen"), USE_GAME_PATH))
						{
							if (pClientArcadeResources->GetReallyDoneStarting())
							{
								pClientArcadeResources->DeleteLocalFile(pActiveKeeper->GetString("screenslocation"));
								pActiveKeeper->SetString("screenslocation", "");	// Only clear this if we are really gonna re-download it, for speed up 3/30/2015
							}
						}
						*/

						//						pPrimaryItemKV->SetString("screenslocation", "");	// Move this higher to try to improve speedup
					}
				}

				bool bPrimaryMarqueesLocationIsALocalImage = false;
				std::string primaryMarqueesLocation = pActiveKeeper->GetString("marquee");

				if (primaryMarqueesLocation.find(":") == 1)
					bPrimaryMarqueesLocationIsALocalImage = true;

				if (!bPrimaryMarqueesLocationIsALocalImage)
				{
					if (!Q_strcmp(pActiveKeeper->GetString("marquee"), "") || !Q_strcmp(pActiveKeeper->GetString("marquee"), pActiveKeeper->GetString("file")))
					{
						pActiveKeeper->SetString("marquee", pActiveMerger->GetString("file"));

						// Clear Primary's "marqueeslocation" as well so the image refreshes next time.
						// First,  delete the cached version of the image so it can be re-downloaded.
						/*
						if (Q_strcmp(pActiveKeeper->GetString("marqueeslocation"), "") && g_pFullFileSystem->FileExists(pActiveKeeper->GetString("marqueeslocation"), USE_GAME_PATH))
						{
							if (pClientArcadeResources->GetReallyDoneStarting())
							{
								pClientArcadeResources->DeleteLocalFile(pActiveKeeper->GetString("marqueeslocation"));

								pActiveKeeper->SetString("marqueeslocation", "");
							}
						}
						*/
					}
				}
			}

			// The "file" property is going to change, and if Primary has no "trailerurl", then Primary's "noembed" property should change along with it.
//			if (!Q_strcmp(pActiveKeeper->GetString("preview"), ""))
	//			pActiveKeeper->SetInt("noembed", pActiveMerger->GetInt("noembed"));


			// Now replace the "file" property.
			pActiveKeeper->SetString("file", pActiveMerger->GetString("file"));
		}
	}

	// Compare "screens/low" values.
	// If Primary's screenslocation is not a local image, use Secondary's.
	bool bSecondaryScreensURLIsGood = true;
	if (!Q_strcmp(pActiveMerger->GetString("screen"), "") || !Q_stricmp(pActiveMerger->GetString("screen"), pActiveKeeper->GetString("screen")))
		bSecondaryScreensURLIsGood = false;

	if (bSecondaryScreensURLIsGood)
	{
		bool bPrimaryScreensLocationIsALocalImage = false;
		std::string primaryScreensLocation = pActiveKeeper->GetString("screen");

		if (primaryScreensLocation.find(":") == 1)
			bPrimaryScreensLocationIsALocalImage = true;

		if (!bPrimaryScreensLocationIsALocalImage)
		{
			// So Secondary's "screens/low" is good, and there is no local image used on Primary's "screenslocation".
			pActiveKeeper->SetString("screen", pActiveMerger->GetString("screen"));

			// Clear Primary's "screenslocation" as well so the image refreshes next time.
			// First,  delete the cached version of the image so it can be re-downloaded.
			/*
			if (Q_strcmp(pActiveKeeper->GetString("screenslocation"), "") && g_pFullFileSystem->FileExists(pActiveKeeper->GetString("screenslocation"), USE_GAME_PATH))
			{
				if (pClientArcadeResources->GetReallyDoneStarting())
				{
					pClientArcadeResources->DeleteLocalFile(pActiveKeeper->GetString("screenslocation"));
					pActiveKeeper->SetString("screenslocation", "");
				}
			}
			*/
		}
	}

	// Compare "marquees/low" values.
	// If Primary's marqueeslocation is not a local image, use Secondary's.
	bool bSecondaryMarqueesURLIsGood = true;
	if (!Q_strcmp(pActiveMerger->GetString("marquee"), "") || !Q_stricmp(pActiveMerger->GetString("marquee"), pActiveKeeper->GetString("marquee")))
		bSecondaryMarqueesURLIsGood = false;

	if (bSecondaryMarqueesURLIsGood)
	{
		bool bPrimaryMarqueesLocationIsALocalImage = false;
		std::string primaryMarqueesLocation = pActiveKeeper->GetString("marquee");

		if (primaryMarqueesLocation.find(":") == 1)
			bPrimaryMarqueesLocationIsALocalImage = true;

		if (!bPrimaryMarqueesLocationIsALocalImage)
		{
			// So Secondary's "marquees/low" is good, and there is no local image used on Primary's "marqueeslocation".
			pActiveKeeper->SetString("marquee", pActiveMerger->GetString("marquee"));

			// Clear Primary's "marqueeslocation" as well so the image refreshes next time.
			// First,  delete the cached version of the image so it can be re-downloaded.
			/*
			if (Q_strcmp(pActiveKeeper->GetString("marqueeslocation"), "") && g_pFullFileSystem->FileExists(pActiveKeeper->GetString("marqueeslocation"), USE_GAME_PATH))
			{
				if (pClientArcadeResources->GetReallyDoneStarting())
				{
					pClientArcadeResources->DeleteLocalFile(pActiveKeeper->GetString("marqueeslocation"));
					pActiveKeeper->SetString("marqueeslocation", "");
				}
			}
			*/
		}
	}

	// Compare "trailerurl" values
	if (Q_strcmp(pActiveMerger->GetString("preview"), ""))
		pActiveKeeper->SetString("preview", pActiveMerger->GetString("preview"));

	/*
	// Compare "publishedfileid" values
	if (Q_strcmp(pActiveMerger->GetString("publishedfileid"), ""))
		pActiveKeeper->SetString("publishedfileid", pActiveMerger->GetString("publishedfileid"));
	*/

	// Compare "model" values
	if (Q_strcmp(pActiveMerger->GetString("model"), ""))
		pActiveKeeper->SetString("model", pActiveMerger->GetString("model"));

	// Compare "type" values
	// Always use Secondary's "type" (unless maybe we have a m_pKeepMyItem"Names"ConVar?)
	pActiveKeeper->SetString("type", pActiveMerger->GetString("type"));

	// If there was a m_bKeepMyItemNamesConvar, this is where it should be used.
	/*
	if (bFullMerg)
	{
		// Compare "title" values
		if (Q_strcmp(pActiveMerger->GetString("title"), ""))
			pActiveKeeper->SetString("title", pActiveMerger->GetString("title"));

		// Compare "description" values
		if (Q_strcmp(pActiveMerger->GetString("description"), ""))
			pActiveKeeper->SetString("description", pActiveMerger->GetString("description"));

		// Compare "downloadurl" values
		if (Q_strcmp(pActiveMerger->GetString("downloadurl"), ""))
			pActiveKeeper->SetString("downloadurl", pActiveMerger->GetString("downloadurl"));

		// Compare "detailsurl" values
		if (Q_strcmp(pActiveMerger->GetString("detailsurl"), ""))
			pActiveKeeper->SetString("detailsurl", pActiveMerger->GetString("detailsurl"));

		// Compare "comments" values
		if (Q_strcmp(pActiveMerger->GetString("comments"), ""))
			pActiveKeeper->SetString("comments", pActiveMerger->GetString("comments"));

		// Compare "instructions" values
		if (Q_strcmp(pActiveMerger->GetString("instructions"), ""))
			pActiveKeeper->SetString("instructions", pActiveMerger->GetString("instructions"));

		// Compare "group" values
		if (Q_strcmp(pActiveMerger->GetString("group"), ""))
			pActiveKeeper->SetString("group", pActiveMerger->GetString("group"));

		// Compare "groupurl" values
		if (Q_strcmp(pActiveMerger->GetString("groupurl"), ""))
			pActiveKeeper->SetString("groupurl", pActiveMerger->GetString("groupurl"));

		// Compare "addon" values
		if (Q_strcmp(pActiveMerger->GetString("addon"), ""))
			pActiveKeeper->SetString("addon", pActiveMerger->GetString("addon"));
	}
	*/
}

std::vector<std::string>* C_MetaverseManager::GetDefaultFields()
{
	return &m_defaultFields;
}

void C_MetaverseManager::UpdateScrapersJS()
{
	std::vector<std::string> scraperList;

	// get a list of all the .js files in the folder
	FileFindHandle_t handle;
	const char *pFilename = g_pFullFileSystem->FindFirstEx("resource\\ui\\html\\scrapers\\*.js", "MOD", &handle);
	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(handle))
		{
			pFilename = g_pFullFileSystem->FindNext(handle);
			continue;
		}

		scraperList.push_back(pFilename);
		pFilename = g_pFullFileSystem->FindNext(handle);
	}

	// generate code
	std::string code = "";//"if( !!arcadeHud) { ";
	unsigned int max = scraperList.size();
	for (unsigned int i = 0; i < max; i++)
		code += "arcadeHud.loadHeadScript(\"scrapers/" + scraperList[i] + "\");\n";
	//code += " } else console.log('ERROR: arcadeHud object was NOT ready to receive the scraper list.');";

	// open up scrapers.js and replace it.  this path is greated when AArcade is started.
	FileHandle_t fh = filesystem->Open("resource\\ui\\html\\scrapers.js", "w", "DEFAULT_WRITE_PATH");
	if (fh)
	{
		//filesystem->FPrintf(fh, "%s", code.c_str());
		filesystem->Write(code.c_str(), V_strlen(code.c_str()), fh);
		filesystem->Close(fh);
	}
}

bool C_MetaverseManager::LoadSQLKevValues(const char* tableName, const char* id, KeyValues* kv, sqlite3* pDb)
{
	if (!pDb)
		pDb = m_db;

	sqlite3_stmt *stmt = NULL;
	//DevMsg("loading from table name: %s _id %s\n", tableName, id);
	int rc = sqlite3_prepare(pDb, VarArgs("SELECT * from %s WHERE id = \"%s\"", tableName, id), -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed: %s\n", sqlite3_errmsg(pDb));

	bool bSuccess = false;
	int length;
	if (sqlite3_step(stmt) == SQLITE_ROW)	// THIS IS WHERE THE LOOP CAN BE BROKEN UP AT!!
	{
		length = sqlite3_column_bytes(stmt, 1);

		if (length > 0)
		{
			CUtlBuffer buf(0, length, 0);
			buf.CopyBuffer(sqlite3_column_blob(stmt, 1), length);
			if (kv->ReadAsBinary(buf))
				bSuccess = true;
			buf.Purge();
		}
		else
			bSuccess = false;
	}
	sqlite3_finalize(stmt);	// TODO: error checking?  Maybe not needed, if this is like a close() operation.

	return bSuccess;
}

void C_MetaverseManager::SaveInstanceTitle(instance_t* pInstance)
{
	C_Backpack* pBackpack = null;
	KeyValues* pInstanceKV = new KeyValues("instance");
	if (!this->LoadSQLKevValues("instances", pInstance->id.c_str(), pInstanceKV))
	{
		// if this wasn't in our library, try other librarys.
		// check all backpacks...
		pBackpack = g_pAnarchyManager->GetBackpackManager()->FindBackpackWithInstanceId(pInstance->id);
		if (pBackpack)
		{
			// we found the backpack containing this instance ID
			DevMsg("Loading from instance backpack w/ ID %s...\n", pBackpack->GetId().c_str());
			pBackpack->OpenDb();
			sqlite3* pDb = pBackpack->GetSQLDb();
			if (!pDb || !g_pAnarchyManager->GetMetaverseManager()->LoadSQLKevValues("instances", pInstance->id.c_str(), pInstanceKV, pDb))
			{
				DevMsg("CRITICAL ERROR: Failed to load instance from library!\n");
				pBackpack->CloseDb();
				pBackpack = null;
			}
			else
				pBackpack->CloseDb();
		}

		if (!pBackpack)
		{
			DevMsg("WARNING: Could not load instance!");// Attempting to load as legacy instance...\n");
			pInstanceKV->deleteThis();
			pInstanceKV = null;
		}
	}

	if (pInstanceKV)
	{
		pInstanceKV->SetString("info/local/title", pInstance->title.c_str());

		if (!pBackpack)
			g_pAnarchyManager->GetMetaverseManager()->SaveSQL(null, "instances", pInstance->id.c_str(), pInstanceKV);
		else
		{
			DevMsg("Weird backpack detected! Don't know how to handle it! Fix it!\n");
			//g_pAnarchyManager->GetMetaverseManager()->SaveSQL(*(pBackpack->GetSQLDb()), "instances", pInstance->id.c_str(), pInstanceKV);
		}

		pInstanceKV->deleteThis();
		pInstanceKV = null;
	}
}

void C_MetaverseManager::DeleteInstance(instance_t* pInstance)
{
	// can't delete the currently loaded instance
	if (pInstance->id == g_pAnarchyManager->GetInstanceId())
		return;

	this->DeleteSQL(null, "instances", pInstance->id.c_str());

	g_pAnarchyManager->GetInstanceManager()->RemoveInstance(pInstance);
}

KeyValues* C_MetaverseManager::LoadLocalItemLegacy(bool& bIsModel, bool& bWasAlreadyLoaded, std::string file, std::string filePath, std::string workshopIds, std::string mountIds, C_Backpack* pBackpack, std::string searchPath, bool bShouldAddToActiveLibrary)
{
	KeyValues* pItem = new KeyValues("item");
	bool bLoaded;
	
	bool bWasLoaded = false;

	std::string fullFile = filePath + file;
	if (searchPath != "")
	{
		bLoaded = pItem->LoadFromFile(g_pFullFileSystem, fullFile.c_str(), searchPath.c_str());
	}
	else if (filePath != "")
		bLoaded = pItem->LoadFromFile(g_pFullFileSystem, fullFile.c_str(), "");
	else
		bLoaded = pItem->LoadFromFile(g_pFullFileSystem, file.c_str(), "MOD");

	bool bResponseIsModel;
	if ( !bLoaded )
	{
		pItem->deleteThis();
		pItem = null;
		bResponseIsModel = false;
	}
	else
	{
		// flag us as being loaded from legacy
		pItem->SetBool("loadedFromLegacy", true);

		// determine the generation of this item
		KeyValues* pGeneration = pItem->FindKey("generation");
		if (!pGeneration)
		{
			// update us to 3rd generation
			pItem->SetInt("generation", 3);

			// add standard info (except for id)
			//pItem->SetInt("local/info/created", 0);
			pItem->SetString("local/info/owner", "local");
			//pItem->SetInt("local/info/removed", 0);
			//pItem->SetString("local/info/remover", "");
			//pItem->SetString("local/info/alias", "");

			// determine if this is a model or not
			std::string itemFile = pItem->GetString("filelocation");
			std::string modelFile = pItem->GetString("filelocation");
			//if (modelFile.find("cabinets") >= 0)
				//DevMsg("Harrrr: %s\n", modelFile.c_str());
			size_t foundExt = modelFile.find(".mdl");
			if (foundExt == modelFile.length() - 4)
			{
				bResponseIsModel = true;
				std::string itemId = g_pAnarchyManager->GenerateLegacyHash(itemFile.c_str());
				
				pItem->SetString("local/info/id", itemId.c_str());
				pItem->SetString("local/title", pItem->GetString("title"));
				pItem->SetString("local/preview", pItem->GetString("trailerurl"));

				if (Q_strcmp(pItem->GetString("group"), ""))
					pItem->SetString("local/keywords", pItem->GetString("group"));
				//pItem->SetInt("local/dynamic", 0);
				pItem->SetString(VarArgs("local/platforms/%s/id", AA_PLATFORM_ID), AA_PLATFORM_ID);
				pItem->SetString(VarArgs("local/platforms/%s/file", AA_PLATFORM_ID), modelFile.c_str());
				pItem->SetString(VarArgs("local/platforms/%s/download", AA_PLATFORM_ID), "");

				pItem->SetString(VarArgs("local/platforms/%s/workshopIds", AA_PLATFORM_ID), workshopIds.c_str());
				pItem->SetString(VarArgs("local/platforms/%s/mountIds", AA_PLATFORM_ID), mountIds.c_str());
//				pItem->SetString(VarArgs("local/platforms/%s/backpackIds", AA_PLATFORM_ID), backpackId.c_str());

				// models can be loaded right away because they don't depend on anything else, like items do. (items depend on models)
				//DevMsg("Loading model with ID %s and model %s\n", itemId.c_str(), modelFile.c_str());

				auto it = m_models.find(itemId);
				if (it != m_models.end())
				{
					pItem->deleteThis();
					pItem = it->second;
					bWasLoaded = true;
				}
				else if (bShouldAddToActiveLibrary)
					m_models[itemId] = pItem;
			}
			else
			{
				bResponseIsModel = false;

				std::string nodeId;

				// NEEDS RESOLVING!!
				std::string legacyType = pItem->GetString("type");
				std::string resolvedType = this->ResolveLegacyType(legacyType);
				pItem->SetString("local/type", resolvedType.c_str());

				if (legacyType == "node")
				{
					// Try to extract a legacy ID from the relative .set file path, cuz redux doesn't save shit that way anymore.
					nodeId = g_pAnarchyManager->ExtractLegacyId(itemFile);

					if (nodeId != "")
					{
						DevMsg("Legacy node consumed from workshop with extracted id: %s\n", nodeId.c_str());

						// Replace the local/file field with the nodeId instead.
						itemFile = nodeId;
					}
				}

				std::string itemId = g_pAnarchyManager->ExtractLegacyId(file, pItem);
				pItem->SetString("local/info/id", itemId.c_str());
				pItem->SetString(VarArgs("local/platforms/%s/workshopIds", AA_PLATFORM_ID), workshopIds.c_str());
				pItem->SetString(VarArgs("local/platforms/%s/mountIds", AA_PLATFORM_ID), mountIds.c_str());
				//pItem->SetString(VarArgs("local/platforms/%s/backpackIds", AA_PLATFORM_ID), backpackId.c_str());

				pItem->SetString("local/title", pItem->GetString("title"));

				if (Q_strcmp(pItem->GetString("group"), ""))
					pItem->SetString("local/keywords", pItem->GetString("group"));

				pItem->SetString("local/description", pItem->GetString("description"));
				pItem->SetString("local/file", itemFile.c_str());
				//pItem->SetString("local/file", pItem->GetString("filelocation"));

				// NEEDS RESOLVING!!
				std::string legacyApp = pItem->GetString("app");
				std::string resolvedApp = this->ResolveLegacyApp(legacyApp);
				pItem->SetString("local/app", resolvedApp.c_str());
				pItem->SetString("local/reference", pItem->GetString("detailsurl"));
				pItem->SetString("local/preview", pItem->GetString("trailerurl"));
				pItem->SetString("local/download", pItem->GetString("downloadurl"));

				// NEEDS RESOLVING!!
				std::string resolvedScreen = pItem->GetString("screens/low");
				if (resolvedScreen == "")
				{
					resolvedScreen = pItem->GetString("screenslocation");
					if (resolvedScreen.find(":") != 1 || !g_pFullFileSystem->FileExists(resolvedScreen.c_str(), ""))
						resolvedScreen = "";
				}
				pItem->SetString("local/screen", resolvedScreen.c_str());


				/*
				// CACHE NEEDS RESOLVING!!
				//std::string legacyPath = "A:\\SteamLibrary\\steamapps\\common\\Anarchy Arcade\\aarcade\\";
				char fullPathBuf[AA_MAX_STRING];
				std::string resolvedScreenCached = pItem->GetString("screenslocation");
				//std::string resolvedScreenCached = VarArgs("%s\\%s\\screens\\%s.tbn", library_type.c_str(), newType.c_str(), this->GenerateHashX(itemfile_ref.c_str();
				//BuildItemKV->SetString("marqueeslocation", UTIL_VarArgs("%s\\%s\\marquees\\%s.tbn", library_type.c_str(), newType.c_str(), this->GenerateHashX(itemfile_ref.c_str())));
				//BuildItemKV->SetString("screenslocation", UTIL_VarArgs("%s\\%s\\screens\\%s.tbn", library_type.c_str(), newType.c_str(), this->GenerateHashX(itemfile_ref.c_str())));

				if (resolvedScreenCached != "" && resolvedScreenCached.find(":") != 1)
				{
					g_pFullFileSystem->RelativePathToFullPath(resolvedScreenCached.c_str(), "MOD", fullPathBuf, AA_MAX_STRING);
					resolvedScreenCached = fullPathBuf;

					if (resolvedScreenCached.find(":") != 1)
					{
						DevMsg("Testing sample path: %s\n", file.c_str());
						resolvedScreenCached = VarArgs("%s\\%s\\screens\\%s.tbn", "library", pItem->GetString("type"), g_pAnarchyManager->GenerateLegacyHash(file.c_str()));
						g_pFullFileSystem->RelativePathToFullPath(resolvedScreenCached.c_str(), "MOD", fullPathBuf, AA_MAX_STRING);
						resolvedScreenCached = fullPathBuf;

						if (resolvedScreenCached.find(":") != 1)
						{
							DevMsg("Testing sample path: %s\n", file.c_str());
							resolvedScreenCached = VarArgs("%s\\%s\\screens\\%s.tbn", "library_cache", pItem->GetString("type"), g_pAnarchyManager->GenerateLegacyHash(file.c_str()));
							g_pFullFileSystem->RelativePathToFullPath(resolvedScreenCached.c_str(), "MOD", fullPathBuf, AA_MAX_STRING);
							resolvedScreenCached = fullPathBuf;

							if (resolvedScreenCached.find(":") != 1)
								resolvedScreenCached = "";
						}
					}

					DevMsg("Arbys full path here is: %s\n", resolvedScreenCached.c_str());
				}
				else
					resolvedScreenCached = "";
				pItem->SetString("local/screencached", resolvedScreenCached.c_str());
				*/



				// NEEDS RESOLVING!!
				std::string resolvedMarquee = pItem->GetString("marquees/low");
				if (resolvedMarquee == "")
				{
					resolvedMarquee = pItem->GetString("marqueeslocation");
					if (resolvedMarquee.find(":") != 1 || !g_pFullFileSystem->FileExists(resolvedMarquee.c_str(), ""))
						resolvedMarquee = "";
				}
				pItem->SetString("local/marquee", resolvedMarquee.c_str());



				/*
				// CACHE NEEDS RESOLVING!!
				//std::string legacyPath = "A:\\SteamLibrary\\steamapps\\common\\Anarchy Arcade\\aarcade\\";
				//char fullPathBuf[AA_MAX_STRING];
				std::string resolvedMarqueeCached = pItem->GetString("marqueeslocation");
				if (resolvedMarqueeCached != "" && resolvedMarqueeCached.find(":") != 1)
				{
					g_pFullFileSystem->RelativePathToFullPath(resolvedMarqueeCached.c_str(), "MOD", fullPathBuf, AA_MAX_STRING);
					resolvedMarqueeCached = fullPathBuf;
					DevMsg("Arbys full path here is: %s\n", resolvedMarqueeCached.c_str());
				}
				else
					resolvedMarqueeCached = "";
				pItem->SetString("local/marqueecached", resolvedMarqueeCached.c_str());
				*/

				/*
				auto it = m_items.find(itemId);
				if (it != m_items.end())
				{
					pItem->deleteThis();
					pItem = it->second;
					bWasLoaded = true;
				}
				else if (bShouldAddToActiveLibrary)
					m_items[itemId] = pItem;
				*/

				if ( bShouldAddToActiveLibrary)
					m_previousLoadLocalItemsLegacyBuffer.push_back(pItem);
			}
		}
		else
			bResponseIsModel = false;
	}

	bIsModel = bResponseIsModel;
	bWasAlreadyLoaded = bWasLoaded;
	return pItem;
}

void C_MetaverseManager::LoadFirstLocalItemLegacy(bool bFastMode, std::string filePath, std::string workshopIds, std::string mountIds, C_Backpack* pBackpack)
{
	// only need to do this on the first one.
	//bool bBadFastMode = true;

	if (m_previousLoadLocalItemLegacyFilePath != "")
		this->LoadLocalItemLegacyClose();

	// start it
	m_previousLoadLocalItemLegacyFastMode = bFastMode;
	m_previousLoadLocalItemLegacyFilePath = filePath;
	m_previousLocaLocalItemLegacyWorkshopIds = workshopIds;
	m_previousLoadLocalItemLegacyMountIds = mountIds;
	m_pPreviousLoadLocalItemLegacyBackpack = pBackpack;

	unsigned int uMountWorkshopNumModels = 0;
	unsigned int uMountWorkshopNumItems = 0;

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");

	KeyValues* pItem;
	std::string fullPath = m_previousLoadLocalItemLegacyFilePath + "library\\*";
	const char *pFoldername = g_pFullFileSystem->FindFirst(fullPath.c_str(), &m_previousLoadLocalItemLegacyFolderHandle);
	while (pFoldername != NULL)
	{
		if (!Q_strcmp(pFoldername, ".") || !Q_strcmp(pFoldername, ".."))
		{
			pFoldername = g_pFullFileSystem->FindNext(m_previousLoadLocalItemLegacyFolderHandle);
			continue;
		}

		m_previousLoadLocalItemLegacyFolderPath = VarArgs("library\\%s", pFoldername);
		if (!g_pFullFileSystem->FindIsDirectory(m_previousLoadLocalItemLegacyFolderHandle))
		{
			DevMsg("All items files must be within a subfolder!\n");
			pFoldername = g_pFullFileSystem->FindNext(m_previousLoadLocalItemLegacyFolderHandle);
			continue;
		}

		const char *pFilename = g_pFullFileSystem->FindFirst(VarArgs("%s%s\\*.itm", m_previousLoadLocalItemLegacyFilePath.c_str(), m_previousLoadLocalItemLegacyFolderPath.c_str()), &m_previousLoadLocalItemLegacyFileHandle);
		while (pFilename != NULL)
		{
			if (g_pFullFileSystem->FindIsDirectory(m_previousLoadLocalItemLegacyFileHandle))
			{
				pFilename = g_pFullFileSystem->FindNext(m_previousLoadLocalItemLegacyFileHandle);
				continue;
			}

			// WE'VE FOUND A FILE TO ATTEMPT TO LOAD!!!
			std::string foundName = VarArgs("%s\\%s", m_previousLoadLocalItemLegacyFolderPath.c_str(), pFilename);
			//pFilename = g_pFullFileSystem->FindNext(m_previousLoadLocalItemLegacyFileHandle);

			// MAKE THE FILE PATH NICE
			char path_buffer[AA_MAX_STRING];
			Q_strcpy(path_buffer, foundName.c_str());
			V_FixSlashes(path_buffer);
			foundName = path_buffer;
			// FINISHED MAKING THE FILE PATH NICE

			bool bIsModel;
			bool bWasAlreadyLoaded;
			pItem = this->LoadLocalItemLegacy(bIsModel, bWasAlreadyLoaded, foundName, m_previousLoadLocalItemLegacyFilePath, m_previousLocaLocalItemLegacyWorkshopIds, m_previousLoadLocalItemLegacyMountIds, m_pPreviousLoadLocalItemLegacyBackpack);
			if (pItem)
			{
				if (bIsModel)
					uMountWorkshopNumModels++;
				else
					uMountWorkshopNumItems++;

				if (!bFastMode)
				{
					if (bIsModel)
					{
						if (m_previousLocaLocalItemLegacyWorkshopIds != "")
							pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Workshop Models", "workshoplibrarymodels", "", "", "+", "loadNextLocalItemLegacyCallback");
						else
							pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Importing Old AArcade Models", "oldlibrarymodels", "", "", "+", "loadNextLocalItemLegacyCallback");
					}
					else
					{
						if (m_previousLocaLocalItemLegacyWorkshopIds != "")
							pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Workshop Items", "workshoplibraryitems", "", "", "+", "loadNextLocalItemLegacyCallback");
						else
							pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Importing Old AArcade Items", "oldlibraryitems", "", "", "+", "loadNextLocalItemLegacyCallback");
					}
					return;
				}
			}

			pFilename = g_pFullFileSystem->FindNext(m_previousLoadLocalItemLegacyFileHandle);
		}

		g_pFullFileSystem->FindClose(m_previousLoadLocalItemLegacyFileHandle);
		pFoldername = g_pFullFileSystem->FindNext(m_previousLoadLocalItemLegacyFolderHandle);
		//break;
	}

	if (bFastMode)
	{
		if (m_previousLocaLocalItemLegacyWorkshopIds != "")
		{
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Workshop Models", "workshoplibrarymodels", "", "", std::string(VarArgs("+%u", uMountWorkshopNumModels)));
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Workshop Items", "workshoplibraryitems", "", "", std::string(VarArgs("+%u", uMountWorkshopNumItems)));
		}
		else
		{
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Importing Old AArcade Models", "oldlibrarymodels", "", "", std::string(VarArgs("+%u", uMountWorkshopNumModels)));
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Importing Old AArcade Items", "oldlibraryitems", "", "", std::string(VarArgs("+%u", uMountWorkshopNumItems)));
		}
	}

	g_pAnarchyManager->GetWorkshopManager()->OnMountWorkshopSucceed();	
	this->LoadLocalItemLegacyClose();
	return;
}

void C_MetaverseManager::LoadNextLocalItemLegacy()
{
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");

	unsigned int uMountWorkshopNumModels = 0;
	unsigned int uMountWorkshopNumItems = 0;

	KeyValues* pItem;
	const char *pFilename = g_pFullFileSystem->FindNext(m_previousLoadLocalItemLegacyFileHandle);
	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(m_previousLoadLocalItemLegacyFileHandle))
		{
			pFilename = g_pFullFileSystem->FindNext(m_previousLoadLocalItemLegacyFileHandle);
			continue;
		}

		// WE'VE FOUND A FILE TO ATTEMPT TO LOAD!!!
		std::string foundName = VarArgs("%s\\%s", m_previousLoadLocalItemLegacyFolderPath.c_str(), pFilename);
		//pFilename = g_pFullFileSystem->FindNext(m_previousLoadLocalItemLegacyFileHandle);

		// MAKE THE FILE PATH NICE
		char path_buffer[AA_MAX_STRING];
		Q_strcpy(path_buffer, foundName.c_str());
		V_FixSlashes(path_buffer);
		foundName = path_buffer;
		// FINISHED MAKING THE FILE PATH NICE

		bool bIsModel;
		bool bWasAlreadyLoaded;
		pItem = this->LoadLocalItemLegacy(bIsModel, bWasAlreadyLoaded, foundName, m_previousLoadLocalItemLegacyFilePath, m_previousLocaLocalItemLegacyWorkshopIds, m_previousLoadLocalItemLegacyMountIds, m_pPreviousLoadLocalItemLegacyBackpack);
		if (pItem)
		{
			if (bIsModel)
				uMountWorkshopNumModels++;
			else
				uMountWorkshopNumItems++;

			if (!m_previousLoadLocalItemLegacyFastMode)
			{
				if (bIsModel)
				{
					if (m_previousLocaLocalItemLegacyWorkshopIds != "")
						pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Workshop Models", "workshoplibrarymodels", "", "", "+", "loadNextLocalItemLegacyCallback");
					else
						pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Importing Old AArcade Models", "oldlibrarymodels", "", "", "+", "loadNextLocalItemLegacyCallback");
				}
				else
				{
					if (m_previousLocaLocalItemLegacyWorkshopIds != "")
						pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Workshop Items", "workshoplibraryitems", "", "", "+", "loadNextLocalItemLegacyCallback");
					else
						pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Importing Old AArcade Items", "oldlibraryitems", "", "", "+", "loadNextLocalItemLegacyCallback");
				}
				return;
			}
		}

		pFilename = g_pFullFileSystem->FindNext(m_previousLoadLocalItemLegacyFileHandle);
		//break;
	}

	g_pFullFileSystem->FindClose(m_previousLoadLocalItemLegacyFileHandle);

	// done searching a folder, continue on to next folder
	std::string fullPath = m_previousLoadLocalItemLegacyFilePath + "library\\*";
	const char *pFoldername = g_pFullFileSystem->FindNext(m_previousLoadLocalItemLegacyFolderHandle);
	while (pFoldername != NULL)
	{
		if (!Q_strcmp(pFoldername, ".") || !Q_strcmp(pFoldername, ".."))
		{
			pFoldername = g_pFullFileSystem->FindNext(m_previousLoadLocalItemLegacyFolderHandle);
			continue;
		}

		m_previousLoadLocalItemLegacyFolderPath = VarArgs("library\\%s", pFoldername);
		if (!g_pFullFileSystem->FindIsDirectory(m_previousLoadLocalItemLegacyFolderHandle))
		{
			DevMsg("All items files must be within a subfolder!\n");
			pFoldername = g_pFullFileSystem->FindNext(m_previousLoadLocalItemLegacyFolderHandle);
			continue;
		}

		const char *pFilename = g_pFullFileSystem->FindFirst(VarArgs("%s%s\\*.itm", m_previousLoadLocalItemLegacyFilePath.c_str(), m_previousLoadLocalItemLegacyFolderPath.c_str()), &m_previousLoadLocalItemLegacyFileHandle);
		while (pFilename != NULL)
		{
			if (g_pFullFileSystem->FindIsDirectory(m_previousLoadLocalItemLegacyFileHandle))
			{
				pFilename = g_pFullFileSystem->FindNext(m_previousLoadLocalItemLegacyFileHandle);
				continue;
			}

			// WE'VE FOUND A FILE TO ATTEMPT TO LOAD!!!
			std::string foundName = VarArgs("%s\\%s", m_previousLoadLocalItemLegacyFolderPath.c_str(), pFilename);

			// MAKE THE FILE PATH NICE
			char path_buffer[AA_MAX_STRING];
			Q_strcpy(path_buffer, foundName.c_str());
			V_FixSlashes(path_buffer);
			foundName = path_buffer;
			// FINISHED MAKING THE FILE PATH NICE

			bool bIsModel;
			bool bWasAlreadyLoaded;
			pItem = this->LoadLocalItemLegacy(bIsModel, bWasAlreadyLoaded, foundName, m_previousLoadLocalItemLegacyFilePath, m_previousLocaLocalItemLegacyWorkshopIds, m_previousLoadLocalItemLegacyMountIds, m_pPreviousLoadLocalItemLegacyBackpack);
			if (pItem)
			{
				if (bIsModel)
					uMountWorkshopNumModels++;
				else
					uMountWorkshopNumItems++;

				if (!m_previousLoadLocalItemLegacyFastMode)
				{
					if (bIsModel)
					{
						if (m_previousLocaLocalItemLegacyWorkshopIds != "")
							pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Workshop Models", "workshoplibrarymodels", "", "", "+", "loadNextLocalItemLegacyCallback");
						else
							pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Importing Old AArcade Models", "oldlibrarymodels", "", "", "+", "loadNextLocalItemLegacyCallback");
					}
					else
					{
						if (m_previousLocaLocalItemLegacyWorkshopIds != "")
							pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Workshop Items", "workshoplibraryitems", "", "", "+", "loadNextLocalItemLegacyCallback");
						else
							pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Importing Old AArcade Items", "oldlibraryitems", "", "", "+", "loadNextLocalItemLegacyCallback");
					}
					return;
				}
			}

			pFilename = g_pFullFileSystem->FindNext(m_previousLoadLocalItemLegacyFileHandle);
		}

		g_pFullFileSystem->FindClose(m_previousLoadLocalItemLegacyFileHandle);
		pFoldername = g_pFullFileSystem->FindNext(m_previousLoadLocalItemLegacyFolderHandle);
		//break;
	}

	if (m_previousLoadLocalItemLegacyFastMode)
	{
		if (m_previousLocaLocalItemLegacyWorkshopIds != "")
		{
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Workshop Models", "workshoplibrarymodels", "", "", std::string(VarArgs("+%u", uMountWorkshopNumModels)));
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Workshop Items", "workshoplibraryitems", "", "", std::string(VarArgs("+%u", uMountWorkshopNumItems)));
		}
		else
		{
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Importing Old AArcade Models", "oldlibrarymodels", "", "", std::string(VarArgs("+%u", uMountWorkshopNumModels)));
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Importing Old AArcade Items", "oldlibraryitems", "", "", std::string(VarArgs("+%u", uMountWorkshopNumItems)));
		}
	}

	g_pAnarchyManager->GetWorkshopManager()->OnMountWorkshopSucceed();
	//g_pAnarchyManager->GetMetaverseManager()->OnMountWorkshopSucceed();
	this->LoadLocalItemLegacyClose();
	return;
}

void C_MetaverseManager::LoadLocalItemLegacyClose()
{
	if (m_previousLoadLocalItemLegacyFilePath != "")
	{
//		g_pFullFileSystem->FindClose(m_previousLoadLocalItemLegacyFolderHandle);

//		if (m_previousLoadLocalItemLegacyFolderPath != "")
//			g_pFullFileSystem->FindClose(m_previousLoadLocalItemLegacyFileHandle);
		m_previousLoadLocalItemLegacyFastMode = false;
		m_previousLoadLocalItemLegacyFilePath = "";
		m_previousLoadLocalItemLegacyFolderPath = "";
		m_previousLocaLocalItemLegacyWorkshopIds = "";
		m_previousLoadLocalItemLegacyMountIds = "";
		m_pPreviousLoadLocalItemLegacyBackpack = null;
//		m_previousLoadLocalItemsLegacyBuffer.clear();
	}
}

void C_MetaverseManager::AddApp(KeyValues* pAppKV)
{
	KeyValues* pActiveKV = this->GetActiveKeyValues(pAppKV);
	std::string id = pActiveKV->GetString("info/id");
	m_apps[id] = pAppKV;
}

void C_MetaverseManager::ResolveLoadLocalItemLegacyBuffer()
{
	// FIXME This is a huge bottleneck

	// This usually gets done after all  workshop mounts are done, but since this is an after-the-fact one, gotta do it again manually here
	std::string legacyModelId;
	unsigned int numResponses = m_previousLoadLocalItemsLegacyBuffer.size();
	unsigned int i;
	unsigned int j;
	KeyValues* pCompoundItemKV;
	KeyValues* active;
	unsigned int numVictims;
	std::vector<KeyValues*> victims;
	//KeyValues* model;
	//KeyValues* modelActive;
	for (i = 0; i < numResponses; i++)
	{
		pCompoundItemKV = m_previousLoadLocalItemsLegacyBuffer[i];

		// OSOLETE! Legacy models & items are no longer pre-resolved like this!
		//active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pCompoundItemKV);
		//std::string resolvedModel = g_pAnarchyManager->GenerateLegacyHash(pCompoundItemKV->GetString("lastmodel"));
		//active->SetString("model", resolvedModel.c_str());

		// legacy model (gets resolved upon map load)
		legacyModelId = pCompoundItemKV->GetString("lastmodel");

		// remove everything not in local or current or generation
		for (KeyValues *sub = pCompoundItemKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		{
			if (Q_strcmp(sub->GetName(), "local") && Q_strcmp(sub->GetName(), "generation"))
				victims.push_back(sub);
		}

		numVictims = victims.size();
		for (j = 0; j < numVictims; j++)
			pCompoundItemKV->RemoveSubKey(victims[j]);

		if (numVictims > 0)
			victims.clear();

		// the itemKV is no longer compound!  It has been stripped down to a GEN3 item now!
		KeyValues* pItemKV = pCompoundItemKV;
		KeyValues* pActiveKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pItemKV);

		//for (KeyValues *sub = pActiveKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		//	DevMsg("Key & Value: %s & %s\n", sub->GetName(), sub->GetString());

		std::string id = VarArgs("%s", pActiveKV->GetString("info/id"));

		KeyValues* pTestItemKV;
		KeyValues* pTestActiveKV;
		auto it = m_items.find(id);
		if (it != m_items.end())
		{
			pTestItemKV = it->second;
			pTestActiveKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pTestItemKV);

			// this is a legacy item, so give the right-of-way to any generation 3 item that was already found.
			if (pTestItemKV->GetInt("legacy") == 1)
			{
				// merg this legacy item with the other legacy item
				this->SmartMergItemKVs(pTestItemKV, pItemKV);
				pItemKV->deleteThis();
				pItemKV = it->second;

				//// FIXME: For now, just delete the old one and let this one overpower it.
				//it->second->deleteThis();
				//m_items.erase(it);
				//m_items[id] = pItem;
			}
			else
			{
				// let the generation 3 item overpower us
				pItemKV->deleteThis();
				pItemKV = it->second;
			}
		}
		else
			m_items[id] = pItemKV;
	}

	m_previousLoadLocalItemsLegacyBuffer.clear();
}

KeyValues* C_MetaverseManager::LoadLocalType(std::string file, std::string filePath)
{
	// TODO: IS THIS OBSOLETE?
	KeyValues* pType = new KeyValues("type");
	bool bLoaded;

	if (filePath != "")
	{
		std::string fullFile = filePath + file;
		bLoaded = pType->LoadFromFile(g_pFullFileSystem, fullFile.c_str(), "");
	}
	else
		bLoaded = pType->LoadFromFile(g_pFullFileSystem, file.c_str(), "MOD");

	if (!bLoaded)
	{
		pType->deleteThis();
		pType = null;
	}
	else
	{
		// TODO: Look up any alias here first!!
		KeyValues* pActive = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pType);
		std::string id = pActive->GetString("info/id");
		m_types[id] = pType;
	}

	return pType;
}

unsigned int C_MetaverseManager::LoadAllLocalTypes(sqlite3* pDb, std::map<std::string, KeyValues*>* pResponseMap)
{
	if (!pDb)
		pDb = m_db;
	// make it use the new shinnit
	unsigned int count = 0;
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(pDb, "SELECT * from types", -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed: %s\n", sqlite3_errmsg(pDb));

	int length;
	while (sqlite3_step(stmt) == SQLITE_ROW)	// THIS IS WHERE THE LOOP CAN BE BROKEN UP AT!!
	{
		length = sqlite3_column_bytes(stmt, 1);

		if (length == 0)
		{
			DevMsg("WARNING: Zero-byte KeyValues skipped.\n");
			continue;
		}

		KeyValues* pType = new KeyValues("type");

		CUtlBuffer buf(0, length, 0);
		buf.CopyBuffer(sqlite3_column_blob(stmt, 1), length);
		pType->ReadAsBinary(buf);
		buf.Purge();

		// TODO: Look up any alias here first!!
		KeyValues* pActive = this->GetActiveKeyValues(pType);
		std::string id = pActive->GetString("info/id");
		if (pResponseMap)
			(*pResponseMap)[id] = pType;
		else
			m_types[id] = pType;
		
		DevMsg("Loaded type %s from Redux library\n", pActive->GetString("title"));
		count++;
	}
	sqlite3_finalize(stmt);	// TODO: error checking?  Maybe not needed, if this is like a close() operation.
	return count;

	/*
	unsigned int count = 0;
	FileFindHandle_t testFileHandle;
	const char *pFilename = g_pFullFileSystem->FindFirst(VarArgs("%slibrary\\types\\*.key", filePath.c_str()), &testFileHandle);

	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(testFileHandle))
		{
			pFilename = g_pFullFileSystem->FindNext(testFileHandle);
			continue;
		}

		std::string foundName = pFilename;
		foundName = VarArgs("library\\types\\%s", pFilename);
		pFilename = g_pFullFileSystem->FindNext(testFileHandle);

		// MAKE THE FILE PATH NICE
		char path_buffer[AA_MAX_STRING];
		Q_strcpy(path_buffer, foundName.c_str());
		V_FixSlashes(path_buffer);

		for (int i = 0; path_buffer[i] != '\0'; i++)
			path_buffer[i] = tolower(path_buffer[i]);
		// FINISHED MAKING THE FILE PATH NICE

		foundName = path_buffer;
		if (this->LoadLocalType(foundName, filePath))
			count++;
	}

	g_pFullFileSystem->FindClose(testFileHandle);
	return count;
	*/
}

std::string C_MetaverseManager::ResolveLegacyType(std::string legacyType)
{
	if (legacyType == "")
		return AA_DEFAULT_TYPEID;

	// iterate through the types
	KeyValues* active;
	for (std::map<std::string, KeyValues*>::iterator it = m_types.begin(); it != m_types.end(); ++it)
	{
		active = this->GetActiveKeyValues(it->second);

		if (!Q_stricmp(active->GetString("title"), legacyType.c_str()))
			return it->first;
	}

	if (legacyType == "other" )
		return AA_DEFAULT_TYPEID;

	///*
	// If no type is found for this legacy type, create one!
	DevMsg("Creating a type w/ title %s because it didn't exist yet.\n", legacyType.c_str());
	KeyValues* pTypeKV = new KeyValues("type");

	// update us to 3rd generation
	pTypeKV->SetInt("generation", 3);

	// add standard info (except for id)
	pTypeKV->SetString("local/info/created", VarArgs("%llu", g_pAnarchyManager->GetTimeNumber()));
	pTypeKV->SetString("local/info/owner", "local");
	//pCurrent->SetInt("local/info/removed", 0);
	//pCurrent->SetString("local/info/remover", "");
	//pCurrent->SetString("local/info/alias", "");

	std::string typeId = g_pAnarchyManager->GenerateUniqueId();
	pTypeKV->SetString("local/info/id", typeId.c_str());

	//pCurrent->SetString("fileformat", "/.+$/i");
	//pCurrent->SetString("fileformat", "<AUTO>");	// will be processed after the apps are loaded so that file extensions can be added.
	//pCurrent->FindKey("fileformat", true);
	pTypeKV->SetString("local/titleformat", "/.+$/i");
	pTypeKV->SetString("local/title", legacyType.c_str());
	pTypeKV->SetInt("local/priority", 1);
	this->AddType(pTypeKV);
	this->SaveType(pTypeKV);	// Save to the user database, so AArcade doesnt' get confused over types when the user unsubscribes from this addon. (if they created other items using the type from this addon, for example.)
	
	return typeId;
	//*/
}

KeyValues* C_MetaverseManager::LoadLocalApp(std::string file, std::string filePath, std::string searchPath)
{
	KeyValues* pApp = new KeyValues("app");
	bool bLoaded;

//	if (filePath != "")
	//{
		std::string fullFile = filePath + file;
		bLoaded = pApp->LoadFromFile(g_pFullFileSystem, fullFile.c_str(), searchPath.c_str());
	//}
	//else
		//bLoaded = pApp->LoadFromFile(g_pFullFileSystem, file.c_str(), "MOD");

	if (!bLoaded)
	{
		pApp->deleteThis();
		pApp = null;
	}
	else
	{
		// TODO: Look up any alias here first!!
		KeyValues* pActive = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pApp);

		std::string id = pActive->GetString("info/id");
		m_apps[id] = pApp;
	}

	return pApp;
}

unsigned int C_MetaverseManager::LoadAllLocalInstances(sqlite3* pDb, std::map<std::string, KeyValues*>* pResponseMap)
{
	if (!pDb)
		pDb = m_db;
	// make it use the new shinnit
	unsigned int count = 0;
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(pDb, "SELECT * from instances", -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed: %s\n", sqlite3_errmsg(pDb));
	DevMsg("C_MetaverseManager::LoadAllLocalInstances\n");
	int length;
	std::string rowId;
	while (sqlite3_step(stmt) == SQLITE_ROW)	// THIS IS WHERE THE LOOP CAN BE BROKEN UP AT!!
	{
		rowId = std::string((const char*)sqlite3_column_text(stmt, 0));
		length = sqlite3_column_bytes(stmt, 1);

		if (length == 0)
		{
			DevMsg("WARNING: Zero-byte KeyValues skipped.\n");
			continue;
		}

		KeyValues* pInstance = new KeyValues("instance");

		CUtlBuffer buf(0, length, 0);
		buf.CopyBuffer(sqlite3_column_blob(stmt, 1), length);
		pInstance->ReadAsBinary(buf);
		buf.Purge();

		//for (KeyValues *sub = pInstance->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		//	DevMsg("%s is %s\n", sub->GetName(), sub->GetString());

		std::string instanceId = pInstance->GetString("info/local/id");
		if (instanceId != rowId)
		{
			DevMsg("Skipping ID conflicted instance: rowid(%s) vs instanceid(%s)\n", rowId.c_str(), instanceId.c_str());
			continue;
		}
		//DevMsg("Instance id is: %s vs %s\n", instanceId.c_str(), rowId.c_str());
		if (instanceId == "")
		{
			DevMsg("Skipping id-less instance...\n");
			continue;
		}

		if (pResponseMap)
			(*pResponseMap)[instanceId] = pInstance;	// FIXME: Make sure these get cleaned up for ALL LoadAllLocalInstances calls.
		else
		{
			int generation = pInstance->GetInt("generation", 3);
			int iLegacy = pInstance->GetInt("legacy", 0);
			//DevMsg("But the other is: %s\n", pInstance->GetString("info/id"));
			KeyValues* pInstanceInfoKV = pInstance->FindKey("info/local", true);
			//std::string instanceId = pInstanceInfoKV->GetString("id");
			std::string mapId = pInstanceInfoKV->GetString("map");
			std::string title = pInstanceInfoKV->GetString("title");
			if (title == "")
				title = "Unnamed (" + instanceId + ")";
			std::string file = "";
			std::string workshopIds = pInstanceInfoKV->GetString(VarArgs("platforms/%s/workshopIds", AA_PLATFORM_ID));
			std::string mountIds = pInstanceInfoKV->GetString(VarArgs("platforms/%s/mountIds", AA_PLATFORM_ID));
			//std::string backpackId = pInstanceInfoKV->GetString(VarArgs("platforms/%s/backpackIds", AA_PLATFORM_ID));
			std::string style = pInstanceInfoKV->GetString("style");
			pInstance->deleteThis();
			pInstance = null;

			g_pAnarchyManager->GetInstanceManager()->AddInstance(iLegacy, instanceId, mapId, title, file, workshopIds, mountIds, style);

			//m_instances[instanceId] = pInstance; //DevMsg("WARNING: No response map was given to LocalAllLocalInstances!\n");
		}
		count++;
	}
	sqlite3_finalize(stmt);	// TODO: error checking?  Maybe not needed, if this is like a close() operation.
	return count;
}

unsigned int C_MetaverseManager::LoadAllLocalApps(sqlite3* pDb, std::map<std::string, KeyValues*>* pResponseMap)
{
	if (!pDb)
		pDb = m_db;
	// make it use the new shinnit
	unsigned int count = 0;
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(pDb, "SELECT * from apps", -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed: %s\n", sqlite3_errmsg(pDb));

	int length;
	while (sqlite3_step(stmt) == SQLITE_ROW)	// THIS IS WHERE THE LOOP CAN BE BROKEN UP AT!!
	{
		length = sqlite3_column_bytes(stmt, 1);

		if (length == 0)
		{
			DevMsg("WARNING: Zero-byte KeyValues skipped.\n");
			continue;
		}

		KeyValues* pApp = new KeyValues("app");

		CUtlBuffer buf(0, length, 0);
		buf.CopyBuffer(sqlite3_column_blob(stmt, 1), length);
		pApp->ReadAsBinary(buf);
		buf.Purge();

		// TODO: Look up any alias here first!!
		KeyValues* pActive = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pApp);
		std::string id = pActive->GetString("info/id");

		if (pResponseMap)
			(*pResponseMap)[id] = pApp;
		else
			m_apps[id] = pApp;

		count++;
	}
	sqlite3_finalize(stmt);	// TODO: error checking?  Maybe not needed, if this is like a close() operation.
	return count;
}

std::string C_MetaverseManager::ResolveLegacyApp(std::string legacyApp)
{
	if (legacyApp == "")
		return "";

	// iterate through the types
	KeyValues* active;
	for (std::map<std::string, KeyValues*>::iterator it = m_apps.begin(); it != m_apps.end(); ++it)
	{
		active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(it->second);
		if (!Q_stricmp(active->GetString("title"), legacyApp.c_str()))
			return it->first;
	}

	return "";
}

KeyValues* C_MetaverseManager::LoadFirstLocalApp(std::string filePath)
{
	if (m_previousLoadLocalAppFilePath != "")
		this->LoadLocalAppClose();

	m_previousLoadLocalAppFilePath = filePath;
	const char *pFilename = g_pFullFileSystem->FindFirstEx("library\\apps\\*.key", filePath.c_str(), &m_previousLoadLocalAppFileHandle);
	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(m_previousLoadLocalAppFileHandle))
		{
			pFilename = g_pFullFileSystem->FindNext(m_previousLoadLocalAppFileHandle);
			continue;
		}

		std::string foundName = pFilename;
		foundName = VarArgs("library\\apps\\%s", pFilename);
		
		// MAKE THE FILE PATH NICE
		char path_buffer[AA_MAX_STRING];
		Q_strcpy(path_buffer, foundName.c_str());
		V_FixSlashes(path_buffer);

		for (int i = 0; path_buffer[i] != '\0'; i++)
			path_buffer[i] = tolower(path_buffer[i]);
		// FINISHED MAKING THE FILE PATH NICE

		foundName = path_buffer;
		return this->LoadLocalApp(foundName, "", m_previousLoadLocalAppFilePath);
	}

	return null;
}

KeyValues* C_MetaverseManager::LoadNextLocalApp()
{
//	if ( != "")
//		g_pFullFileSystem->FindClose(m_previousLoadLocalAppFileHandle);

	const char *pFilename = g_pFullFileSystem->FindNext(m_previousLoadLocalAppFileHandle);
	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(m_previousLoadLocalAppFileHandle))
		{
			pFilename = g_pFullFileSystem->FindNext(m_previousLoadLocalAppFileHandle);
			continue;
		}

		std::string foundName = pFilename;
		foundName = VarArgs("library\\apps\\%s", pFilename);

		// MAKE THE FILE PATH NICE
		char path_buffer[AA_MAX_STRING];
		Q_strcpy(path_buffer, foundName.c_str());
		V_FixSlashes(path_buffer);

		for (int i = 0; path_buffer[i] != '\0'; i++)
			path_buffer[i] = tolower(path_buffer[i]);
		// FINISHED MAKING THE FILE PATH NICE

		foundName = path_buffer;
		return this->LoadLocalApp(foundName, "", m_previousLoadLocalAppFilePath);
	}

	//g_pFullFileSystem->FindClose(m_previousLoadLocalAppFileHandle);
	//m_previousLoadLocalAppFilePath = "";
	return null;
}

void C_MetaverseManager::LoadLocalAppClose()
{
	if (m_previousLoadLocalAppFilePath != "")
	{
		g_pFullFileSystem->FindClose(m_previousLoadLocalAppFileHandle);
		m_previousLoadLocalAppFilePath = "";
	}
}

KeyValues* C_MetaverseManager::GetFirstLibraryType()
{
	m_previousGetTypeIterator = m_types.begin();
	if (m_previousGetTypeIterator != m_types.end())
		return m_previousGetTypeIterator->second;

	return null;
}

KeyValues* C_MetaverseManager::GetNextLibraryType()
{
	if (m_previousGetTypeIterator != m_types.end())
	{
		m_previousGetTypeIterator++;
		
		if (m_previousGetTypeIterator != m_types.end())
			return m_previousGetTypeIterator->second;
	}

	return null;
}
/*
KeyValues* C_MetaverseManager::GetFirstLibraryApp()
{
	m_previousGetAppIterator = m_apps.begin();
	if (m_previousGetAppIterator != m_apps.end())
		return m_previousGetAppIterator->second;

	return null;
}

KeyValues* C_MetaverseManager::GetNextLibraryApp()
{
	if (m_previousGetAppIterator != m_apps.end())
	{
		m_previousGetAppIterator++;

		if (m_previousGetAppIterator != m_apps.end())
			return m_previousGetAppIterator->second;
	}

	return null;
}
*/

/*
KeyValues* C_MetaverseManager::FindLibraryType(std::string term)
{
	// note: only finds exact matches.  IS case sensi
	auto it = m_types.begin();
	while (it != m_types.end())
	{
		if (it->first == term)
			return it->second;

		it++;
	}

	return null;
}
*/

KeyValues* C_MetaverseManager::GetLibraryType(std::string id)
{
	std::map<std::string, KeyValues*>::iterator it = m_types.find(id);
	if (it != m_types.end())
		return it->second;
	else
		return null;
}

std::string C_MetaverseManager::GetSpecialTypeId(std::string typeTitle)
{
	KeyValues* pTypeKV;
	std::map<std::string, KeyValues*>::iterator it = m_types.begin();// find(id);
	while (it != m_types.end())
	{
		pTypeKV = this->GetActiveKeyValues(it->second);
		if (pTypeKV && std::string(pTypeKV->GetString("title")) == typeTitle)
			return std::string(pTypeKV->GetString("info/id"));

		it++;
	}

	// FUTURE-PROOF WAY OF FINDING IMPORTANT SHIT:
	// If we can't find it, add it to the library.

	pTypeKV = new KeyValues("type");
	if (pTypeKV->LoadFromFile(g_pFullFileSystem, VarArgs("defaultLibrary\\types\\%s.txt", typeTitle.c_str()), "MOD"))
	{
		KeyValues* active = this->GetActiveKeyValues(pTypeKV);
		this->SaveSQL(null, "types", active->GetString("info/id"), pTypeKV);
		this->AddType(pTypeKV);

		return std::string(active->GetString("info/id"));
	}

	DevMsg("WARNING: Could NOT find special type with title %s\n", typeTitle.c_str());
	return "";
}

std::string C_MetaverseManager::GetSpecialModelId(std::string modelType)
{
	std::string file = "";
	if (modelType == "node")
		file = "models\\cabinets\\node.mdl";

	KeyValues* pSearchInfo = new KeyValues("search");
	pSearchInfo->SetString("file", file.c_str());

	KeyValues* pModelKV = this->GetActiveKeyValues(this->FindLibraryModel(pSearchInfo));
	if (pModelKV)
		return std::string(pModelKV->GetString("info/id"));


	// FUTURE-PROOF WAY OF FINDING IMPORTANT SHIT:
	// If we can't find it, add it to the library.

	KeyValues* pCabinetKV = new KeyValues("model");
	if (pCabinetKV->LoadFromFile(g_pFullFileSystem, VarArgs("defaultLibrary\\cabinets\\%s.txt", modelType.c_str()), "MOD"))
	{
		KeyValues* active = this->GetActiveKeyValues(pCabinetKV);
		this->SaveSQL(null, "models", active->GetString("info/id"), pCabinetKV);
		this->AddModel(pCabinetKV);

		return std::string(active->GetString("info/id"));
	}

	DevMsg("WARNING: Could NOT find special model with file %s\n", file.c_str());
	return "";
}

std::string C_MetaverseManager::GetFirstLibraryEntry(KeyValues*& response, const char* category)//const char* 
{
	//char queryId[20];
	//const char* queryId = g_pAnarchyManager->GenerateUniqueId();
	//g_pAnarchyManager->GenerateUniqueId(queryId);

	//const char*
	std::string queryId = g_pAnarchyManager->GenerateUniqueId();
	DevMsg("Here id is: %s\n", queryId.c_str());

	//std::string idBuf = std::string(queryId);

	// TODO: use queryId to organize N search queries & store the category info for each.
	//*
	if (!Q_strcmp(category, "items"))
	{
		m_previousGetItemIterator = m_items.begin();
		response = (m_previousGetItemIterator != m_items.end()) ? m_previousGetItemIterator->second : null;
	}
	else if(!Q_strcmp(category, "models"))
	{
		m_previousGetModelIterator = m_models.begin();
		response = (m_previousGetModelIterator != m_models.end()) ? m_previousGetModelIterator->second : null;
	}
	else if(!Q_strcmp(category, "apps"))
	{
		m_previousGetAppIterator = m_apps.begin();
		response = (m_previousGetAppIterator != m_apps.end()) ? m_previousGetAppIterator->second : null;
	}
	//*/

	/*
	auto categoryEntries = (!Q_strcmp(category, "items")) ? m_items : m_models;
	auto it = (!Q_strcmp(category, "items")) ? m_previousGetItemIterator : m_previousGetModelIterator;

	it = categoryEntries.begin();

	if (it != categoryEntries.end())
		response = it->second;
	else
		response = null;
	*/

	return queryId;
}
KeyValues* C_MetaverseManager::GetNextLibraryEntry(const char* queryId, const char* category)
{
	///*
	//DevMsg("AAaaannnd here it is: %s and %s\n", queryId, category);
	if (!Q_strcmp(category, "items"))
	{
		//DevMsg("Has it: %i\n", m_previousGetItemIterator);
		if (m_previousGetItemIterator != m_items.end())
			m_previousGetItemIterator++;

		return (m_previousGetItemIterator != m_items.end()) ? m_previousGetItemIterator->second : null;
	}
	else if(!Q_strcmp(category, "models"))
	{
		if (m_previousGetModelIterator != m_models.end())
			m_previousGetModelIterator++;

		return (m_previousGetModelIterator != m_models.end()) ? m_previousGetModelIterator->second : null;
	}
	else if (!Q_strcmp(category, "apps"))
	{
		if (m_previousGetAppIterator != m_models.end())
			m_previousGetAppIterator++;

		return (m_previousGetAppIterator != m_apps.end()) ? m_previousGetAppIterator->second : null;
	}
	//*/

	/*
	// TODO: use queryId to organize N search queries & store the category info for each.
	auto categoryEntries = (!Q_strcmp(category, "items")) ? m_items : m_models;
	auto it = (!Q_strcmp(category, "items")) ? m_previousGetItemIterator : m_previousGetModelIterator;

	if (it != categoryEntries.end())
	{
		it++;

		if (it != categoryEntries.end())
			return it->second;
	}
	*/

	return null;
}

// LEGACY! OBSOLETE!!
KeyValues* C_MetaverseManager::GetFirstLibraryItem()
{
	m_previousGetItemIterator = m_items.begin();
	if (m_previousGetItemIterator != m_items.end())
		return m_previousGetItemIterator->second;

	return null;
}

KeyValues* C_MetaverseManager::GetNextLibraryItem()
{
	if (m_previousGetItemIterator != m_items.end())
	{
		m_previousGetItemIterator++;

		if (m_previousGetItemIterator != m_items.end())
			return m_previousGetItemIterator->second;
	}

	return null;
}

KeyValues* C_MetaverseManager::GetFirstLibraryModel()
{
	m_previousGetModelIterator = m_models.begin();
	if (m_previousGetModelIterator != m_models.end())
		return m_previousGetModelIterator->second;

	return null;
}

KeyValues* C_MetaverseManager::GetNextLibraryModel()
{
	if (m_previousGetModelIterator != m_models.end())
	{
		m_previousGetModelIterator++;

		if (m_previousGetModelIterator != m_models.end())
			return m_previousGetModelIterator->second;
	}

	return null;
}

KeyValues* C_MetaverseManager::GetFirstLibraryApp()
{
	m_previousGetAppIterator = m_apps.begin();
	if (m_previousGetAppIterator != m_apps.end())
		return m_previousGetAppIterator->second;

	return null;
}

KeyValues* C_MetaverseManager::GetNextLibraryApp()
{
	if (m_previousGetAppIterator != m_apps.end())
	{
		m_previousGetAppIterator++;

		if (m_previousGetAppIterator != m_apps.end())
			return m_previousGetAppIterator->second;
	}

	return null;
}

KeyValues* C_MetaverseManager::GetLibraryItem(std::string id)
{
	std::map<std::string, KeyValues*>::iterator it = m_items.find(id);
	if (it != m_items.end())
		return it->second;
	else
		return null;
}

KeyValues* C_MetaverseManager::GetLibraryModel(std::string id)
{
	std::map<std::string, KeyValues*>::iterator it = m_models.find(id);
	if (it != m_models.end())
		return it->second;
	else
		return null;
}

KeyValues* C_MetaverseManager::FindLibraryModel(KeyValues* pSearchInfo, std::map<std::string, KeyValues*>::iterator& it)
{
	KeyValues* potential;
	KeyValues* active;
	KeyValues* searchField;
	std::string fieldName, potentialBuf, searchBuf;
	char charBuf[AA_MAX_STRING];
	std::vector<std::string> searchTokens;
	//unsigned int i, numTokens;
	bool bFoundMatch;
	while (it != m_models.end())
	{
		bFoundMatch = false;
		potential = it->second;
		active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(potential);
		// active has the potential model data
		// pSearchInfo has the search criteria
		bool bGood = false;
		for (searchField = pSearchInfo->GetFirstSubKey(); searchField; searchField = searchField->GetNextKey())
		{
			fieldName = searchField->GetName();
			if (fieldName == "title")
			{
				if (!Q_strcmp(searchField->GetString(), ""))
					bGood = true;
				else
				{
					potentialBuf = active->GetString("title");
					std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

					searchBuf = searchField->GetString();
					std::transform(searchBuf.begin(), searchBuf.end(), searchBuf.begin(), ::tolower);

					/*
					searchTokens.clear();
					g_pAnarchyManager->Tokenize(searchBuf, searchTokens, " ");
					numTokens = searchTokens.size();

					for (i = 0; i < numTokens; i++)
					{
					if (searchTokens[i].length() < 2)
					continue;

					if (potentialBuf.find(searchTokens[i]) != std::string::npos)
					{
					bFoundMatch = true;
					break;
					}
					}
					*/

					if (potentialBuf.find(searchBuf) != std::string::npos)
						bGood = true;
					else
						bGood = false;
				}
			}
			else if (fieldName == "dynamic")
			{
				if (!Q_strcmp(searchField->GetString(), "") || !Q_strcmp(active->GetString("dynamic"), searchField->GetString()))
					bGood = true;
				else
					bGood = false;
			}

			if (!bGood)
				break;
		}

		if (bGood)
		{
			bFoundMatch = true;
			break;
		}

		if (bFoundMatch)
			break;
		else
			it++;
	}

	if (bFoundMatch)
		return it->second;
	else
		return null;
}

KeyValues* C_MetaverseManager::FindLibraryModel(KeyValues* pSearchInfo)
{
	KeyValues* potential;
	KeyValues* active;
	KeyValues* searchField;
	std::string fieldName, potentialBuf, searchBuf;
	char charBuf[AA_MAX_STRING];
	std::vector<std::string> searchTokens;
	unsigned int i, numTokens;
	bool bFoundMatch;
	std::map<std::string, KeyValues*>::iterator it = m_models.begin();
	while (it != m_models.end())
	{
		bFoundMatch = false;
		potential = it->second;
		active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(potential);
		// active has the potential model data
		// pSearchInfo has the search criteria
		for (searchField = pSearchInfo->GetFirstSubKey(); searchField; searchField = searchField->GetNextKey())
		{
			fieldName = searchField->GetName();
			if (fieldName == "title")
			{
				potentialBuf = searchField->GetString();	// FIXME: Does this work? seems to work... but how?
				std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

				searchBuf = pSearchInfo->GetString(fieldName.c_str());
				searchTokens.clear();
				g_pAnarchyManager->Tokenize(searchBuf, searchTokens, ", ");
				numTokens = searchTokens.size();

				for (i = 0; i < numTokens; i++)
				{
					if (searchTokens[i].length() < 2)
						continue;

					if (potentialBuf.find(searchTokens[i]) != std::string::npos)
					{
						bFoundMatch = true;
						break;
					}
				}

				if (bFoundMatch)
					break;
			}
			else if (fieldName == "file")
			{
				potentialBuf = active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));
				std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);
				std::replace(potentialBuf.begin(), potentialBuf.end(), '/', '\\'); // replace all '/' to '\'

				searchBuf = searchField->GetString();

				// TODO: Make these changes be required to the value prior to calling find.
				std::transform(searchBuf.begin(), searchBuf.end(), searchBuf.begin(), ::tolower);
				std::replace(searchBuf.begin(), searchBuf.end(), '/', '\\'); // replace all '/' to '\'

				if (potentialBuf == searchBuf)
				{
					//	DevMsg("Found match with %s = %s\n", potentialBuf.c_str(), searchBuf.c_str());
					bFoundMatch = true;
					break;
				}
			}
			else
			{
				potentialBuf = active->GetString(fieldName.c_str());
				std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

				searchBuf = searchField->GetString();
				std::transform(searchBuf.begin(), searchBuf.end(), searchBuf.begin(), ::tolower);

				if (potentialBuf == searchBuf)
				{
					//	DevMsg("Found match with %s = %s\n", potentialBuf.c_str(), searchBuf.c_str());
					bFoundMatch = true;
					break;
				}
			}
		}

		if (bFoundMatch)
			break;
		else
			it++;
	}

	// Do this here because we MUST handle the deletion for findNext and findFirst, so this usage should match!!
	pSearchInfo->deleteThis();

	if (bFoundMatch)
		return it->second;
	else
		return null;
}

KeyValues* C_MetaverseManager::FindLibraryApp(KeyValues* pSearchInfo, std::map<std::string, KeyValues*>::iterator& it)
{
	KeyValues* potential;
	KeyValues* active;
	KeyValues* searchField;
	std::string fieldName, potentialBuf, searchBuf;
	char charBuf[AA_MAX_STRING];
	std::vector<std::string> searchTokens;
	//unsigned int i, numTokens;
	bool bFoundMatch;
	while (it != m_apps.end())
	{
		bFoundMatch = false;
		potential = it->second;
		active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(potential);
		// active has the potential app data
		// pSearchInfo has the search criteria
		bool bGood = false;
		for (searchField = pSearchInfo->GetFirstSubKey(); searchField; searchField = searchField->GetNextKey())
		{
			fieldName = searchField->GetName();
			if (fieldName == "title")
			{
				if (!Q_strcmp(searchField->GetString(), ""))
					bGood = true;
				else
				{
					potentialBuf = active->GetString("title");
					std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

					searchBuf = searchField->GetString();
					std::transform(searchBuf.begin(), searchBuf.end(), searchBuf.begin(), ::tolower);

					/*
					searchTokens.clear();
					g_pAnarchyManager->Tokenize(searchBuf, searchTokens, " ");
					numTokens = searchTokens.size();

					for (i = 0; i < numTokens; i++)
					{
					if (searchTokens[i].length() < 2)
					continue;

					if (potentialBuf.find(searchTokens[i]) != std::string::npos)
					{
					bFoundMatch = true;
					break;
					}
					}
					*/

					if (potentialBuf.find(searchBuf) != std::string::npos)
						bGood = true;
					else
						bGood = false;
				}
			}
			else
			{
				potentialBuf = active->GetString(fieldName.c_str());
				std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

				searchBuf = searchField->GetString();
				std::transform(searchBuf.begin(), searchBuf.end(), searchBuf.begin(), ::tolower);

				if (potentialBuf == searchBuf)
					bGood = true;
				else
					bGood = false;
			}
			/*
			else if (fieldName == "type")
			{
			if (!Q_strcmp(searchField->GetString(), "") || !Q_strcmp(active->GetString("type"), searchField->GetString()))
			bGood = true;
			else
			bGood = false;
			}
			*/

			if (!bGood)
				break;
		}

		if (bGood)
		{
			bFoundMatch = true;
			break;
		}

		if (bFoundMatch)
			break;
		else
			it++;
	}

	if (bFoundMatch)
		return it->second;
	else
		return null;
}

KeyValues* C_MetaverseManager::FindLibraryApp(KeyValues* pSearchInfo)
{
	//DevMsg("C_MetaverseManager: FindLibraryApp with ONLY pSearchinfo!!\n");
	KeyValues* potential;
	KeyValues* active;
	KeyValues* searchField;
	std::string fieldName, potentialBuf, searchBuf;
	char charBuf[AA_MAX_STRING];
	std::vector<std::string> searchTokens;
	unsigned int i, numTokens;
	bool bFoundMatch = false;
	std::map<std::string, KeyValues*>::iterator it = m_apps.begin();
	while (it != m_apps.end())
	{
		bFoundMatch = false;
		potential = it->second;
		active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(potential);
		// active has the potential app data
		// pSearchInfo has the search criteria
		for (searchField = pSearchInfo->GetFirstSubKey(); searchField; searchField = searchField->GetNextKey())
		{
			fieldName = searchField->GetName();
			/*
			if (fieldName == "title")
			{
				potentialBuf = searchField->GetString();	// FIXME: Does this work? seems to work... but how?
				std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

				searchBuf = pSearchInfo->GetString(fieldName.c_str());
				searchTokens.clear();
				g_pAnarchyManager->Tokenize(searchBuf, searchTokens, ", ");
				numTokens = searchTokens.size();

				for (i = 0; i < numTokens; i++)
				{
					if (searchTokens[i].length() < 2)
						continue;

					if (potentialBuf.find(searchTokens[i]) != std::string::npos)
					{
						bFoundMatch = true;
						break;
					}
				}

				if (bFoundMatch)
					break;
			}
			else
			{*/
				potentialBuf = active->GetString(fieldName.c_str());
				std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

				searchBuf = searchField->GetString();
				std::transform(searchBuf.begin(), searchBuf.end(), searchBuf.begin(), ::tolower);

				if (potentialBuf == searchBuf)
				{
					//	DevMsg("Found match with %s = %s\n", potentialBuf.c_str(), searchBuf.c_str());
					bFoundMatch = true;
					break;
				}
			//}
		}

		if (bFoundMatch)
			break;
		else
			it++;
	}

	// Do this here because we MUST handle the deletion for findNext and findFirst, so this usage should match!!
	pSearchInfo->deleteThis();

	if (bFoundMatch)
		return it->second;
	else
		return null;
}

KeyValues* C_MetaverseManager::FindLibraryType(KeyValues* pSearchInfo, std::map<std::string, KeyValues*>::iterator& it)
{
	KeyValues* potential;
	KeyValues* active;
	KeyValues* searchField;
	std::string fieldName, potentialBuf, searchBuf;
	char charBuf[AA_MAX_STRING];
	std::vector<std::string> searchTokens;
	//unsigned int i, numTokens;
	bool bFoundMatch;
	while (it != m_types.end())
	{
		bFoundMatch = false;
		potential = it->second;
		active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(potential);

		// active has the potential type data
		// pSearchInfo has the search criteria
		bool bGood = false;
		for (searchField = pSearchInfo->GetFirstSubKey(); searchField; searchField = searchField->GetNextKey())
		{
			fieldName = searchField->GetName();
			if (fieldName == "title")
			{
				if (!Q_strcmp(searchField->GetString(), ""))
					bGood = true;
				else
				{
					potentialBuf = active->GetString("title");
					std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

					searchBuf = searchField->GetString();
					std::transform(searchBuf.begin(), searchBuf.end(), searchBuf.begin(), ::tolower);

					/*
					searchTokens.clear();
					g_pAnarchyManager->Tokenize(searchBuf, searchTokens, " ");
					numTokens = searchTokens.size();

					for (i = 0; i < numTokens; i++)
					{
					if (searchTokens[i].length() < 2)
					continue;

					if (potentialBuf.find(searchTokens[i]) != std::string::npos)
					{
					bFoundMatch = true;
					break;
					}
					}
					*/

					if (potentialBuf.find(searchBuf) != std::string::npos)
						bGood = true;
					else
						bGood = false;
				}
			}
			else
			{
				potentialBuf = active->GetString(fieldName.c_str());
				std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

				searchBuf = searchField->GetString();
				std::transform(searchBuf.begin(), searchBuf.end(), searchBuf.begin(), ::tolower);

				if (potentialBuf == searchBuf)
					bGood = true;
				else
					bGood = false;
			}

			if (!bGood)
				break;
		}

		if (bGood)
		{
			bFoundMatch = true;
			break;
		}

		if (bFoundMatch)
			break;
		else
			it++;
	}

	if (bFoundMatch)
		return it->second;
	else
		return null;
}

KeyValues* C_MetaverseManager::FindLibraryType(KeyValues* pSearchInfo)
{
	//DevMsg("C_MetaverseManager: FindLibraryType with ONLY pSearchinfo!!\n");
	KeyValues* potential;
	KeyValues* active;
	KeyValues* searchField;
	std::string fieldName, potentialBuf, searchBuf;
	char charBuf[AA_MAX_STRING];
	std::vector<std::string> searchTokens;
	unsigned int i, numTokens;
	bool bFoundMatch = false;
	std::map<std::string, KeyValues*>::iterator it = m_types.begin();
	while (it != m_types.end())
	{
		bFoundMatch = false;
		potential = it->second;
		active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(potential);
		// active has the potential type data
		// pSearchInfo has the search criteria
		for (searchField = pSearchInfo->GetFirstSubKey(); searchField; searchField = searchField->GetNextKey())
		{
			fieldName = searchField->GetName();
			/*
			if (fieldName == "title")
			{
			potentialBuf = searchField->GetString();	// FIXME: Does this work? seems to work... but how?
			std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

			searchBuf = pSearchInfo->GetString(fieldName.c_str());
			searchTokens.clear();
			g_pAnarchyManager->Tokenize(searchBuf, searchTokens, ", ");
			numTokens = searchTokens.size();

			for (i = 0; i < numTokens; i++)
			{
			if (searchTokens[i].length() < 2)
			continue;

			if (potentialBuf.find(searchTokens[i]) != std::string::npos)
			{
			bFoundMatch = true;
			break;
			}
			}

			if (bFoundMatch)
			break;
			}
			else
			{*/
			potentialBuf = active->GetString(fieldName.c_str());
			std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

			searchBuf = searchField->GetString();
			std::transform(searchBuf.begin(), searchBuf.end(), searchBuf.begin(), ::tolower);

			if (potentialBuf == searchBuf)
			{
				//	DevMsg("Found match with %s = %s\n", potentialBuf.c_str(), searchBuf.c_str());
				bFoundMatch = true;
				break;
			}
			//}
		}

		if (bFoundMatch)
			break;
		else
			it++;
	}

	// Do this here because we MUST handle the deletion for findNext and findFirst, so this usage should match!!
	pSearchInfo->deleteThis();

	if (bFoundMatch)
		return it->second;
	else
		return null;
}

KeyValues* C_MetaverseManager::FindFirstLibraryModel(KeyValues* pSearchInfo)
{
	// remember this search query
	if (!m_pPreviousModelSearchInfo)
		m_pPreviousModelSearchInfo = pSearchInfo;// new KeyValues("search");
	else if (m_pPreviousModelSearchInfo != pSearchInfo)	// this should never be called!!!
	{
		m_pPreviousModelSearchInfo->deleteThis();
		m_pPreviousModelSearchInfo = pSearchInfo;
	}

	m_previousFindModelIterator = m_models.begin();

	// start the search
	KeyValues* response = this->FindLibraryModel(m_pPreviousModelSearchInfo, m_previousFindModelIterator);
	return response;
}

KeyValues* C_MetaverseManager::FindFirstLibraryApp(KeyValues* pSearchInfo)
{
	// remember this search query
	if (!m_pPreviousAppSearchInfo)
		m_pPreviousAppSearchInfo = pSearchInfo;// new KeyValues("search");
	else if (m_pPreviousAppSearchInfo != pSearchInfo)	// this should never be called!!!
	{
		m_pPreviousAppSearchInfo->deleteThis();
		m_pPreviousAppSearchInfo = pSearchInfo;
	}

	m_previousFindAppIterator = m_apps.begin();

	// start the search
	KeyValues* response = this->FindLibraryApp(m_pPreviousAppSearchInfo, m_previousFindAppIterator);
	return response;
}

KeyValues* C_MetaverseManager::FindNextLibraryModel()
{
	// continue the search
	KeyValues* response = null;
	m_previousFindModelIterator++;
	if (m_previousFindModelIterator != m_models.end())
		response = this->FindLibraryModel(m_pPreviousModelSearchInfo, m_previousFindModelIterator);
	return response;
}

KeyValues* C_MetaverseManager::FindNextLibraryApp()
{
	// continue the search
	KeyValues* response = null;
	m_previousFindAppIterator++;
	if (m_previousFindAppIterator != m_apps.end())
		response = this->FindLibraryApp(m_pPreviousAppSearchInfo, m_previousFindAppIterator);
	return response;
}

/*
KeyValues* C_MetaverseManager::FindLibraryModel(KeyValues* pSearchInfo, bool bExactOnly)
{
	KeyValues* potential;
	KeyValues* active;
	KeyValues* searchField;
	std::string fieldName, potentialBuf, searchBuf;
	char charBuf[AA_MAX_STRING];
	std::vector<std::string> searchTokens;
	unsigned int i, numTokens;
	bool bFoundMatch;
	std::map<std::string, KeyValues*>::iterator it = m_models.begin();
	char slashBuffer[AA_MAX_STRING];
	while (it != m_models.end())
	{
		bFoundMatch = false;
		potential = it->second;
		active = potential->FindKey("current");
		if (!active)
			active = potential->FindKey("local");
		if (active)
		{
			// active has the potential model data
			// pSearchInfo has the search criteria
			for (searchField = pSearchInfo->GetFirstSubKey(); searchField; searchField = searchField->GetNextKey())
			{
				fieldName = searchField->GetName();
				if (fieldName == "lastmodel")
				{
					if (!bExactOnly)
					{
						potentialBuf = searchField->GetString();
						std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

						searchBuf = pSearchInfo->GetString(fieldName.c_str());
						searchTokens.clear();
						g_pAnarchyManager->Tokenize(searchBuf, searchTokens, ", ");
						numTokens = searchTokens.size();

						for (i = 0; i < numTokens; i++)
						{
							if (searchTokens[i].length() < 2)
								continue;

							if (potentialBuf.find(searchTokens[i]) != std::string::npos)
							{
								bFoundMatch = true;
								break;
							}
						}
					}
					else
					{
						// fix slashes to reduce duplicates
						Q_strcpy(slashBuffer, searchField->GetString());
						V_FixSlashes(slashBuffer);
						potentialBuf = slashBuffer;

						Q_strcpy(slashBuffer, pSearchInfo->GetString(fieldName.c_str()));
						V_FixSlashes(slashBuffer);
						searchBuf = slashBuffer;

						if (potentialBuf == searchBuf)
							bFoundMatch = true;
					}

					if (bFoundMatch)
						break;

				}
			}
		}

		if (bFoundMatch)
			break;
		else
			it++;
	}

	// Do this here because we MUST handle the deletion for findNext and findFirst, so this usage should match!!
	pSearchInfo->deleteThis();

	if (bFoundMatch)
		return it->second;
	else
		return null;
}
*/

KeyValues* C_MetaverseManager::FindLibraryEntry(const char* category, KeyValues* pSearchInfo, std::map<std::string, KeyValues*>::iterator& it)
{
	KeyValues* potential;
	KeyValues* active;
	KeyValues* searchField;
	std::string fieldName, potentialBuf, searchBuf;
	char charBuf[AA_MAX_STRING];
	std::vector<std::string> searchTokens;
	//unsigned int i, numTokens;
	bool bFoundMatch;

	// Determine if we are dealing with type = nodes
	bool bIsNodeTypeSearch = false;
	KeyValues* pTypeKV = this->GetActiveKeyValues(this->GetLibraryType(pSearchInfo->GetString("type")));
	if (pTypeKV && !Q_strcmp(pTypeKV->GetString("title"), "node"))
		bIsNodeTypeSearch = true;

	instance_t* pNodeInstance;
	std::string nodeInstanceId;
	std::string testNodeStyle = pSearchInfo->GetString("nodestyle");

	std::map<std::string, KeyValues*>* pCategoryEntries;// = (!Q_strcmp(category, "items")) ? &m_items : &m_models;
	if (!Q_strcmp(category, "items"))
		pCategoryEntries = &m_items;
	else if (!Q_strcmp(category, "models"))
		pCategoryEntries = &m_models;
	else if (!Q_strcmp(category, "apps"))
		pCategoryEntries = &m_apps;
	
	while (it != pCategoryEntries->end())
	{
		bFoundMatch = false;
		active = this->GetActiveKeyValues(it->second);
		if (active)
		{
			// active has the potential item data
			// pSearchInfo has the search criteria
			bool bGood = false;
			for (searchField = pSearchInfo->GetFirstSubKey(); searchField; searchField = searchField->GetNextKey())
			{
				fieldName = searchField->GetName();
				if (fieldName == "nodestyle")	// skip this psuedo search field
					continue;
				else if (fieldName == "title")
				{
					if (!Q_strcmp(searchField->GetString(), ""))
						bGood = true;
					else
					{
						potentialBuf = active->GetString("title");
						std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

						searchBuf = searchField->GetString();
						std::transform(searchBuf.begin(), searchBuf.end(), searchBuf.begin(), ::tolower);

						/*
						searchTokens.clear();
						g_pAnarchyManager->Tokenize(searchBuf, searchTokens, " ");
						numTokens = searchTokens.size();

						for (i = 0; i < numTokens; i++)
						{
						if (searchTokens[i].length() < 2)
						continue;

						if (potentialBuf.find(searchTokens[i]) != std::string::npos)
						{
						bFoundMatch = true;
						break;
						}
						}
						*/

						if (potentialBuf.find(searchBuf) != std::string::npos)
							bGood = true;
						else
							bGood = false;
					}
				}
				/*
				else if (categoryEntries == m_models && fieldName == "dynamic")
				{
					if (!Q_strcmp(searchField->GetString(), "") || !Q_strcmp(active->GetString("dynamic"), searchField->GetString()))
						bGood = true;
					else
						bGood = false;
				}*/
				else
				{
					searchBuf = searchField->GetString();

					if (pCategoryEntries == &m_models && fieldName == "file")
					{
						potentialBuf = active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));
						std::replace(potentialBuf.begin(), potentialBuf.end(), '/', '\\');

						//std::replace(searchBuf.begin(), searchBuf.end(), '/', '\\');	// now done prior to calling us. (fixed.)	// FIXME: This really only needs to be done once! (It's doing it EVERY compare right now.)
					}
					else
						potentialBuf = active->GetString(fieldName.c_str());

					std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);
					//std::transform(searchBuf.begin(), searchBuf.end(), searchBuf.begin(), ::tolower);	// done prior to calling us

					if (potentialBuf == searchBuf)
						bGood = true;
					else
						bGood = false;
				}

				if (!bGood)
					break;
			}

			if (bGood)
			{
				// validate node search results
				if (bIsNodeTypeSearch)
				{
					if (testNodeStyle == "")
					{
						// if we are NOT given a nodestyle, then we are ONLY good if we are of node_walls or node_floors
						nodeInstanceId = active->GetString("file");
						//DevMsg("Node isntance id is: ")
						pNodeInstance = g_pAnarchyManager->GetInstanceManager()->GetInstance(nodeInstanceId);
						if (!pNodeInstance)
							bGood = false;
						else
						{
							//testNodeStyle = pNodeInstance->style;

							//if (!pNodeInstance || pNodeInstance->style != testNodeStyle)
							if (!pNodeInstance || (pNodeInstance->style != "node_smallwall" && pNodeInstance->style != "node_floor3x4"))
								bGood = false;
						}
					}
					else
					{
						// confirm our match is also the right nodestyle
						nodeInstanceId = active->GetString("file");
						pNodeInstance = g_pAnarchyManager->GetInstanceManager()->GetInstance(nodeInstanceId);
						if (!pNodeInstance || pNodeInstance->style != testNodeStyle)
							bGood = false;
					}
				}

				if (bGood)
				{
					bFoundMatch = true;
					break;
				}
			}
		}

		if (bFoundMatch)
			break;
		else
			it++;
	}

	if (bFoundMatch)
		return it->second;
	else
		return null;
}

KeyValues* C_MetaverseManager::FindLibraryItem(KeyValues* pSearchInfo, std::map<std::string, KeyValues*>::iterator& it)
{
	KeyValues* potential;
	KeyValues* active;
	KeyValues* searchField;
	std::string fieldName, potentialBuf, searchBuf;
	char charBuf[AA_MAX_STRING];
	std::vector<std::string> searchTokens;
	//unsigned int i, numTokens;
	bool bFoundMatch;
	while (it != m_items.end())
	{
		bFoundMatch = false;
		potential = it->second;
		active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(potential);
		// active has the potential item data
		// pSearchInfo has the search criteria
		bool bGood = false;
		for (searchField = pSearchInfo->GetFirstSubKey(); searchField; searchField = searchField->GetNextKey())
		{
			fieldName = searchField->GetName();
			if (fieldName == "title")
			{
				if (!Q_strcmp(searchField->GetString(), ""))
					bGood = true;
				else
				{
					potentialBuf = active->GetString("title");
					std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

					searchBuf = searchField->GetString();
					std::transform(searchBuf.begin(), searchBuf.end(), searchBuf.begin(), ::tolower);

					/*
					searchTokens.clear();
					g_pAnarchyManager->Tokenize(searchBuf, searchTokens, " ");
					numTokens = searchTokens.size();

					for (i = 0; i < numTokens; i++)
					{
					if (searchTokens[i].length() < 2)
					continue;

					if (potentialBuf.find(searchTokens[i]) != std::string::npos)
					{
					bFoundMatch = true;
					break;
					}
					}
					*/

					if (potentialBuf.find(searchBuf) != std::string::npos)
						bGood = true;
					else
						bGood = false;
				}
			}
			else
			{
				potentialBuf = active->GetString(fieldName.c_str());
				std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

				searchBuf = searchField->GetString();
				std::transform(searchBuf.begin(), searchBuf.end(), searchBuf.begin(), ::tolower);

				if (potentialBuf == searchBuf)
					bGood = true;
				else
					bGood = false;
			}
			/*
			else if (fieldName == "type")
			{
				if (!Q_strcmp(searchField->GetString(), "") || !Q_strcmp(active->GetString("type"), searchField->GetString()))
					bGood = true;
				else
					bGood = false;
			}
			*/

			if (!bGood)
				break;
		}

		if (bGood)
		{
			bFoundMatch = true;
			break;
		}

		if (bFoundMatch)
			break;
		else
			it++;
	}

	if (bFoundMatch)
		return it->second;
	else
		return null;
}

KeyValues* C_MetaverseManager::FindLibraryItem(KeyValues* pSearchInfo)
{
	//DevMsg("C_MetaverseManager: FindLibraryItem with ONLY pSearchinfo!!\n");
	KeyValues* potential;
	KeyValues* active;
	KeyValues* searchField;
	std::string fieldName, potentialBuf, searchBuf;
	char charBuf[AA_MAX_STRING];
	std::vector<std::string> searchTokens;
	unsigned int i, numTokens;
	bool bFoundMatch = false;
	std::map<std::string, KeyValues*>::iterator it = m_items.begin();
	while (it != m_items.end())
	{
		bFoundMatch = false;
		potential = it->second;
		active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(potential);
		// active has the potential item data
		// pSearchInfo has the search criteria
		for (searchField = pSearchInfo->GetFirstSubKey(); searchField; searchField = searchField->GetNextKey())
		{
			fieldName = searchField->GetName();
			if (fieldName == "title")
			{
				potentialBuf = searchField->GetString();	// FIXME: Does this work? seems to work... but how?
				std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

				searchBuf = pSearchInfo->GetString(fieldName.c_str());
				searchTokens.clear();
				g_pAnarchyManager->Tokenize(searchBuf, searchTokens, ", ");
				numTokens = searchTokens.size();

				for (i = 0; i < numTokens; i++)
				{
					if (searchTokens[i].length() < 2)
						continue;

					if (potentialBuf.find(searchTokens[i]) != std::string::npos)
					{
						bFoundMatch = true;
						break;
					}
				}

				if (bFoundMatch)
					break;
			}
			else
			{
				potentialBuf = active->GetString(fieldName.c_str());
				std::transform(potentialBuf.begin(), potentialBuf.end(), potentialBuf.begin(), ::tolower);

				searchBuf = searchField->GetString();
				std::transform(searchBuf.begin(), searchBuf.end(), searchBuf.begin(), ::tolower);

				if (potentialBuf == searchBuf)
				{
				//	DevMsg("Found match with %s = %s\n", potentialBuf.c_str(), searchBuf.c_str());
					bFoundMatch = true;
					break;
				}
			}
		}

		if (bFoundMatch)
			break;
		else
			it++;
	}

	// Do this here because we MUST handle the deletion for findNext and findFirst, so this usage should match!!
	pSearchInfo->deleteThis();

	if (bFoundMatch)
		return it->second;
	else
		return null;
}

std::string C_MetaverseManager::FindFirstLibraryEntry(KeyValues*& response, const char* category, KeyValues* pSearchInfo)
{
	//std::string categoryBuf = std::string(category);

	//if (categoryBuf == "")
	//	categoryBuf = "items";

	std::string queryId = g_pAnarchyManager->GenerateUniqueId();
	//DevMsg("Query ID point A: %s\n", queryId.c_str());
	//std::string idBuf = std::string(queryId);

	// TODO: use queryId to organize N search queries & store the category info for each.
	KeyValues* pEntry;
	KeyValues* pPreviousSearchInfo = null;
	std::map<std::string, KeyValues*>::iterator* it;
	std::map<std::string, KeyValues*>* categoryEntries;
	if (!Q_strcmp(category, "items"))
	{
		categoryEntries = &m_items;

		if (m_pPreviousSearchInfo)
		{
			DevMsg("Cleaning up a library query context that was left open...\n");	// FIXME: This entire block should be handled differently as soon as concurrent library queries are supported, but right now we're actually limited to 1 query per category.
			pPreviousSearchInfo->deleteThis();
		}

		m_pPreviousSearchInfo = pSearchInfo;
		it = &m_previousFindItemIterator;
	}
	else if (!Q_strcmp(category, "models")) // if (!Q_strcmp(category, "items"))
	{
		categoryEntries = &m_models;

		if (m_pPreviousModelSearchInfo)
		{
			DevMsg("Cleaning up a library query context that was left open...\n");	// FIXME: This entire block should be handled differently as soon as concurrent library queries are supported, but right now we're actually limited to 1 query per category.
			m_pPreviousModelSearchInfo->deleteThis();
		}

		m_pPreviousModelSearchInfo = pSearchInfo;
		it = &m_previousFindModelIterator;
	}
	else if (!Q_strcmp(category, "apps"))
	{
		categoryEntries = &m_apps;

		if (m_pPreviousAppSearchInfo)
		{
			DevMsg("Cleaning up a library query context that was left open...\n");	// FIXME: This entire block should be handled differently as soon as concurrent library queries are supported, but right now we're actually limited to 1 query per category.
			m_pPreviousAppSearchInfo->deleteThis();
		}

		m_pPreviousAppSearchInfo = pSearchInfo;
		it = &m_previousFindAppIterator;
	}
	
	// start the search
	//m_previousFindItemIterator = categoryEntries->begin();
	*it = categoryEntries->begin();
	pEntry = this->FindLibraryEntry(category, pSearchInfo, *it);

	response = pEntry;
	//std::string idBuf = std::string(queryId);
	return queryId.c_str();
}

KeyValues* C_MetaverseManager::FindNextLibraryEntry(const char* queryId, const char* category)
{
	KeyValues* response = null;
	if (!Q_strcmp(category, "items"))
	{
		// continue the search
		m_previousFindItemIterator++;
		if (m_previousFindItemIterator != m_items.end())
			response = this->FindLibraryEntry(category, m_pPreviousSearchInfo, m_previousFindItemIterator);
	}
	else if (!Q_strcmp(category, "models"))
	{
		// continue the search
		m_previousFindModelIterator++;
		if (m_previousFindModelIterator != m_models.end())
			response = this->FindLibraryEntry(category, m_pPreviousModelSearchInfo, m_previousFindModelIterator);
	}
	else if (!Q_strcmp(category, "apps"))
	{
		// continue the search
		m_previousFindAppIterator++;
		if (m_previousFindAppIterator != m_apps.end())
			response = this->FindLibraryEntry(category, m_pPreviousAppSearchInfo, m_previousFindAppIterator);
	}

	return response;
}

KeyValues* C_MetaverseManager::FindFirstLibraryItem(KeyValues* pSearchInfo)
{
	// remember this search query
	if (!m_pPreviousSearchInfo)
		m_pPreviousSearchInfo = pSearchInfo;// new KeyValues("search");
	else if (m_pPreviousSearchInfo != pSearchInfo)	// this should never be called!!!
	{
		m_pPreviousSearchInfo->deleteThis();
		m_pPreviousSearchInfo = pSearchInfo;
	}

	m_previousFindItemIterator = m_items.begin();

	// start the search
	KeyValues* response = this->FindLibraryItem(m_pPreviousSearchInfo, m_previousFindItemIterator);
	return response;
}

KeyValues* C_MetaverseManager::FindNextLibraryItem()
{
	// continue the search
	KeyValues* response = null;
	m_previousFindItemIterator++;
	if (m_previousFindItemIterator != m_items.end())
		response = this->FindLibraryItem(m_pPreviousSearchInfo, m_previousFindItemIterator);
	return response;
}

KeyValues* C_MetaverseManager::GetScreenshot(std::string id)
{
	std::map<std::string, KeyValues*>::iterator it = m_mapScreenshots.find(id);
	if (it != m_mapScreenshots.end())
		return it->second;
	else
		return null;
}

void C_MetaverseManager::AddScreenshot(KeyValues* pScreenshotKV)
{
	std::string id = pScreenshotKV->GetString("id");
	m_mapScreenshots[id] = pScreenshotKV;
}

void C_MetaverseManager::DeleteScreenshot(std::string id)
{
	std::string filename;
	KeyValues* pScreenshotKV = this->GetScreenshot(id);
	if (pScreenshotKV)
	{
		// 1. Delete [ID].tga and [ID].txt from the shots folder.
		filename = "shots\\" + id + ".tga";
		if (g_pFullFileSystem->FileExists(filename.c_str(), "DEFAULT_WRITE_PATH"))
			g_pFullFileSystem->RemoveFile(filename.c_str(), "DEFAULT_WRITE_PATH");

		filename = "shots\\" + id + ".txt";
		if (g_pFullFileSystem->FileExists(filename.c_str(), "DEFAULT_WRITE_PATH"))
			g_pFullFileSystem->RemoveFile(filename.c_str(), "DEFAULT_WRITE_PATH");

		// 2. Remove the entry from m_mapScreenshots & delete the KeyValues.
		pScreenshotKV->deleteThis();
		m_mapScreenshots.erase(id);
	}
}

KeyValues* C_MetaverseManager::FindMostRecentScreenshot(std::string mapId, instance_t* pInstance)
{
	std::string instanceId = (pInstance) ? pInstance->id : "";

	KeyValues* pMapKV = null;
	std::string mapFileName = "";
	std::string goodMapId = "";
	if (mapId != "")
	{
		goodMapId = mapId;
		pMapKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(this->GetMap(goodMapId));
		if (pMapKV)
		{
			goodMapId = pMapKV->GetString("info/id");
			mapFileName = pMapKV->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));
		}
	}
	else
	{
		mapFileName = std::string(g_pAnarchyManager->MapName()) + std::string(".bsp");
		pMapKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(this->FindMap(mapFileName.c_str()));
		if (pMapKV)
			goodMapId = pMapKV->GetString("info/id");
	}

	if (goodMapId == "" || mapFileName == "" )
		return null;

	//std::string testId;
	std::vector<KeyValues*> screenshots;
	std::map<std::string, KeyValues*>::iterator it = m_mapScreenshots.begin();
	while (it != m_mapScreenshots.end())
	{
		//testId = it->second->GetString("map/id");	// SOMEHOW COULD BE 1.#INF00 when printed as a string, even though it was fine in the KV text. Like for ID 545e1841
		//testId = (instanceId == "") ? it->second->GetString("instance/mapId") : it->second->GetString("instance/id");
		//DevMsg("IDs: %s vs %s\n", testId.c_str(), goodMapId.c_str());
		//if ((instanceId == "" && testId == goodMapId) || (instanceId != "" && testId == instanceId))
		if (!Q_strcmp(it->second->GetString("map/file"), mapFileName.c_str()))
			screenshots.push_back(it->second);

		it++;
	}

	// now screenshots holds all applicable screenshots.  find the most recent one.
	KeyValues* pBestScreenshot = null;
	uint64 bestValue = 0;
	uint64 testValue;
	unsigned int max = screenshots.size();
	for (unsigned int i = 0; i < max; i++)
	{
		testValue = Q_atoui64(screenshots[i]->GetString("created"));
		if (testValue > bestValue)
		{
			bestValue = testValue;
			pBestScreenshot = screenshots[i];
		}
	}

	return pBestScreenshot;
}

KeyValues* C_MetaverseManager::GetLibraryApp(std::string id)
{
	std::map<std::string, KeyValues*>::iterator it = m_apps.find(id);
	if (it != m_apps.end())
		return it->second;
	else
		return null;
}

KeyValues* C_MetaverseManager::GetLibraryMap(std::string id)
{
	std::map<std::string, KeyValues*>::iterator it = m_maps.find(id);
	if (it != m_maps.end())
		return it->second;
	else
		return null;
}

std::map<std::string, KeyValues*>& C_MetaverseManager::DetectAllMapScreenshots()
{
	DevMsg("Detecting all map screenshots...\n");
	size_t found;
	std::string id;
	//std::map<std::string, std::string>::iterator it;

	// get every shot that exists
	KeyValues* pShotKV;
	FileFindHandle_t pFileFindHandle;
	const char *pScreenshotInfoFile = g_pFullFileSystem->FindFirstEx("shots\\*.txt", "MOD", &pFileFindHandle);
	while (pScreenshotInfoFile != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(pFileFindHandle))
		{
			pScreenshotInfoFile = g_pFullFileSystem->FindNext(pFileFindHandle);
			continue;
		}

		id = pScreenshotInfoFile;
		id = id.substr(0, id.find(".txt"));

		pShotKV = new KeyValues("screenshot");
		if (pShotKV->LoadFromFile(g_pFullFileSystem, VarArgs("shots\\%s", pScreenshotInfoFile), "MOD"))
		{
			// IMPORTANT NOTE: Key values loaded with LoadFromFile forget the data types for their fields and if a string is just numbers, it gets turned into a number instead of a string after loading.
			// This matters if the number started with a 0, because leading zeros get removed for numbers.
			// So to catch this, additional checks must be performed on ID's read from KeyValues files.
			//DevMsg("Here the map ID for the screenshot is: %s or %i or %s and type %s\n", pShotKV->GetString("map/id"), pShotKV->GetInt("map/id"), pShotKV->GetRawString("map/id"), pShotKV->GetDataType("map/id"));
			m_mapScreenshots[id] = pShotKV;
		}
		else
			pShotKV->deleteThis();
		
		pScreenshotInfoFile = g_pFullFileSystem->FindNext(pFileFindHandle);
	}
	g_pFullFileSystem->FindClose(pFileFindHandle);

	return m_mapScreenshots;
}

void C_MetaverseManager::GetAllMapScreenshots(std::vector<KeyValues*>& responseVector, std::string mapId)
{
	std::map<std::string, KeyValues*>::iterator it = m_mapScreenshots.begin();
	while (it != m_mapScreenshots.end())
	{
		//DevMsg("Screenshot check: %s vs %s vs %i\n", mapId.c_str(), it->second->GetString("map/id"), it->second->GetInt("map/id"));
		//if (mapId == "" || mapId == it->second->GetString("map/id"))//; it->first)
		if (mapId == "" || g_pAnarchyManager->CompareLoadedFromKeyValuesFileId(it->second->GetString("map/id"), mapId.c_str()) )
			responseVector.push_back(it->second);

		it++;
	}
}

KeyValues* C_MetaverseManager::FindMap(const char* mapFile)
{
	KeyValues* map;
	std::map<std::string, KeyValues*>::iterator it = m_maps.begin();
	while (it != m_maps.end())
	{
		map = this->GetActiveKeyValues(it->second);
		//DevMsg("Map name is: %s\n", map->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file"));
		if (!Q_stricmp(map->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file"), mapFile))
			return it->second;

		it++;
	}

	return null;
}

KeyValues* C_MetaverseManager::GetMap(std::string mapId)
{
	std::map<std::string, KeyValues*>::iterator it = m_maps.find(mapId);
	if (it != m_maps.end())
		return it->second;
	return null;
}

void C_MetaverseManager::DetectAllLegacyCabinets()
{
	std::string goodTitle;
	std::string modelId;
	KeyValues* pModel;
	KeyValues* cat = new KeyValues("cat");
	FileFindHandle_t handle;
	const char *pFilename = g_pFullFileSystem->FindFirstEx("resource\\models\\*.cat", "GAME", &handle);
	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(handle))
		{
			pFilename = g_pFullFileSystem->FindNext(handle);
			continue;
		}

		// load up the cat
		cat->Clear();
		if (cat->LoadFromFile(g_pFullFileSystem, VarArgs("resource\\models\\%s", pFilename), "GAME"))
		{
			modelId = g_pAnarchyManager->GenerateLegacyHash(cat->GetString("model"));

			std::map<std::string, KeyValues*>::iterator oldIt = m_models.find(modelId);
			if (oldIt == m_models.end())
			{
				pModel = new KeyValues("model");

				// update us to 3rd generation
				pModel->SetInt("generation", 3);

				// add standard info (except for id)
				pModel->SetInt("local/info/created", 0);
				pModel->SetString("local/info/owner", "local");
				pModel->SetInt("local/info/removed", 0);
				//pModel->SetString("local/info/remover", "");
				//pModel->SetString("local/info/alias", "");


				pModel->SetString("local/info/id", modelId.c_str());

				pModel->SetString("local/title", cat->GetString("name"));
				pModel->SetString("local/keywords", cat->GetString("category"));
				pModel->SetInt("local/dynamic", 1);
				pModel->SetString(VarArgs("local/platforms/%s/id", AA_PLATFORM_ID), AA_PLATFORM_ID);
				pModel->SetString(VarArgs("local/platforms/%s/file", AA_PLATFORM_ID), cat->GetString("model"));
				//pModel->SetString(VarArgs("local/platforms/%s/download", AA_PLATFORM_ID), "");

				//pModel->SetString(VarArgs("local/platforms/%s/workshopIds", AA_PLATFORM_ID), "");
				//pModel->SetString(VarArgs("local/platforms/%s/mountIds", AA_PLATFORM_ID), "");

				// models can be loaded right away because they don't depend on anything else, like items do. (items depend on models)
				DevMsg("Loading cabinet model with ID %s and model %s\n", modelId.c_str(), cat->GetString("model"));
				m_models[modelId] = pModel;
			}
		}

		pFilename = g_pFullFileSystem->FindNext(handle);
	}

	g_pFullFileSystem->FindClose(handle);
	cat->deleteThis();
}

void C_MetaverseManager::DetectAllMaps()
{
	DevMsg("C_MetaverseManager::DetectAllMaps\n");	// fix me down there at the todo!!

	// Load all the .key files from the library/instances folder.
	// then add their instances:

	/* done elsewhere now
	// make it use the new shinnit
	unsigned int count = 0;
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(m_db, "SELECT * from instances", -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed: %s\n", sqlite3_errmsg(m_db));

	// TODO: Fix these things to have real values!!  This is very much like the logic done ELSEWHERE!! FIND IT!!

	int length;
	int iGeneration;
	int iLegacy;
	std::string instanceId;
	std::string mapId;
	std::string title;
	std::string file;
	std::string workshopIds;
	std::string mountIds;
	//std::string backpackId;
	std::string style;
	KeyValues* pInstanceKV;
	KeyValues* pInstanceInfoKV;
	while (sqlite3_step(stmt) == SQLITE_ROW)	// THIS IS WHERE THE LOOP CAN BE BROKEN UP AT!!
	{
		length = sqlite3_column_bytes(stmt, 1);

		// FIXME: TODO: Detect if the map is from a workshop or mount id too!
		pInstanceKV = new KeyValues("instance");
		if (this->LoadSQLKevValues("instances", (const char*)sqlite3_column_text(stmt, 0), pInstanceKV))
		{
			iGeneration = pInstanceKV->GetInt("generation", 3);
			iLegacy = pInstanceKV->GetInt("legacy");

			pInstanceInfoKV = pInstanceKV->FindKey("info/local", true);
			instanceId = pInstanceInfoKV->GetString("id");

			if (instanceId == "")
			{
				DevMsg("Warning: Skipping instance with no ID.\n");
				continue;
			}

			mapId = pInstanceInfoKV->GetString("map");
			title = pInstanceInfoKV->GetString("title");
			if (title == "")
				title = "Unnamed (" + instanceId + ")";
			file = "";
			workshopIds = pInstanceInfoKV->GetString(VarArgs("platforms/%s/workshopIds", AA_PLATFORM_ID));
			mountIds = pInstanceInfoKV->GetString(VarArgs("platforms/%s/mountIds", AA_PLATFORM_ID));
			//backpackId = pInstanceInfoKV->GetString(VarArgs("platforms/%s/backpackIds", AA_PLATFORM_ID));
			style = pInstanceInfoKV->GetString("style");

			g_pAnarchyManager->GetInstanceManager()->AddInstance(iLegacy, instanceId, mapId, title, file, workshopIds, mountIds, style);// (kv->GetInt("legacy"), instanceId, mapId, title, file, workshopIds, mountIds, style);//(instanceId, kv->GetString("info/map"), goodTitle, goodLegacyFile);
		}
		pInstanceKV->deleteThis();
		pInstanceKV = null;
	}
	sqlite3_finalize(stmt);
	*/

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	bool bAlreadyExists;

	DevMsg("Detect first map...\n");
	KeyValues* map = this->DetectFirstMap(bAlreadyExists);
	if (map)
	{
		if (bAlreadyExists)
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Detecting Maps", "detectmaps", "", "", "+0", "detectNextMapCallback");
		else
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Detecting Maps", "detectmaps", "", "", "+", "detectNextMapCallback");
	}
	else
		this->OnDetectAllMapsCompleted();

	DevMsg("Initial logic for DetectAllMaps complete.\n");
}

KeyValues* C_MetaverseManager::CreateModelFromFileTarget(std::string modelFile)
{
	KeyValues* pModel = new KeyValues("model");
	pModel->SetInt("generation", 3);
	// add standard info (except for id)
	pModel->SetString("local/info/created", VarArgs("%llu", g_pAnarchyManager->GetTimeNumber()));
	pModel->SetString("local/info/owner", "local");
	//pModel->SetInt("local/info/removed", 0);
	//pModel->SetString("local/info/remover", "");
	//pModel->SetString("local/info/alias", "");

	std::string modelId = g_pAnarchyManager->GenerateLegacyHash(modelFile.c_str());// g_pAnarchyManager->GenerateUniqueId();// > GenerateLegacyHash(itemFile.c_str());
	pModel->SetString("local/info/id", modelId.c_str());

	std::string goodTitle = modelFile;
	goodTitle = goodTitle.substr(0, goodTitle.length() - 4);

	size_t foundLastSlash = goodTitle.find_last_of("/\\");
	if (foundLastSlash != std::string::npos)
		goodTitle = goodTitle.substr(foundLastSlash + 1);

	pModel->SetString("local/title", goodTitle.c_str());
	//pModel->SetString("local/preview", );
	//pModel->SetString("local/keywords", );

	//modelFile = VarArgs("%s\\%s", folder, potentialFile);
	if ((modelFile.find("models\\cabinets\\") == 0 || modelFile.find("models/cabinets/") == 0 || modelFile.find("models\\banners\\") == 0 || modelFile.find("models/banners/") == 0 || modelFile.find("models\\frames\\") == 0 || modelFile.find("models/frames/") == 0 || modelFile.find("models\\icons\\") == 0 || modelFile.find("models/icons/") == 0) && (modelFile.find("room_divider.mdl") == std::string::npos && modelFile.find("newton_toy.mdl") == std::string::npos))
		pModel->SetInt("local/dynamic", 1);
	else
		pModel->SetInt("local/dynamic", 0);

	pModel->SetString(VarArgs("local/platforms/%s/id", AA_PLATFORM_ID), AA_PLATFORM_ID);
	pModel->SetString(VarArgs("local/platforms/%s/file", AA_PLATFORM_ID), modelFile.c_str());
	//pModel->SetString(VarArgs("local/platforms/%s/download", AA_PLATFORM_ID), );
	//pModel->SetString(VarArgs("local/platforms/%s/workshopIds", AA_PLATFORM_ID), );


	KeyValues* modelInfo = g_pAnarchyManager->GetMetaverseManager()->DetectRequiredWorkshopForModelFile(modelFile);
	/*
	KeyValues* modelInfo = new KeyValues("modelInfo");
	modelInfo->SetString("fullfile", modelBuf.c_str());
	modelInfo->SetString("workshopIds", workshopId.c_str());
	modelInfo->SetString("mountIds", mountId.c_str());
	modelInfo->SetString("mountTitle", mountTitle.c_str());
	modelInfo->SetBool("bIsWorkshop", bIsWorkshop);
	modelInfo->SetBool("bIsLegacyImported", bIsLegacyImported);
	return modelInfo;
	*/

	std::string buf = modelInfo->GetString("mountIds");
	if (buf != "")
		pModel->SetString(VarArgs("local/platforms/%s/mountIds", AA_PLATFORM_ID), buf.c_str());

	buf = modelInfo->GetString("workshopIds");
	if (buf != "")
		pModel->SetString(VarArgs("local/platforms/%s/workshopIds", AA_PLATFORM_ID), buf.c_str());

	// backpack stuff
	buf = g_pAnarchyManager->GetBackpackManager()->DetectRequiredBackpackForModelFile(modelFile);
	if (buf != "")
		pModel->SetString(VarArgs("local/platforms/%s/backpackIds", AA_PLATFORM_ID), buf.c_str());

	modelInfo->deleteThis();
	return pModel;
}

bool C_MetaverseManager::ProcessModel(std::string modelFile)
{
	KeyValues* pModel = this->CreateModelFromFileTarget(modelFile);
	if (pModel)
	{
		g_pAnarchyManager->GetMetaverseManager()->AddModel(pModel);
		g_pAnarchyManager->GetMetaverseManager()->SaveModel(pModel);
		return true;
	}

	return false;
}

unsigned int C_MetaverseManager::ProcessModels(importInfo_t* pImportInfo)
{
	KeyValues* pSearchInfoKV = new KeyValues("search");
	std::string modelFile;
	std::string standardizedModelFile;
	KeyValues* pEntry;
	KeyValues* pModel;
	KeyValues* modelInfo;
	size_t foundLastSlash;
	std::string buf;
	unsigned int uGoodCount = 0;
	unsigned int uMax = pImportInfo->data.size();
	for (unsigned int i = 0; i < uMax; i++)
	{
		modelFile = pImportInfo->data[i];

		// Check if this MDL already exists...
		standardizedModelFile = modelFile;
		std::replace(standardizedModelFile.begin(), standardizedModelFile.end(), '/', '\\');
		std::transform(standardizedModelFile.begin(), standardizedModelFile.end(), standardizedModelFile.begin(), ::tolower);

		pSearchInfoKV->Clear();
		pSearchInfoKV->SetString("file", standardizedModelFile.c_str());	// platform/AA_SOURCE_PLATFORM_ID/file is automatically used in the search function for models.

		// find the first entry that matches the search params
		auto it = m_models.begin();
		pEntry = g_pAnarchyManager->GetMetaverseManager()->FindLibraryEntry("models", pSearchInfoKV, it);// m_models.begin());
		if (!pEntry)
		{
			pModel = new KeyValues("model");
			pModel->SetInt("generation", 3);
			// add standard info (except for id)
			pModel->SetString("local/info/created", VarArgs("%llu", g_pAnarchyManager->GetTimeNumber()));
			pModel->SetString("local/info/owner", "local");
			//pModel->SetInt("local/info/removed", 0);
			//pModel->SetString("local/info/remover", "");
			//pModel->SetString("local/info/alias", "");

			std::string modelId = g_pAnarchyManager->GenerateLegacyHash(modelFile.c_str());	//GenerateUniqueId();// > GenerateLegacyHash(itemFile.c_str());
			pModel->SetString("local/info/id", modelId.c_str());

			std::string goodTitle = modelFile;
			goodTitle = goodTitle.substr(0, goodTitle.length() - 4);

			foundLastSlash = goodTitle.find_last_of("/\\");
			if (foundLastSlash != std::string::npos)
				goodTitle = goodTitle.substr(foundLastSlash + 1);

			pModel->SetString("local/title", goodTitle.c_str());
			//pModel->SetString("local/preview", );
			//pModel->SetString("local/keywords", );

			//modelFile = VarArgs("%s\\%s", folder, potentialFile);
			if ((modelFile.find("models\\cabinets\\") == 0 || modelFile.find("models/cabinets/") == 0 || modelFile.find("models\\banners\\") == 0 || modelFile.find("models/banners/") == 0 || modelFile.find("models\\frames\\") == 0 || modelFile.find("models/frames/") == 0 || modelFile.find("models\\icons\\") == 0 || modelFile.find("models/icons/") == 0) && (modelFile.find("room_divider.mdl") == std::string::npos && modelFile.find("newton_toy.mdl") == std::string::npos))
				pModel->SetInt("local/dynamic", 1);
			else
				pModel->SetInt("local/dynamic", 0);

			pModel->SetString(VarArgs("local/platforms/%s/id", AA_PLATFORM_ID), AA_PLATFORM_ID);
			pModel->SetString(VarArgs("local/platforms/%s/file", AA_PLATFORM_ID), modelFile.c_str());
			//pModel->SetString(VarArgs("local/platforms/%s/download", AA_PLATFORM_ID), );
			//pModel->SetString(VarArgs("local/platforms/%s/workshopIds", AA_PLATFORM_ID), );


			modelInfo = g_pAnarchyManager->GetMetaverseManager()->DetectRequiredWorkshopForModelFile(modelFile);
			/*
			KeyValues* modelInfo = new KeyValues("modelInfo");
			modelInfo->SetString("fullfile", modelBuf.c_str());
			modelInfo->SetString("workshopIds", workshopId.c_str());
			modelInfo->SetString("mountIds", mountId.c_str());
			modelInfo->SetString("mountTitle", mountTitle.c_str());
			modelInfo->SetBool("bIsWorkshop", bIsWorkshop);
			modelInfo->SetBool("bIsLegacyImported", bIsLegacyImported);
			return modelInfo;
			*/

			buf = modelInfo->GetString("mountIds");
			if (buf != "")
				pModel->SetString(VarArgs("local/platforms/%s/mountIds", AA_PLATFORM_ID), buf.c_str());

			buf = modelInfo->GetString("workshopIds");
			if (buf != "")
				pModel->SetString(VarArgs("local/platforms/%s/workshopIds", AA_PLATFORM_ID), buf.c_str());

			// backpack stuff
			buf = g_pAnarchyManager->GetBackpackManager()->DetectRequiredBackpackForModelFile(modelFile);
			if (buf != "")
				pModel->SetString(VarArgs("local/platforms/%s/backpackIds", AA_PLATFORM_ID), buf.c_str());

			modelInfo->deleteThis();

			m_models[modelId] = pModel;
			this->SaveSQL(null, "models", modelId.c_str(), pModel);
			uGoodCount++;
		}
	}

	pSearchInfoKV->deleteThis();
	return uGoodCount;
}

void C_MetaverseManager::DetectAllModels()
{
	DevMsg("C_MetaverseManager::DetectAllModels\n");

	//FileFindHandle_t previousDetectedLocalModelFileHandle;
	//std::string pathname = "models";
	//const char* pFilename = g_pFullFileSystem->FindFirstEx("models\\*", "GAME", &previousDetectedLocalModelFileHandle);
	//this->DetectAllModelsRecursive(pathname, previousDetectedLocalModelFileHandle, pFilename);
	unsigned int count = this->DetectAllModelsRecursive("models");
	DevMsg("Detected %u models.\n", count);
}


unsigned int C_MetaverseManager::DetectAllModelsRecursive(const char* folder)//std::string pathname, FileFindHandle_t hFile, const char* pFilename)
{
	//DevMsg("Folder is: %s\n", folder);
	KeyValues* pSearchInfoKV = new KeyValues("search");
	unsigned int count = 0;
	FileFindHandle_t hFileSearch;
	std::string composedFile;
	std::string modelFile;
	std::string standardizedModelFile;
	KeyValues* pEntry;
	KeyValues* pModel;
	KeyValues* modelInfo;
	//size_t foundLastSlash;
	std::string buf;
	const char* potentialFile = g_pFullFileSystem->FindFirstEx(VarArgs("%s\\*", folder), "GAME", &hFileSearch);
	while (potentialFile)
	{
		if (!Q_strcmp(potentialFile, ".") || !Q_strcmp(potentialFile, ".."))
		{
			potentialFile = g_pFullFileSystem->FindNext(hFileSearch);
			continue;
		}

		composedFile = std::string(folder) + "\\" + std::string(potentialFile);

		if (g_pFullFileSystem->FindIsDirectory(hFileSearch))
		{
			//DevMsg("Checking folder: %s\n", composedFile.c_str());
			count += this->DetectAllModelsRecursive(composedFile.c_str());
			potentialFile = g_pFullFileSystem->FindNext(hFileSearch);
			continue;
		}
		else
		{
			if (V_GetFileExtension(potentialFile) && !Q_stricmp(V_GetFileExtension(potentialFile), "mdl"))
			{
				//DevMsg("Dump: %s\n", composedFile.c_str());
				modelFile = composedFile;// std::string(folder) + "\\" + std::string(potentialFile);// VarArgs("%s\\%s", folder, potentialFile);

				// Check if this MDL already exists...
				standardizedModelFile = modelFile;
				std::replace(standardizedModelFile.begin(), standardizedModelFile.end(), '/', '\\');
				std::transform(standardizedModelFile.begin(), standardizedModelFile.end(), standardizedModelFile.begin(), ::tolower);

				pSearchInfoKV->Clear();
				pSearchInfoKV->SetString("file", standardizedModelFile.c_str());	// platform/AA_SOURCE_PLATFORM_ID/file is automatically used in the search function for models.

				// find the first entry that matches the search params
				auto it = m_models.begin();
				pEntry = g_pAnarchyManager->GetMetaverseManager()->FindLibraryEntry("models", pSearchInfoKV, it);// m_models.begin());
				if (!pEntry)
				{
					pModel = new KeyValues("model");
					pModel->SetInt("generation", 3);
					// add standard info (except for id)
					pModel->SetString("local/info/created", VarArgs("%llu", g_pAnarchyManager->GetTimeNumber()));
					pModel->SetString("local/info/owner", "local");
					//pModel->SetInt("local/info/removed", 0);
					//pModel->SetString("local/info/remover", "");
					//pModel->SetString("local/info/alias", "");

					std::string modelId = g_pAnarchyManager->GenerateLegacyHash(modelFile.c_str()); //GenerateUniqueId();
					pModel->SetString("local/info/id", modelId.c_str());

					std::string goodTitle = potentialFile;
					goodTitle = goodTitle.substr(0, goodTitle.length() - 4);

					//foundLastSlash = goodTitle.find_last_of("/\\");
					//if (foundLastSlash != std::string::npos)
					//	goodTitle = goodTitle.substr(foundLastSlash + 1);

					pModel->SetString("local/title", goodTitle.c_str());
					//pModel->SetString("local/preview", );
					//pModel->SetString("local/keywords", );

					//modelFile = VarArgs("%s\\%s", folder, potentialFile);
					if ((modelFile.find("models\\cabinets\\") == 0 || modelFile.find("models/cabinets/") == 0 || modelFile.find("models\\banners\\") == 0 || modelFile.find("models/banners/") == 0 || modelFile.find("models\\frames\\") == 0 || modelFile.find("models/frames/") == 0 || modelFile.find("models\\icons\\") == 0 || modelFile.find("models/icons/") == 0) && (modelFile.find("room_divider.mdl") == std::string::npos && modelFile.find("newton_toy.mdl") == std::string::npos))
						pModel->SetInt("local/dynamic", 1);
					else
						pModel->SetInt("local/dynamic", 0);

					pModel->SetString(VarArgs("local/platforms/%s/id", AA_PLATFORM_ID), AA_PLATFORM_ID);
					pModel->SetString(VarArgs("local/platforms/%s/file", AA_PLATFORM_ID), modelFile.c_str());
					//pModel->SetString(VarArgs("local/platforms/%s/download", AA_PLATFORM_ID), );
					//pModel->SetString(VarArgs("local/platforms/%s/workshopIds", AA_PLATFORM_ID), );


					modelInfo = g_pAnarchyManager->GetMetaverseManager()->DetectRequiredWorkshopForModelFile(modelFile);
					/*
					KeyValues* modelInfo = new KeyValues("modelInfo");
					modelInfo->SetString("fullfile", modelBuf.c_str());
					modelInfo->SetString("workshopIds", workshopId.c_str());
					modelInfo->SetString("mountIds", mountId.c_str());
					modelInfo->SetString("mountTitle", mountTitle.c_str());
					modelInfo->SetBool("bIsWorkshop", bIsWorkshop);
					modelInfo->SetBool("bIsLegacyImported", bIsLegacyImported);
					return modelInfo;
					*/

					buf = modelInfo->GetString("mountIds");
					if (buf != "")
						pModel->SetString(VarArgs("local/platforms/%s/mountIds", AA_PLATFORM_ID), buf.c_str());

					buf = modelInfo->GetString("workshopIds");
					if (buf != "")
						pModel->SetString(VarArgs("local/platforms/%s/workshopIds", AA_PLATFORM_ID), buf.c_str());
					
					// backpack stuff
					buf = g_pAnarchyManager->GetBackpackManager()->DetectRequiredBackpackForModelFile(modelFile);
					if (buf != "")
						pModel->SetString(VarArgs("local/platforms/%s/backpackIds", AA_PLATFORM_ID), buf.c_str());

					modelInfo->deleteThis();

					m_models[modelId] = pModel;
					this->SaveSQL(null, "models", modelId.c_str(), pModel);
					//DevMsg("Added model: %s\n", modelFile.c_str());
					count++;
				}
			}

			potentialFile = g_pFullFileSystem->FindNext(hFileSearch);
			continue;
		}
	}

	g_pFullFileSystem->FindClose(hFileSearch);
	pSearchInfoKV->deleteThis();
	return count;
}

/*
void C_MetaverseManager::DetectAllModelsRecursive(std::string pathname, FileFindHandle_t hFile, const char* pFilename)
{
	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(hFile))
		{
			std::string nextpathname = pathname + "\\" + std::string(pFilename);

			this->DetectAllModelsRecursive(nextpathname, hFile, pFilename);
			pFilename = g_pFullFileSystem->FindNext(hFile);
			continue;	// automatically skip to the next one, until a non-folder is hit.
		}

		std::string foundName = pathname + std::string(pFilename);

		KeyValues* active;
		std::map<std::string, KeyValues*>::iterator it = m_maps.begin();
		while (it != m_maps.end())
		{
			active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(it->second);

			if (!Q_strcmp(active->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file"), foundName.c_str()))
			{
				//g_pFullFileSystem->FindClose(m_previousDetectLocalMapFileHandle);
				bAlreadyExists = true;
				return it->second;
			}

			it++;
		}

		// if we haven't found a map for this yet, let's make one.
		KeyValues* map = new KeyValues("map");
		map->SetInt("generation", 3);

		// add standard info
		//std::string id = g_pAnarchyManager->GenerateUniqueId();

		std::string goodTitle = pFilename;
		size_t found = goodTitle.find(".");
		if (found != std::string::npos)
			goodTitle = goodTitle.substr(0, found);

		std::string id = g_pAnarchyManager->GenerateLegacyHash(goodTitle.c_str());

		map->SetString("local/info/id", id.c_str());
		map->SetInt("local/info/created", 0);
		map->SetString("local/info/owner", "local");
		map->SetInt("local/info/removed", 0);
		map->SetString("local/info/remover", "");
		map->SetString("local/info/alias", "");

		std::string mapName = foundName.substr(0, foundName.length() - 4);
		map->SetString("local/title", mapName.c_str());
		map->SetString("local/keywords", "");
		map->SetString("local/platforms/-KJvcne3IKMZQTaG7lPo/file", foundName.c_str());

		KeyValues* stuffKV = g_pAnarchyManager->GetMetaverseManager()->DetectRequiredWorkshopForMapFile(foundName.c_str());
		if (stuffKV)
		{
			map->SetString("local/platforms/-KJvcne3IKMZQTaG7lPo/workshopIds", stuffKV->GetString("workshopIds"));
			map->SetString("local/platforms/-KJvcne3IKMZQTaG7lPo/mountIds", stuffKV->GetString("mountIds"));
			stuffKV->deleteThis();
		}

		m_maps[id.c_str()] = map;
		bAlreadyExists = false;

		// update any legacy instances that were detected that use this map
		std::string legacyMapName = foundName.substr(0, foundName.length() - 4);
		DevMsg("LEGACY MAP NAME: %s\n", legacyMapName.c_str());
		g_pAnarchyManager->GetInstanceManager()->LegacyMapIdFix(legacyMapName, id);
		//		if (pInstance)
		//		pInstance->mapId = id;

		return map;
	}

	g_pFullFileSystem->FindClose(m_previousDetectLocalMapFileHandle);
	return null;
}
*/

void C_MetaverseManager::OnDetectAllMapsCompleted()
{
	DevMsg("Done detecting maps!\n");
	g_pAnarchyManager->OnDetectAllMapsComplete();
}

KeyValues* C_MetaverseManager::GetActiveKeyValues(KeyValues* entry)
{
	// return null if given a null argument to avoid the if-then cluster fuck of error checking each step of this common task
	if (!entry)
		return null;

	KeyValues* active = entry->FindKey("current");
	if (!active)
		active = entry->FindKey("local", true);

	return active;
}

void C_MetaverseManager::SetLibraryBrowserContext(std::string category, std::string id, std::string search, std::string filter)
{
	m_libraryBrowserContextCategory = category;
	m_libraryBrowserContextId = id;
	m_libraryBrowserContextSearch = search;
	m_libraryBrowserContextFilter = filter;
}

std::string C_MetaverseManager::GetLibraryBrowserContext(std::string name)
{
	if (name == "category")
		return m_libraryBrowserContextCategory;
	else if (name == "id")
		return m_libraryBrowserContextId;
	else if (name == "filter")
		return m_libraryBrowserContextFilter;
	else if (name == "search")
		return m_libraryBrowserContextSearch;

	DevMsg("Unhandled Error: Invalid context variable name.\n");
	return "contextError";
}

void C_MetaverseManager::ScaleObject(C_PropShortcutEntity* pEntity, int iDelta)
{
	float goodScale = pEntity->GetModelScale() + 0.1f * iDelta;

//	C_PropShortcutEntity* pEntity = this->GetSpawningObjectEntity();
	engine->ServerCmd(VarArgs("setscale %i %f", pEntity->entindex(), goodScale), false);
	/*
	object_t* object = g_pAnarchyManager->GetInstanceManager()->GetInstanceObject(pEntity->GetObjectId());
	if (object)
	{
		object->scale += 0.1f * iDelta;	// NOTE: Changes are made to the object here but aren't saved yet!! (is this ok?)
		C_PropShortcutEntity* pEntity = this->GetSpawningObjectEntity();
		engine->ServerCmd(VarArgs("setscale %i %f", pEntity->entindex(), object->scale), false);
	}
	*/
}

void C_MetaverseManager::SetObjectScale(C_PropShortcutEntity* pEntity, float scale)
{
	engine->ServerCmd(VarArgs("setscale %i %f", pEntity->entindex(), scale), false);
}

int C_MetaverseManager::CycleSpawningRotationAxis(int direction)
{
	m_iSpawningRotationAxis += direction;
	if (m_iSpawningRotationAxis > 2)
		m_iSpawningRotationAxis = 0;
	else if (m_iSpawningRotationAxis < 0)
		m_iSpawningRotationAxis = 2;

	return m_iSpawningRotationAxis;
}

void C_MetaverseManager::ResetSpawningAngles()
{
	m_spawningAngles.x = 0;
	m_spawningAngles.y = 0;
	m_spawningAngles.z = 0;
}

void C_MetaverseManager::GetObjectInfo(object_t* pObject, KeyValues* &pObjectInfo, KeyValues* &pItemInfo, KeyValues* &pModelInfo)
{
	pObjectInfo = new KeyValues("objectInfo");

	pObjectInfo->SetString("id", pObject->objectId.c_str());
	pObjectInfo->SetBool("slave", pObject->slave);
	pObjectInfo->SetFloat("scale", pObject->scale);

	// origin
	char buf[AA_MAX_STRING];
	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", pObject->origin.x, pObject->origin.y, pObject->origin.z);
	std::string origin = buf;
	pObjectInfo->SetString("origin", origin.c_str());

	// angles
	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", pObject->angles.x, pObject->angles.y, pObject->angles.z);
	std::string angles = buf;
	pObjectInfo->SetString("angles", angles.c_str());

	pObjectInfo->SetBool("child", pObject->child);

	C_PropShortcutEntity* pParentShortcut = dynamic_cast<C_PropShortcutEntity*>(C_BaseEntity::Instance(pObject->parentEntityIndex));
	std::string parentObjectId = (pParentShortcut) ? pParentShortcut->GetObjectId() : "";
	pObjectInfo->SetString("parentObject", parentObjectId.c_str());

	KeyValues* pItemKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(pObject->itemId));
	if (pItemKV)
	{
		pItemInfo = new KeyValues("itemInfo");

		pItemInfo->SetString("id", pItemKV->GetString("info/id"));
		pItemInfo->SetString("title", pItemKV->GetString("title"));

		std::string workshopId = pItemKV->GetString(VarArgs("platforms/%s/workshopIds", AA_PLATFORM_ID));
		pItemInfo->SetString("workshopIds", workshopId.c_str());

		// if there is a workshop ID, get more info.
		SteamWorkshopDetails_t* pWorkshopDetails = g_pAnarchyManager->GetWorkshopManager()->GetWorkshopSubscription(Q_atoui64(workshopId.c_str()));
		if (pWorkshopDetails)
			pItemInfo->SetString("workshopTitle", pWorkshopDetails->title.c_str());
		else
			pItemInfo->SetString("workshopTitle", "");

		/*
		KeyValues* someInfoKV = pItemKV->FindKey(VarArgs("platforms/%s", AA_PLATFORM_ID));
		if (someInfoKV)
		{
			for (KeyValues *sub = someInfoKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
				DevMsg("Test value: %s\n", sub->GetName());
		}
		*/
		//pItemInfo->SetString("workshopIds", "TBD");
	}

	KeyValues* pModelKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(pObject->modelId));
	if (pModelKV)
	{
		pModelInfo = new KeyValues("modelInfo");

		pModelInfo->SetString("id", pModelKV->GetString("info/id"));
		pModelInfo->SetString("title", pModelKV->GetString("title"));
		
		KeyValues* someInfoKV = g_pAnarchyManager->GetMetaverseManager()->DetectRequiredWorkshopForModelFile(pModelKV->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID)));
		std::string workshopTitle = "";
		for (KeyValues *sub = someInfoKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			pModelInfo->SetString(sub->GetName(), workshopTitle.c_str());

		std::string workshopId = someInfoKV->GetString("workshopIds");
		someInfoKV->deleteThis();
		SteamWorkshopDetails_t* pWorkshopDetails = g_pAnarchyManager->GetWorkshopManager()->GetWorkshopSubscription(Q_atoui64(workshopId.c_str()));
		pModelInfo->SetString("workshopIds", workshopId.c_str());

		if (pWorkshopDetails)
			pModelInfo->SetString("workshopTitle", pWorkshopDetails->title.c_str());
		else
			pModelInfo->SetString("workshopTitle", "");

		// backpack stuff
		std::string backpackTitle;
		std::string backpackId = g_pAnarchyManager->GetBackpackManager()->DetectRequiredBackpackForModelFile(pModelKV->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID)));
		if (backpackId != "")
		{
			C_Backpack* pBackpack = g_pAnarchyManager->GetBackpackManager()->GetBackpack(backpackId);
			backpackTitle = pBackpack->GetTitle();
		}

		pModelInfo->SetString("backpackIds", backpackId.c_str());
		pModelInfo->SetString("backpackTitle", backpackTitle.c_str());

		/*
		std::string workshopId = pModelKV->GetString(VarArgs("platforms/%s/workshopIds", AA_PLATFORM_ID));
		pModelInfo->SetString("workshopIds", workshopId.c_str());

		// if there is a workshop ID, get more info.
		SteamWorkshopDetails_t* pWorkshopDetails = g_pAnarchyManager->GetWorkshopManager()->GetWorkshopSubscription(Q_atoui64(workshopId.c_str()));
		if (pWorkshopDetails)
			pModelInfo->SetString("workshopTitle", pWorkshopDetails->title.c_str());
		else
			pModelInfo->SetString("workshopTitle", "");*/

		//pModelInfo->SetString("mountIds", "TBD");
		
		//pModelInfo->SetString("file", pModelKV->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID)));
	}
}

void C_MetaverseManager::UserInfoReceived(HTTPRequestCompleted_t* pResult, bool bIOFailure)
{
	//C_AwesomiumBrowserInstance* pNetworkInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetNetworkAwesomiumBrowserInstance();
	//if (!pNetworkInstance)
	//	return;

	//uint8* pBuf = new uint8(pResult->m_unBodySize);
	void* pBuf = malloc(pResult->m_unBodySize);
	steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pResult->m_hRequest, (uint8*)pBuf, pResult->m_unBodySize);

	std::string avatarURL = VarArgs("%s", (unsigned char*)pBuf);
	free(pBuf);
	//delete[] pBuf;
	steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pResult->m_hRequest);

	size_t found = avatarURL.find("\"avatarfull\": \"");
	if (found != std::string::npos)
		avatarURL = avatarURL.substr(found + 15);

	found = avatarURL.find("\"");
	if (found != std::string::npos)
		avatarURL = avatarURL.substr(0, found);

	cvar->FindVar("avatar_url")->SetValue(avatarURL.c_str());
	m_bHostSessionNow = true;
}

void C_MetaverseManager::HostSessionNow()
{
	m_bHostSessionNow = false;

	// if we need to extract the overview, now's the time.
	bool bShouldHostNow = true;
	if (cvar->FindVar("sync_overview")->GetBool())
	{
		// 1. resource/overviews/[mapname].txt must exist
		// 2. materials/overviews/[mapname].vtf must exist
		if (g_pFullFileSystem->FileExists(VarArgs("resource/overviews/%s.txt", g_pAnarchyManager->MapName()), "GAME") && g_pFullFileSystem->FileExists(VarArgs("materials/overviews/%s.vtf", g_pAnarchyManager->MapName()), "GAME"))
		{
			bShouldHostNow = false;
			this->ExtractOverviewTGA();
		}
	}

	if ( bShouldHostNow)
		this->ReallyHostNow();
}

void C_MetaverseManager::ReallyHostNow()
{
	C_AwesomiumBrowserInstance* pNetworkInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetNetworkAwesomiumBrowserInstance();
	if (!pNetworkInstance)
		return;

	std::vector<std::string> args;
	args.push_back(cvar->FindVar("avatar_url")->GetString());
	args.push_back("");
	args.push_back(cvar->FindVar("aamp_server_key")->GetString());
	args.push_back(g_pAnarchyManager->GetInstanceId());
	args.push_back(cvar->FindVar("aamp_client_id")->GetString());
	args.push_back(cvar->FindVar("aamp_client_key")->GetString());
	const char *buf = steamapicontext->SteamFriends()->GetPersonaName();;

	std::string personaName = buf;
	if (personaName == "")
		personaName = "Human Player";

	args.push_back(personaName);
	//args.push_back(cvar->FindVar("aamp_display_name")->GetString());

	args.push_back(cvar->FindVar("aamp_lobby_id")->GetString());
	args.push_back(cvar->FindVar("aamp_public")->GetString());
	args.push_back(cvar->FindVar("aamp_lobby_password")->GetString());

	pNetworkInstance->DispatchJavaScriptMethod("aampNetwork", "hostSession", args);
}

void C_MetaverseManager::HostSession()
{
	//DevMsg("Do it from here now!\n");
	C_AwesomiumBrowserInstance* pNetworkInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetNetworkAwesomiumBrowserInstance();
	if (!pNetworkInstance)
		return;

	m_bHasDisconnected = false;

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	pHudBrowserInstance->AddHudLoadingMessage("", "", "Connecting to server...", "connectingToServer", "", "", "", "");


	if (!Q_strcmp(cvar->FindVar("avatar_url")->GetString(), ""))
	{
		CSteamID sid = steamapicontext->SteamUser()->GetSteamID();
		HTTPRequestHandle requestHandle = steamapicontext->SteamHTTP()->CreateHTTPRequest(k_EHTTPMethodGET, VarArgs("https://api.steampowered.com/ISteamUser/GetPlayerSummaries/v2/?key=03125303B0E40F94D4A758C0F730F546&steamids=%llu", sid.ConvertToUint64()));

		SteamAPICall_t hAPICall;
		steamapicontext->SteamHTTP()->SendHTTPRequest(requestHandle, &hAPICall);
		m_UserInfoCallback.Set(hAPICall, this, &C_MetaverseManager::UserInfoReceived);
	}
	else
		this->HostSessionNow();
}

void C_MetaverseManager::ForgetAvatarDeathList()
{
	m_avatarDeathList.clear();
}

void C_MetaverseManager::RestartNetwork(bool bCleanupAvatars)
{
	//DevMsg("Do it from here now!\n");
	C_AwesomiumBrowserInstance* pNetworkInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetNetworkAwesomiumBrowserInstance();
	if (!pNetworkInstance)
		return;

	this->RemoveAllInstanceUsers();

	if (!bCleanupAvatars)
		this->ForgetAvatarDeathList();

	m_bHasDisconnected = false;
	g_pAnarchyManager->ClearConnectedUniverse();
	m_say = "";
	m_followingUserId = "";
	m_pLocalUser = null;
	pNetworkInstance->SetUrl("asset://ui/network.html");
}

void C_MetaverseManager::SendObjectRemoved(object_t* object)
{
	if (!g_pAnarchyManager->GetConnectedUniverse() || !g_pAnarchyManager->GetConnectedUniverse()->connected)
		return;

	C_AwesomiumBrowserInstance* pNetworkBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetNetworkAwesomiumBrowserInstance();
	if (!pNetworkBrowserInstance)
		return;

	/*
	0 - object (string)
	1 - instance (string)
	2 - item (string)
	3 - model (string)
	4 - slave (int)
	5 - child (int)
	6 - parentObject (string)
	7 - scale (number)
	8 - origin (string)
	9 - angles (string)
	*/

	std::vector<std::string> args;
	args.push_back(object->objectId);
	args.push_back(g_pAnarchyManager->GetInstanceId());
	args.push_back(object->itemId);
	args.push_back(object->modelId);
	args.push_back(VarArgs("%i", object->slave));
	args.push_back(VarArgs("%i", object->child));

	std::string parentObjectId;
	C_PropShortcutEntity* pParentShortcut = null;
	if (object->parentEntityIndex >= 0)
	{
		pParentShortcut = dynamic_cast<C_PropShortcutEntity*>(C_BaseEntity::Instance(object->parentEntityIndex));
		if (pParentShortcut)
			parentObjectId = pParentShortcut->GetObjectId();
	}
	args.push_back(parentObjectId);

	args.push_back(VarArgs("%.10f", object->scale));

	char buf[AA_MAX_STRING];
	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", object->origin.x, object->origin.y, object->origin.z);
	std::string origin = buf;
	args.push_back(origin);

	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", object->angles.x, object->angles.y, object->angles.z);
	std::string angles = buf;
	args.push_back(angles);

	pNetworkBrowserInstance->DispatchJavaScriptMethod("aampNetwork", "localObjectRemove", args);
}

void C_MetaverseManager::SendLocalChatMsg(std::string chatText)
{
	m_say = chatText;
	g_pAnarchyManager->AddToastMessage(VarArgs("You say: %s", chatText.c_str()));
}

void C_MetaverseManager::InstanceUserClicked(user_t* pUser)
{
	if (m_followingUserId == pUser->userId)
	{
		// we were already followed
		if (pUser->entity)
			engine->ServerCmd(VarArgs("removehovergloweffect %i", pUser->entity->entindex()), false);

		m_followingUserId = "";
		g_pAnarchyManager->AddToastMessage(VarArgs("Stopped following %s.", pUser->displayName.c_str()));
	}
	else
	{
		// if we are already following a user, unfollow!
		if (m_followingUserId != "")
		{
			user_t* pFollowedUser = this->GetInstanceUser(m_followingUserId);
			if (pFollowedUser)
			{
				if (pFollowedUser->entity)
					engine->ServerCmd(VarArgs("removehovergloweffect %i", pFollowedUser->entity->entindex()), false);
			}
		}

		if (pUser->entity)
			engine->ServerCmd(VarArgs("addhovergloweffect %i", pUser->entity->entindex()), false);

		m_followingUserId = pUser->userId;
		g_pAnarchyManager->AddToastMessage(VarArgs("Started following %s.", pUser->displayName.c_str()));

		this->SyncToUser(pUser->objectId, "");
	}
}

void C_MetaverseManager::AvatarObjectCreated(int iEntIndex, std::string userId)
{
	C_DynamicProp* pProp = dynamic_cast<C_DynamicProp*>(C_BaseEntity::Instance(iEntIndex));
	if (!pProp)
	{
		DevMsg("ERROR: Could not obtain avatar prop.\n");
		return;
	}

	user_t* pUser = this->GetInstanceUser(userId);
	if (!pUser)
	{
		DevMsg("ERROR: Could not obtain the user for the avatar.\n");
		return;
	}

	if (pUser->entity)
	{
		DevMsg("ERROR: User already has an entity.\n");
		return;
	}

	pUser->entity = pProp;
	///*
	float fPlayerHeight = 60.0f;
	C_BasePlayer* pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	float fFudgeHeight = pLocalPlayer->GetAbsOrigin().z + fPlayerHeight;
	//fFudgeHeight += pLocalPlayer->EyePosition().z;

	engine->ServerCmd(VarArgs("set_object_pos %i %f %f %f %f %f %f;\n", pUser->entity->entindex(), pUser->bodyOrigin.x, pUser->bodyOrigin.y, fFudgeHeight, pUser->bodyAngles.x, pUser->bodyAngles.y, pUser->bodyAngles.z), false);
	//*/
}

void C_MetaverseManager::SendEntryUpdate(std::string mode, std::string entryId)
{
	if (!g_pAnarchyManager->GetConnectedUniverse() || !g_pAnarchyManager->GetConnectedUniverse()->connected)
		return;

	C_AwesomiumBrowserInstance* pNetworkBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetNetworkAwesomiumBrowserInstance();
	if (!pNetworkBrowserInstance)
		return;

	std::vector<std::string> args;
	if (mode == "Item")
	{
		KeyValues* pEntryKV = this->GetActiveKeyValues(this->GetLibraryItem(entryId));

		if (!pEntryKV)
		{
			DevMsg("FATAL ERROR: Could not get entry to send update for!\n");
			return;
		}

		/*
			0 - mode (string)
			1 - item (string)
			2 - app (string)
			3 - description (string)
			4 - download (string)
			5 - file (string)
			6 - marquee (string)
			7 - preview (string)
			8 - reference (string)
			9 - screen (string)
			10 - stream (string)
			11 - title (string)
			12 - type (string)
		*/

		args.push_back(mode);
		args.push_back(entryId);
		args.push_back(pEntryKV->GetString("app"));
		args.push_back(pEntryKV->GetString("description"));
		args.push_back(pEntryKV->GetString("download"));
		args.push_back(pEntryKV->GetString("file"));
		args.push_back(pEntryKV->GetString("marquee"));
		args.push_back(pEntryKV->GetString("preview"));
		args.push_back(pEntryKV->GetString("reference"));
		args.push_back(pEntryKV->GetString("screen"));
		args.push_back(pEntryKV->GetString("stream"));
		args.push_back(pEntryKV->GetString("title"));
		args.push_back(pEntryKV->GetString("type"));
	}
	else if (mode == "Model")
	{
		KeyValues* pEntryKV = this->GetActiveKeyValues(this->GetLibraryModel(entryId));

		if (!pEntryKV)
		{
			DevMsg("FATAL ERROR: Could not get entry to send update for!\n");
			return;
		}

		/*
			0 - mode
			1 - model (string)
			2 - dynamic (int)
			3 - keywords (string)
			4 - file (string)
			5 - mountIds (string)
			6 - workshopIds (string)
			7 - title (string)
			8 - screen (string)
			9 - preview (string)
			10 - download (string)
		*/

		args.push_back(mode);
		args.push_back(entryId);
		args.push_back(pEntryKV->GetString("dynamic"));
		args.push_back(pEntryKV->GetString("keywords"));
		args.push_back(pEntryKV->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID)));
		args.push_back(pEntryKV->GetString(VarArgs("platforms/%s/mountIds", AA_PLATFORM_ID)));
		args.push_back(pEntryKV->GetString(VarArgs("platforms/%s/workshopIds", AA_PLATFORM_ID)));
		args.push_back(pEntryKV->GetString("title"));
		args.push_back(pEntryKV->GetString("screen"));
		args.push_back(pEntryKV->GetString("preview"));
		args.push_back(pEntryKV->GetString(VarArgs("platforms/%s/download", AA_PLATFORM_ID)));
	}
	else if (mode == "App")
	{
		KeyValues* pEntryKV = this->GetActiveKeyValues(this->GetLibraryApp(entryId));

		if (!pEntryKV)
		{
			DevMsg("FATAL ERROR: Could not get entry to send update for!\n");
			return;
		}

		/*
			0 - mode
			1 - app (string)
			2 - title (string)
			3 - file (string)
			4 - commandFormat (string)
			5 - type (string)
			6 - download (string)
			7 - reference (string)
			8 - screen (string)commandFormat
			9 - description (string)
			10 - filepaths (string)
		*/

		args.push_back(mode);
		args.push_back(entryId);
		args.push_back(pEntryKV->GetString("title"));
		args.push_back(pEntryKV->GetString("file"));
		args.push_back(pEntryKV->GetString("commandformat"));
		args.push_back(pEntryKV->GetString("type"));
		args.push_back(pEntryKV->GetString("download"));
		args.push_back(pEntryKV->GetString("reference"));
		args.push_back(pEntryKV->GetString("screen"));
		args.push_back(pEntryKV->GetString("description"));
		args.push_back(pEntryKV->GetString("filepaths"));
	}
	else if (mode == "Type")
	{
		KeyValues* pEntryKV = this->GetActiveKeyValues(this->GetLibraryType(entryId));

		if (!pEntryKV)
		{
			DevMsg("FATAL ERROR: Could not get entry to send update for!\n");
			return;
		}

		/*
			0 - mode
			1 - type (string)
			2 - fileformat (string)
			3 - titleformat (string)
			4 - title (string)
			5 - priority (string)
		*/

		args.push_back(mode);
		args.push_back(entryId);
		args.push_back(pEntryKV->GetString("fileformat"));
		args.push_back(pEntryKV->GetString("titleformat"));
		args.push_back(pEntryKV->GetString("title"));
		args.push_back(pEntryKV->GetString("priority"));
	}

	pNetworkBrowserInstance->DispatchJavaScriptMethod("aampNetwork", "localEntryUpdate", args);

}

void C_MetaverseManager::SendObjectUpdate(C_PropShortcutEntity* pShortcut)
{
	if (!g_pAnarchyManager->GetConnectedUniverse() || !g_pAnarchyManager->GetConnectedUniverse()->connected)
		return;
	
	C_AwesomiumBrowserInstance* pNetworkBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetNetworkAwesomiumBrowserInstance();
	if (!pNetworkBrowserInstance)
		return;

	std::string objectId = pShortcut->GetObjectId();
	object_t* object = g_pAnarchyManager->GetInstanceManager()->GetInstanceObject(objectId);
	if (!object)
	{
		DevMsg("FATAL ERROR: This shortcut has no object data struct here!\n");
		return;
	}

	/*
		0 - object (string)
		1 - instance (string)
		2 - item (string)
		3 - model (string)
		4 - slave (int)
		5 - child (int)
		6 - parentObject (string)
		7 - scale (number)
		8 - origin (string)
		9 - angles (string)
	*/

	std::vector<std::string> args;
	args.push_back(object->objectId);
	args.push_back(g_pAnarchyManager->GetInstanceId());
	args.push_back(object->itemId);
	args.push_back(object->modelId);
	args.push_back(VarArgs("%i", object->slave));
	args.push_back(VarArgs("%i", object->child));

	std::string parentObjectId;
	C_PropShortcutEntity* pParentShortcut = null;
	if (object->parentEntityIndex >= 0)
	{
		pParentShortcut = dynamic_cast<C_PropShortcutEntity*>(C_BaseEntity::Instance(object->parentEntityIndex));
		if (pParentShortcut)
			parentObjectId = pParentShortcut->GetObjectId();
	}
	args.push_back(parentObjectId);

	args.push_back(VarArgs("%.10f", object->scale));
	
	char buf[AA_MAX_STRING];
	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", object->origin.x, object->origin.y, object->origin.z);
	std::string origin = buf;
	args.push_back(origin);

	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", object->angles.x, object->angles.y, object->angles.z);
	std::string angles = buf;
	args.push_back(angles);
	
	pNetworkBrowserInstance->DispatchJavaScriptMethod("aampNetwork", "localObjectUpdate", args);
}

user_t* C_MetaverseManager::GetInstanceUser(std::string userId)
{
	auto it = m_users.find(userId);
	if (it != m_users.end())
		return it->second;

	return null;
}

void C_MetaverseManager::GetAllInstanceUsers(std::vector<user_t*>& users)
{
	auto it = m_users.begin();
	while (it != m_users.end())
	{
		users.push_back(it->second);
		it++;
	}
}

user_t* C_MetaverseManager::FindInstanceUser(C_DynamicProp* pProp)
{
	auto it = m_users.begin();
	while (it != m_users.end())
	{
		if (it->second->entity == pProp)
			return it->second;
		else
			it++;
	}

	return null;
}

void C_MetaverseManager::RemoveInstanceUser(std::string userId)
{
	auto it = m_users.find(userId);
	if (it != m_users.end())
	{
		user_t* pUser = it->second;

		//if (g_pAnarchyManager->GetInputManager()->GetMainMenuMode())
		//	DevMsg("ERROR: Could not remove entity for user because game is paused!\n");
		//else
			this->RemoveInstanceUser(pUser);

		m_users.erase(it);
	}
}

void C_MetaverseManager::RemoveInstanceUser(user_t* pUser)
{
	//DevMsg("Removing %s...\n", pUser->displayName.c_str());

	if (pUser->entity)
	{
		m_avatarDeathList.push_back(pUser->entity);
		pUser->entity = null;
	}

	// check if we have a pending user update.
	auto it = m_pendingUserUpdates.find(pUser->userId);
	if (it != m_pendingUserUpdates.end())
	{
		delete it->second;
		m_pendingUserUpdates.erase(it);
	}

	//CDynamicProp* pProp = pUser->entity;
	////if (pProp)
	//{
	//	engine->ServerCmd(VarArgs("removeobject %i;\n", pProp->entindex()), false);
	//}

	delete pUser;
}

void C_MetaverseManager::RemoveAllInstanceUsers()
{
	std::vector<user_t*> victims;
	auto it = m_users.begin();
	while (it != m_users.end())
	{
		victims.push_back(it->second);
		it++;
	}

	for (unsigned int i = 0; i < victims.size(); i++)
		this->RemoveInstanceUser(victims[i]);

	m_users.clear();
}

unsigned int C_MetaverseManager::GetNumInstanceUsers()
{
	return m_users.size();
}

void C_MetaverseManager::InstanceUserRemoved(std::string userId)
{
	user_t* pUser = this->GetInstanceUser(userId);

	if (m_followingUserId == userId)
	{
		m_followingUserId = "";
		g_pAnarchyManager->AddToastMessage(VarArgs("Stopped following %s.", pUser->displayName.c_str()));
	}

	g_pAnarchyManager->AddToastMessage(VarArgs("%s has LEFT the session.", pUser->displayName.c_str()));
	this->RemoveInstanceUser(userId);
}

void C_MetaverseManager::InstanceUserAddedReceived(std::string userId, std::string sessionId, std::string displayName)
{
	// does this user exist?
	user_t* pUser = this->GetInstanceUser(userId);
	if (pUser)
	{
		DevMsg("ERROR: User already exists.\n");
		return;
	}
	else
	{
		user_t* pUser = new user_t();
		//pUser->instanceId = instanceId;
		pUser->sessionId = sessionId;
		pUser->userId = userId;
		pUser->displayName = displayName;
		pUser->entity = null;
		pUser->sessionId = (g_pAnarchyManager->GetConnectedUniverse()) ? g_pAnarchyManager->GetConnectedUniverse()->session : "";
		pUser->needsEntity = true;

		m_users[userId] = pUser;

		if (!Q_strcmp(cvar->FindVar("aamp_client_id")->GetString(), userId.c_str()))
		{
			m_pLocalUser = pUser;
			pUser->avatarUrl = std::string(cvar->FindVar("avatar_url")->GetString());
		}
		else
			g_pAnarchyManager->AddToastMessage(VarArgs("%s has JOINED the session.", displayName.c_str()));
	}
}

void C_MetaverseManager::SyncToUser(std::string objectId, std::string oldObjectId)
{
	if (oldObjectId != "")
	{
		// 1. find any embedded instance that uses the OLD objectId
		// 2. close all matches.  if a match is the currently selected shortcut, de-select it 1st.

		object_t* pOldObject = g_pAnarchyManager->GetInstanceManager()->GetInstanceObject(oldObjectId);
		if (pOldObject)
		{
			std::string oldItemId = pOldObject->itemId;
			std::string oldTabId = "auto" + oldItemId;
			C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetCanvasManager()->FindEmbeddedInstance(oldTabId);
			if (pEmbeddedInstance)
			{
				// we have found the victim.  the only reason ever to NOT close it right away is if it's the selected entity for us.
				//if (g_pAnarchyManager->GetInputManager()->GetInputMode() && g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pEmbeddedInstance)
				C_PropShortcutEntity* pOldShortcut = dynamic_cast<C_PropShortcutEntity*>(g_pAnarchyManager->GetSelectedEntity());
				if (pOldShortcut && pOldShortcut->entindex() == pEmbeddedInstance->GetOriginalEntIndex())
				{
					g_pAnarchyManager->DeselectEntity();
				}
				else
					pEmbeddedInstance->Close();
			}
		}
	}

	if (objectId != "")
	{
		// 1. find the object
		// 2. put it on continuous play (without selecting it 1st) :S

		object_t* pObject = g_pAnarchyManager->GetInstanceManager()->GetInstanceObject(objectId);
		if (pObject)
		{
			C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(C_BaseEntity::Instance(pObject->entityIndex));

			// only if shortcut is an item
			if (pShortcut && pShortcut->GetItemId() != pShortcut->GetModelId() && pShortcut->GetItemId() != "" && pShortcut->GetModelId() != "")
			{
				//g_pAnarchyManager->AttemptSelectEntity(pShortcut);

				// FIXME: This is redundant code.  It's also in the selectEntity method!
				// TODO: Generalize this into a method of g_pAnarchymanager!!

				std::string tabTitle = "auto" + pShortcut->GetItemId();
				C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetCanvasManager()->FindEmbeddedInstance(tabTitle);// this->GetWebManager()->FindWebTab(tabTitle);
				if (!pEmbeddedInstance)
				{
					std::string itemId = pShortcut->GetItemId();
					KeyValues* item = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(itemId);
					if (item)
					{
						KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(item);

						bool bDoAutoInspect = true;

						std::string gameFile = "";
						std::string coreFile = "";
						bool bShouldLibretroLaunch = (g_pAnarchyManager->DetermineLibretroCompatible(item, gameFile, coreFile) && g_pAnarchyManager->GetLibretroManager()->GetInstanceCount() == 0);

						// auto-libretro
						if (cvar->FindVar("auto_libretro")->GetBool() && bShouldLibretroLaunch && g_pFullFileSystem->FileExists(gameFile.c_str()))
						{
							C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->CreateLibretroInstance();
							pLibretroInstance->Init(tabTitle, VarArgs("%s - Libretro", active->GetString("title", "Untitled")), pShortcut->entindex());
							DevMsg("Setting game to: %s\n", gameFile.c_str());
							pLibretroInstance->SetOriginalGame(gameFile);
							pLibretroInstance->SetOriginalItemId(itemId);
							pLibretroInstance->SetOriginalEntIndex(pShortcut->entindex());	// probably NOT needed?? (or maybe so, from here.)
							if (!pLibretroInstance->LoadCore(coreFile))	// FIXME: elegantly revert back to autoInspect if loading the core failed!
								DevMsg("ERROR: Failed to load core: %s\n", coreFile.c_str());
							pEmbeddedInstance = pLibretroInstance;
							bDoAutoInspect = false;
						}

						if (bDoAutoInspect)
						{
							std::string uri = "file://";
							uri += engine->GetGameDirectory();
							uri += "/resource/ui/html/autoInspectItem.html?imageflags=" + g_pAnarchyManager->GetAutoInspectImageFlags() + "&id=" + g_pAnarchyManager->encodeURIComponent(itemId) + "&title=" + g_pAnarchyManager->encodeURIComponent(active->GetString("title")) + "&screen=" + g_pAnarchyManager->encodeURIComponent(active->GetString("screen")) + "&marquee=" + g_pAnarchyManager->encodeURIComponent(active->GetString("marquee")) + "&preview=" + g_pAnarchyManager->encodeURIComponent(active->GetString("preview")) + "&reference=" + g_pAnarchyManager->encodeURIComponent(active->GetString("reference")) + "&file=" + g_pAnarchyManager->encodeURIComponent(active->GetString("file"));

							C_SteamBrowserInstance* pSteamBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->CreateSteamBrowserInstance();
							pSteamBrowserInstance->Init(tabTitle, uri, "Newly selected item...", null, pShortcut->entindex());
							pSteamBrowserInstance->SetOriginalItemId(itemId);	// FIXME: do we need to do this for original entindex too???
							pSteamBrowserInstance->SetOriginalEntIndex(pShortcut->entindex());	// probably NOT needed?? (or maybe so, from here.)

							pEmbeddedInstance = pSteamBrowserInstance;
						}

						if (pEmbeddedInstance)
							g_pAnarchyManager->GetCanvasManager()->SetDisplayInstance(pEmbeddedInstance);
					}
				}
				else
					g_pAnarchyManager->GetCanvasManager()->SetDisplayInstance(pEmbeddedInstance);
			}
		}
	}
}

user_update_t* C_MetaverseManager::FindPendingUserUpdate(std::string userId)
{
	auto it = m_pendingUserUpdates.find(userId);
	if (it != m_pendingUserUpdates.end())
		return it->second;

	return null;
}

void C_MetaverseManager::UserSessionUpdated(int iUpdateMask, std::string userId, std::string sessionId, std::string displayName, std::string itemId, std::string objectId, std::string say, std::string bodyOrigin, std::string bodyAngles, std::string headOrigin, std::string headAngles, std::string mouseX, std::string mouseY, std::string webUrl, std::string avatarUrl)
{
	//user_t* pUser = this->GetInstanceUser(userId);
	//if (!pUser)
	//{
		// FIXME: This should be handled very differently after pendingUserUpdates is fully implemented!  User joined / exited messages would be nice to just be special regular session updates in that case, and ALL user session events could be added to the pending list instead.  User IDs will remain constant between user sessions, even if they leave & rejoin.




		/*
		// TODO: This should call UserJoined, and the InstanceUserAdded should do absolutely nothing.
		DevMsg("User session updated for user not yet added to the session: hasOrigin(%i) hasAngles(%i)\n", iUpdateMask & 0x40, iUpdateMask & 0x80);

		pUser = new user_t();
		//pUser->instanceId = instanceId;
		pUser->sessionId = sessionId;
		pUser->userId = userId;
		pUser->displayName = displayName;
		pUser->entity = null;
		pUser->needsEntity = true;

		m_users[userId] = pUser;
		*/
		//g_pAnarchyManager->AddToastMessage(VarArgs("%s has JOINED the session.", displayName.c_str()));

	//	return;	// just return, for now.
	//}

	// find the pending update for this user, if one exists.
	bool bIsPendingUpdate = true;
	user_update_t* pUserUpdate = this->FindPendingUserUpdate(userId);
	if (!pUserUpdate)
	{
		bIsPendingUpdate = false;
		pUserUpdate = new user_update_t();	// create a new update for this user if one doesn't exist already.
	}

	// populate our update
	if (iUpdateMask & 0x1)
		pUserUpdate->userId = userId;

	if (iUpdateMask & 0x2)
		pUserUpdate->sessionId = sessionId;

	if (iUpdateMask & 0x4)
		pUserUpdate->displayName = displayName;

	if (iUpdateMask & 0x8)
		pUserUpdate->itemId = itemId;

	if (iUpdateMask & 0x10)
		pUserUpdate->objectId = objectId;

	if (iUpdateMask & 0x20)
		pUserUpdate->say = say;

	if (iUpdateMask & 0x40)
		pUserUpdate->bodyOrigin = bodyOrigin;

	if (iUpdateMask & 0x80)
		pUserUpdate->bodyAngles = bodyAngles;

	if (iUpdateMask & 0x100)
		pUserUpdate->headOrigin = headOrigin;

	if (iUpdateMask & 0x200)
		pUserUpdate->headAngles = headAngles;

	if (iUpdateMask & 0x400)
		pUserUpdate->mouseX = mouseX;

	if (iUpdateMask & 0x800)
		pUserUpdate->mouseY = mouseY;

	if (iUpdateMask & 0x1000)
		pUserUpdate->webUrl = webUrl;

	if (iUpdateMask & 0x2000)
		pUserUpdate->avatarUrl = avatarUrl;

	// set/update our update mask
	pUserUpdate->updateMask |= iUpdateMask;

	// now we have a valid user update to process (if we are allowed)
	if (!bIsPendingUpdate && !engine->IsPaused() && !g_pAnarchyManager->IsPaused() && !g_pAnarchyManager->GetInputManager()->GetMainMenuMode() && this->m_pendingUserUpdates.size() == 0)
		this->ProcessUserSessionUpdate(pUserUpdate);
	else
		m_pendingUserUpdates[userId] = pUserUpdate;
}

bool C_MetaverseManager::ProcessUserSessionUpdate(user_update_t* pUserUpdate)
{
	if (g_pAnarchyManager->IsPaused() || engine->IsPaused() )//g_pAnarchyManager->GetInputManager()->GetMainMenuMode())
	{
		//DevMsg("Mark\n");
		//DevMsg("ERROR: ProcessUserSessionUpdate called while AArcade was paused!!\n");
		//delete pUserUpdate;
		return false;
	}

	user_t* pUser = this->GetInstanceUser(pUserUpdate->userId);
	if (!pUser)
	{
		DevMsg("ERROR: User NOT found for session update processing: %s\n", pUserUpdate->userId.c_str());
		//delete pUserUpdate;
		return false;
	}

	if (pUserUpdate->updateMask & 0x20)
	{
		if (pUserUpdate->say != "")
			g_pAnarchyManager->AddToastMessage(VarArgs("%s says: %s", pUser->displayName.c_str(), pUserUpdate->say.c_str()));

		pUser->say = pUserUpdate->say;
	}

	if (pUserUpdate->updateMask & 0x40 || pUserUpdate->updateMask & 0x80 || pUserUpdate->updateMask & 0x100 || pUserUpdate->updateMask & 0x200)
	{
		if (pUserUpdate->bodyOrigin != "")
			UTIL_StringToVector(pUser->bodyOrigin.Base(), pUserUpdate->bodyOrigin.c_str());

		if (pUserUpdate->bodyAngles != "")
			UTIL_StringToVector(pUser->bodyAngles.Base(), pUserUpdate->bodyAngles.c_str());

		if (pUserUpdate->headOrigin != "")
			UTIL_StringToVector(pUser->headOrigin.Base(), pUserUpdate->headOrigin.c_str());

		if (pUserUpdate->headAngles != "")
			UTIL_StringToVector(pUser->headAngles.Base(), pUserUpdate->headAngles.c_str());

		if (pUser->entity)
		{
			float fPlayerHeight = 60.0f;
			C_BasePlayer* pLocalPlayer = C_BasePlayer::GetLocalPlayer();
			float fFudgeHeight = pLocalPlayer->GetAbsOrigin().z + fPlayerHeight;
			//fFudgeHeight += pLocalPlayer->EyePosition().z;

			engine->ServerCmd(VarArgs("set_object_pos %i %f %f %f %f %f %f;\n", pUser->entity->entindex(), pUser->bodyOrigin.x, pUser->bodyOrigin.y, fFudgeHeight, pUser->bodyAngles.x, pUser->bodyAngles.y, pUser->bodyAngles.z), false);
		}
		else if (pUser->needsEntity)
		{
			pUser->needsEntity = false;

			/*
			1 - modelFile
			2 - origin X
			3 - origin Y
			4 - origin Z
			5 - angles P
			6 - angles Y
			7 - angles R
			8 - userId
			*/
			std::vector<std::string> modelNames;
			modelNames.push_back("models/players/heads/cowboycarl.mdl");
			modelNames.push_back("models/players/heads/flipflopfred.mdl");
			modelNames.push_back("models/players/heads/hackerhaley.mdl");
			modelNames.push_back("models/players/heads/ninjanancy.mdl");
			modelNames.push_back("models/players/heads/zombiejoe.mdl");

			unsigned int index = rand() % modelNames.size();	// non-uniform, but who cares :S
			std::string modelName = modelNames[index];

			float fPlayerHeight = 60.0f;
			C_BasePlayer* pLocalPlayer = C_BasePlayer::GetLocalPlayer();
			Vector localPlayerOrigin = pLocalPlayer->GetAbsOrigin();
			float fFudgeHeight = localPlayerOrigin.z + fPlayerHeight;
			//float x = localPlayerOrigin.x;//pUser->bodyOrigin.x
			//float y = localPlayerOrigin.y;//pUser->bodyOrigin.y
			float x = pUser->bodyOrigin.x;
			float y = pUser->bodyOrigin.y;
			float z = fFudgeHeight;// localPlayerOrigin.z;//pUser->bodyOrigin.z

			// FIXME: Need to make this use same logic as choosing a player spawn point, in case the local player is inside of a wall when a new player joins.

			engine->ServerCmd(VarArgs("create_avatar_object \"%s\" %f %f %f %f %f %f \"%s\";\n", modelName.c_str(), x, y, z, pUser->bodyAngles.x, pUser->bodyAngles.y, pUser->bodyAngles.z, pUserUpdate->userId.c_str()), false);
			//g_pAnarchyManager->AddToastMessage(VarArgs("%s has JOINED the session.", pUser->displayName.c_str()));
		}
		// else there's no entity to do anything with yet.
	}

	if (pUserUpdate->updateMask & 0x4 && pUserUpdate->displayName != pUser->displayName && pUserUpdate->displayName != "" )	// FIXME: We probably don't need to include this with every update anymore. :)
	{
		std::string goodName = (pUserUpdate->displayName != "") ? pUserUpdate->displayName : "Human Player";
		pUser->displayName = goodName;

		g_pAnarchyManager->AddToastMessage(VarArgs("%s changed their name to %s.", pUser->displayName.c_str(), goodName.c_str()));
	}

	if (pUserUpdate->updateMask & 0x10)
	{
		object_t* pObject = g_pAnarchyManager->GetInstanceManager()->GetInstanceObject(pUserUpdate->objectId);
		if (pObject)
		{
			KeyValues* pItemKV = this->GetActiveKeyValues(this->GetLibraryItem(pObject->itemId));
			if (pItemKV)
			{
				std::string msg = VarArgs("%s tuned into %s", pUser->displayName.c_str(), pItemKV->GetString("title"));
				g_pAnarchyManager->AddToastMessage(msg);
			}
		}

		if (m_followingUserId == pUser->userId)
			this->SyncToUser(pUserUpdate->objectId, pUser->objectId);

		pUser->objectId = pUserUpdate->objectId;
	}

	if (pUserUpdate->updateMask & 0x2000 && pUserUpdate->avatarUrl != "")
	{
		pUser->avatarUrl = pUserUpdate->avatarUrl;
	}

	/*
	var maskMap = {
	"userId": 0x1,
	"sessionId": 0x2,
	"displayName": 0x4,
	"item": 0x8,
	"object": 0x10,
	"say": 0x20,
	"bodyOrigin": 0x40,
	"bodyAngles": 0x80,
	"headOrigin": 0x100,
	"headAngles": 0x200,
	"mouseX": 0x400,
	"mouseY": 0x800,
	"web": 0x1000,
	"avatar": 0x2000
	};
	*/

	delete pUserUpdate;	// after an update is processed, it is dead.
	return true;
}

void C_MetaverseManager::SendChangeInstanceNotification(std::string instanceId, std::string map)
{
	C_AwesomiumBrowserInstance* pNetworkBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetNetworkAwesomiumBrowserInstance();
	if (!pNetworkBrowserInstance)
		return;

	std::vector<std::string> args;
	args.push_back(instanceId);
	args.push_back(map);
	pNetworkBrowserInstance->DispatchJavaScriptMethod("aampNetwork", "localUserChangeInstance", args);
}

void C_MetaverseManager::BanSessionUser(std::string userId)
{
	C_AwesomiumBrowserInstance* pNetworkBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetNetworkAwesomiumBrowserInstance();
	if (!pNetworkBrowserInstance)
		return;

	std::vector<std::string> args;
	args.push_back(userId);
	pNetworkBrowserInstance->DispatchJavaScriptMethod("aampNetwork", "banUser", args);
}

void C_MetaverseManager::UnbanSessionUser(std::string userId)
{
	C_AwesomiumBrowserInstance* pNetworkBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetNetworkAwesomiumBrowserInstance();
	if (!pNetworkBrowserInstance)
		return;

	std::vector<std::string> args;
	args.push_back(userId);
	pNetworkBrowserInstance->DispatchJavaScriptMethod("aampNetwork", "unbanUser", args);
}

//#include "../public/bitmap/tgaloader.h"

void C_MetaverseManager::OverviewExtracted()
{
	DevMsg("Overview extracted!!\n");
	this->ReallyHostNow();
	/*
	C_AwesomiumBrowserInstance* pNetworkBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetNetworkAwesomiumBrowserInstance();
	if (!pNetworkBrowserInstance)
		return;

	KeyValues* pOverviewKV = new KeyValues("overview");
	if (pOverviewKV->LoadFromFile(g_pFullFileSystem, VarArgs("resource/overviews/%s.txt", g_pAnarchyManager->MapName()), "GAME"))
	{
		std::vector<std::string> args;
		args.push_back(VarArgs("%s.tga", g_pAnarchyManager->MapName()));
		args.push_back(VarArgs("%i", pOverviewKV->GetInt("pos_x")));
		args.push_back(VarArgs("%i", pOverviewKV->GetInt("pos_y")));
		args.push_back(VarArgs("%.10f", pOverviewKV->GetFloat("scale")));

		pNetworkBrowserInstance->DispatchJavaScriptMethod("aampNetwork", "syncOverview", args);
	}
	*/
}

void C_MetaverseManager::ExtractOverviewTGA()
{
	g_pAnarchyManager->AddToastMessage("Extracting map overview VTF and converting...");

	std::string mapName = g_pAnarchyManager->MapName();
	//std::string textureName = "overviews/" + mapName;

	//ITexture* pTexture = g_pMaterialSystem->FindTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, false, 1);

	//std::string goodTextureName = pTexture->GetName();

	bool bShouldHostNow = true;

	// read the screenshot
	FileHandle_t fh = filesystem->Open(VarArgs("materials/overviews/%s.vtf", mapName.c_str()), "rb", "GAME");
	if (fh)
	{
		int file_len = filesystem->Size(fh);
		unsigned char* pImageData = new unsigned char[file_len + 1];

		filesystem->Read((void*)pImageData, file_len, fh);
		pImageData[file_len] = 0; // null terminator

		filesystem->Close(fh);

		// write the screenshot
		// ORDER: FORWARD, RIGHT, BACK, LEFT, BOTTOM, TOP
		//FileHandle_t fh2 = filesystem->Open(VarArgs("screenshots/panoramic/pano/%s.jpg", directions[i].c_str()), "wb", "DEFAULT_WRITE_PATH");

		g_pFullFileSystem->CreateDirHierarchy("screenshots/overviews", "DEFAULT_WRITE_PATH");

		//std::string textureFile = VarArgs("screenshots/overviews/%s.vtf", mapName.c_str());
		FileHandle_t fh2 = filesystem->Open(VarArgs("screenshots/overviews/%s.vtf", mapName.c_str()), "wb", "DEFAULT_WRITE_PATH");
		if (fh2)
		{
			filesystem->Write(pImageData, file_len, fh2);
			filesystem->Close(fh2);

			// cleanup
			delete[] pImageData;

			std::string toolsFolder = g_pAnarchyManager->GetAArcadeToolsFolder();
			std::string userFolder = g_pAnarchyManager->GetAArcadeUserFolder();

			/*
			std::string command = VarArgs("\"%s\\vtf2tga.exe\" -i \"%s\\screenshots\\overviews\\%s.vtf\" -o \"%s\\screenshots\\overviews\\%s.tga\"", toolsFolder.c_str(), userFolder.c_str(), mapName.c_str(), userFolder.c_str(), mapName.c_str());
			DevMsg("%s\n", command.c_str());
			system(command.c_str());
			*/

			FileHandle_t launch_file = filesystem->Open("Arcade_Launcher.bat", "w", "EXECUTABLE_PATH");
			if (launch_file)
			{
				std::string executable = VarArgs("%s\\vtf2tga.exe", toolsFolder.c_str());
				std::string goodExecutable = "\"" + executable + "\"";
				filesystem->FPrintf(launch_file, "%s:\n", goodExecutable.substr(1, 1).c_str());
				filesystem->FPrintf(launch_file, "cd \"%s\"\n", goodExecutable.substr(1, goodExecutable.find_last_of("/\\", goodExecutable.find("\"", 1)) - 1).c_str());
				filesystem->FPrintf(launch_file, "START \"Launching item...\" %s -i \"%s\\screenshots\\overviews\\%s.vtf\" -o \"%s\\screenshots\\overviews\\%s.tga\"", goodExecutable.c_str(), userFolder.c_str(), mapName.c_str(), userFolder.c_str(), mapName.c_str());
				filesystem->Close(launch_file);
				system("Arcade_Launcher.bat");

				g_pAnarchyManager->WaitForOverviewExtract();
				bShouldHostNow = false;

				/*
				std::string masterCommands = VarArgs(" -i \"%s\\screenshots\\overviews\\%s.vtf\" -o \"%s\\screenshots\\overviews\\%s.tga\"", userFolder.c_str(), mapName.c_str(), userFolder.c_str(), mapName.c_str());

				char pCommands[AA_MAX_STRING];
				Q_strcpy(pCommands, masterCommands.c_str());

				// start the program up
				STARTUPINFO si;
				PROCESS_INFORMATION pi;

				// set the size of the structures
				ZeroMemory(&si, sizeof(si));
				si.cb = sizeof(si);
				ZeroMemory(&pi, sizeof(pi));

				CreateProcess(VarArgs("%s\\vtf2tga.exe", toolsFolder.c_str()),   // the path
				pCommands,        // Command line
				NULL,           // Process handle not inheritable
				NULL,           // Thread handle not inheritable
				FALSE,          // Set handle inheritance to FALSE
				0,//CREATE_DEFAULT_ERROR_MODE,              //0 // No creation flags
				NULL,           // Use parent's environment block
				VarArgs("%s", toolsFolder.c_str()),           // Use parent's starting directory
				&si,            // Pointer to STARTUPINFO structure
				&pi);
				*/







				//Error("Usage: vtf2tga -i <input vtf> [-o <output tga>] [-mip]\n");
				//g_pFullFileSystem->RemoveFile(VarArgs("screenshots/%s", panoshots[i].c_str()), "DEFAULT_WRITE_PATH");
			}
		}
	}

	if (bShouldHostNow)
		this->ReallyHostNow();

	//g_pFullFileSystem->Save

	//FileHandle_t* pFile = g_pFullFileSystem->Open
	//g_pFullFileSystem->FindFirstEx(VarArgs("materials/%s", textureName.c_str()), "GAME", 
	//std::string textureName = "canvas_hud";// "maps/" + mapName;
	//textureName += "/c-1344_1152_112";

//	ITexture* pTexture = g_pMaterialSystem->FindTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, false, 1);
	//pTexture->Download();

	//IVTFTexture* pVTFTexture = CreateVTFTexture();
	//pVTFTexture->Init()
	//pVTFTexture->
	//IVTFTexture::ImageData()
	//int numRes1 = pTexture->GetResourceTypes(NULL, 0);
	//pTexture->GetResourceData()
	//DevMsg("Texture Name: %s\n", pTexture->GetName());

	/*
	int iWidth = pTexture->GetActualWidth();
	int iHeight = pTexture->GetActualHeight();
	int iDepth = pTexture->GetActualDepth();
	ImageFormat format = pTexture->GetImageFormat();
	int size = ImageLoader::GetMemRequired(iWidth, iHeight, iDepth, format, false);
	ImageLoader::Load(&imageData, "D:\Projects\AArcade-Source\game\aarcade_user\crap\overviews\de_dust2.vtf", iWidth, iHeight, format, 1.0, false);
	*/

	/*
	int iInfoWidth = 0;
	int iInfoHeight = 0;
	ImageFormat infoFormat;
	float fGamma;
	if (TGALoader::GetInfo(textureName.c_str(), &iInfoWidth, &iInfoHeight, &infoFormat, &fGamma))
		DevMsg("Fetched stuff is: %i %i\n", iInfoWidth, iInfoHeight);
	else
		DevMsg("Could not fetch.\n");

	//clientdll->WriteSaveGameScreenshotOfSize("testerJoint.jpg", iWidth, iHeight, false, false);
	//#include "../public/bitmap/tgawriter.h"
	//TGAWriter::WriteToBuffer()
	DevMsg("Texture Loaded: %i\n", g_pMaterialSystem->IsTextureLoaded(textureName.c_str()));

	DevMsg("Width: %i\n", pTexture->GetActualWidth());

	size_t byteSize = 0;
	void* result = pTexture->GetResourceData(1, &byteSize);
	DevMsg("Resource Data Gotten: %i (%u)\n", (int)(result != null), byteSize);
	*/

}

//void C_MetaverseManager::OverviewSyncComplete()
//{
//	this->ReallyHostNow();
	/*
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	std::vector<std::string> args;
	pHudBrowserInstance->DispatchJavaScriptMethod("syncListener", "overviewSyncComplete", args);
	*/
//}

void C_MetaverseManager::PanoSyncComplete(std::string cachedPanoName, std::string panoId)
{
	//PanoSyncComplete
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	std::vector<std::string> args;
	args.push_back(cachedPanoName);
	args.push_back(panoId);
	pHudBrowserInstance->DispatchJavaScriptMethod("syncListener", "panoSyncComplete", args);
}

void C_MetaverseManager::SyncPano()
{
	if (!g_pAnarchyManager->GetConnectedUniverse() || !g_pAnarchyManager->GetConnectedUniverse()->connected)
		return;

	C_AwesomiumBrowserInstance* pNetworkBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetNetworkAwesomiumBrowserInstance();
	if (!pNetworkBrowserInstance)
		return;

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	QAngle playerBodyAngles = pPlayer->GetAbsAngles();
	Vector playerBodyOrigin = pPlayer->GetAbsOrigin();

	char buf[AA_MAX_STRING];
	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", playerBodyOrigin.x, playerBodyOrigin.y, playerBodyOrigin.z);
	std::string bodyOrigin = buf;

	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", playerBodyAngles.x, playerBodyAngles.y, playerBodyAngles.z);
	std::string bodyAngles = buf;

	std::vector<std::string> args;
	args.push_back(bodyOrigin);
	args.push_back(bodyAngles);
	args.push_back(VarArgs("%i", ScreenWidth()));
	args.push_back(VarArgs("%i", ScreenHeight()));
	pNetworkBrowserInstance->DispatchJavaScriptMethod("aampNetwork", "syncPano", args);
}

void C_MetaverseManager::ObjectUpdateReceived(bool bIsLocalUserUpdate, bool bIsFreshObject, std::string id, std::string item, std::string model, bool bSlave, bool bChild, std::string parentObject, float fScale, std::string in_origin, std::string in_angles)
{
	Vector origin;
	UTIL_StringToVector(origin.Base(), in_origin.c_str());

	QAngle angles;
	UTIL_StringToVector(angles.Base(), in_angles.c_str());

	// does this object exist?
	object_t* pObject = g_pAnarchyManager->GetInstanceManager()->GetInstanceObject(id);
	if (!pObject)
	{
		DevMsg("Update for NEW object\n");
	}
	else
	{
		if (bIsLocalUserUpdate && bIsFreshObject)
		{
			//C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
			//pHudBrowserInstance->AddHudLoadingMessage()
			//pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Syncing Objects", "syncobjects", "", VarArgs("%u", m_pImportInfo->count), VarArgs("%u", m_uLastProcessedModelIndex + 1), "processNextModelCallback");
			DevMsg("Update for existing starter object\n");
		}
		else
			DevMsg("Update for existing object\n");
	}
}

KeyValues* C_MetaverseManager::DetectRequiredWorkshopForMapFile(std::string mapFile)
{
	// BEST CAST: this map already has an entry in the library
	// TODO: work

	// else, WORST CASE: figure out where the map is from based on its file location
	char* fullPath = new char[AA_MAX_STRING];
	PathTypeQuery_t pathTypeQuery;
	//std::string fullPathBuf;

	std::string mapBuf = mapFile;
	
	size_t found = mapBuf.find_last_of("/\\");
	if (found != std::string::npos)
		mapBuf = mapBuf.substr(found + 1);

	mapBuf = std::string("maps/") + mapBuf;

	g_pFullFileSystem->RelativePathToFullPath(mapBuf.c_str(), "GAME", fullPath, AA_MAX_STRING, FILTER_NONE, &pathTypeQuery);
	mapBuf = fullPath;

	// mapBuf is now a full file path to the BSP
	std::string workshopId;
	std::string mountId;
	std::string mountTitle;
	bool bIsWorkshop = false;
	bool bIsLegacyWorkshop = false;
	bool bIsLegacyImported = false;
	bool bIsLegacyImportedGen1Workshop = false;	// legacy imported GEN 1 workshop maps get treated like regular legacy imported non-workshop maps

	std::string baseDir = engine->GetGameDirectory();
	std::string importedLegacyDir = g_pAnarchyManager->GetLegacyFolder();
	std::string workshopDir = g_pAnarchyManager->GetWorkshopFolder();

	// Source gives the map path in all-lowercase (WHY GABE N?!) so the paths we test against also need to be lowercase
	std::transform(baseDir.begin(), baseDir.end(), baseDir.begin(), ::tolower);
	std::transform(importedLegacyDir.begin(), importedLegacyDir.end(), importedLegacyDir.begin(), ::tolower);
	std::transform(workshopDir.begin(), workshopDir.end(), workshopDir.begin(), ::tolower);

	if (importedLegacyDir != "" && importedLegacyDir.find_last_of("\\") == importedLegacyDir.length() - 1)
	{
		// check for the very old GEN1 workshop maps being imported from a legacy folder.  importing these are not supported (but subscribing to them is)
		std::string legacyWorkshopGen1MapsDir = importedLegacyDir + "workshop\\workshopmaps\\maps\\";

		if (mapBuf.find(legacyWorkshopGen1MapsDir) == 0)
			bIsLegacyImportedGen1Workshop = true;
	}
	
	if (!bIsLegacyImportedGen1Workshop)
	{
		//DevMsg("Map: %s vs %s\n", mapBuf.c_str(), workshopDir.c_str());
		// check for content from the workshop
		if (mapBuf.find(workshopDir) == 0)
		{
			bIsWorkshop = true;
			// extract the workshop ID
			workshopId = mapBuf.substr(workshopDir.length());
			workshopId = workshopId.substr(0, workshopId.find_last_of("\\"));
			workshopId = workshopId.substr(0, workshopId.find_last_of("\\"));

			// TODO: determine if this is a legacy workshop map (somehow)
			//bIsLegacyWorkshop = ?;
		}
	}

	C_Mount* pMount = g_pAnarchyManager->GetMountManager()->FindOwningMount(mapBuf);
	if (pMount)
	{
		//	DevMsg("Yar.\n");
		mountId = pMount->GetId();
		mountTitle = pMount->GetTitle();
	}

	// populate mapInfo with stuff
	KeyValues* mapInfo = new KeyValues("mapInfo");
	mapInfo->SetString("fullfile", mapBuf.c_str());
	mapInfo->SetString("workshopIds", workshopId.c_str());
	mapInfo->SetString("mountIds", mountId.c_str());
	mapInfo->SetString("mountTitle", mountTitle.c_str());
	mapInfo->SetBool("bIsWorkshop", bIsWorkshop);
	mapInfo->SetBool("bIsLegacyImportedGen1Workshop", bIsLegacyImportedGen1Workshop);
	return mapInfo;
}

KeyValues* C_MetaverseManager::DetectRequiredWorkshopForModelFile(std::string modelFile)
{
	// BEST CAST: this model already has an entry in the library
	// TODO: work

	// else, WORST CASE: figure out where the map is from based on its file location
	char* fullPath = new char[AA_MAX_STRING];
	PathTypeQuery_t pathTypeQuery;
	//std::string fullPathBuf;

	std::string modelBuf = modelFile;

	/*
	size_t found = modelBuf.find_last_of("/\\");
	if (found != std::string::npos)
		modelBuf = modelBuf.substr(found + 1);

	modelBuf = std::string("maps/") + modelBuf;
	*/
	//DevMsg("Here model is: %s\n", modelBuf.c_str());
	g_pFullFileSystem->RelativePathToFullPath(modelBuf.c_str(), "GAME", fullPath, AA_MAX_STRING, FILTER_NONE, &pathTypeQuery);
	modelBuf = fullPath;
	//DevMsg("And now it is: %s\n", modelBuf.c_str());

	// modelBuf is now a full file path to the BSP
	std::string workshopId;
	std::string mountId;
	std::string mountTitle;
	bool bIsWorkshop = false;
	bool bIsLegacyImported = false;
	//bool bIsLegacyWorkshop = false;

	std::string baseDir = engine->GetGameDirectory();
	std::string importedLegacyDir = g_pAnarchyManager->GetLegacyFolder();
	std::string workshopDir = g_pAnarchyManager->GetWorkshopFolder();

	// Source gives the model path in all-lowercase (WHY GABE N?!) so the paths we test against also need to be lowercase
	std::transform(baseDir.begin(), baseDir.end(), baseDir.begin(), ::tolower);
	std::transform(importedLegacyDir.begin(), importedLegacyDir.end(), importedLegacyDir.begin(), ::tolower);
	std::transform(workshopDir.begin(), workshopDir.end(), workshopDir.begin(), ::tolower);

	if (modelBuf.find(importedLegacyDir) == 0)
		bIsLegacyImported = true;

	// check for content from the workshop
	if (modelBuf.find(workshopDir) == 0)
	{
		bIsWorkshop = true;

		// extract the workshop ID
		workshopId = modelBuf.substr(workshopDir.length());
		workshopId = workshopId.substr(0, workshopId.find("\\"));

		// TODO: determine if this is a legacy workshop model (somehow)
		//bIsLegacyWorkshop = ?;
	}

	// determine mount ID

	C_Mount* pMount = g_pAnarchyManager->GetMountManager()->FindOwningMount(modelBuf);
	if (pMount)
	{
	//	DevMsg("Yar.\n");
		mountId = pMount->GetId();
		mountTitle = pMount->GetTitle();
	}

	// populate mapInfo with stuff
	KeyValues* modelInfo = new KeyValues("modelInfo");
	modelInfo->SetString("fullfile", modelBuf.c_str());
	modelInfo->SetString("workshopIds", workshopId.c_str());
	modelInfo->SetString("mountIds", mountId.c_str());
	modelInfo->SetString("mountTitle", mountTitle.c_str());	// FIXME: This is singluar while the other ones are plural.  Hmmmmm.
	modelInfo->SetBool("bIsWorkshop", bIsWorkshop);
	modelInfo->SetBool("bIsLegacyImported", bIsLegacyImported);
	return modelInfo;
}

void C_MetaverseManager::FlagDynamicModels()
{
	// technically shouldn't be needed, but it does allow addon cabinet models to be added to the list just by living in the right folder, and thats cool.
	std::string buf;
	KeyValues* active;
	std::map<std::string, KeyValues*>::iterator it = m_models.begin();
	while (it != m_models.end())
	{
		active = this->GetActiveKeyValues(it->second);
		//active = it->second->FindKey("current");
		//if (!active)
		//	active = it->second->FindKey("local", true);

		if (active->GetInt("dynamic") != 1)
		{
			buf = active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));
			if ((buf.find("models\\cabinets\\") == 0 || buf.find("models/cabinets/") == 0 || buf.find("models\\banners\\") == 0 || buf.find("models/banners/") == 0 || buf.find("models\\frames\\") == 0 || buf.find("models/frames/") == 0 || buf.find("models\\icons\\") == 0 || buf.find("models/icons/") == 0) && (buf.find("room_divider.mdl") == std::string::npos && buf.find("newton_toy.mdl") == std::string::npos))
			{
				active->SetInt("dynamic", 1);
				DevMsg("Flagged %s as dynamic model\n", buf.c_str());
			}
		}
		/*
		else
		{
			buf = active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));
			if ()// || (buf.find("models\\cabinets\\") != 0 && buf.find("models/cabinets/") != 0 && buf.find("models\\banners\\") != 0 && buf.find("models/banners/") != 0 && buf.find("models\\frames\\") != 0 && buf.find("models/frames/") != 0 && buf.find("models\\icons\\") != 0 && buf.find("models/icons/") != 0))
			{
				active->SetInt("dynamic", 0);
				DevMsg("Un-flagged %s as dynamic model\n", buf.c_str());
			}
		}
		*/

		it++;
	}
}

KeyValues* C_MetaverseManager::DetectFirstMap(bool& bAlreadyExists)
{
	//if (m_previousDetectLocalMapFilePath != "")	// FIXME: need a way to detect if there is already a DetectFirst/Next query active.
		//this->DetectLocalMapClose();

	instance_t* pInstance;
	const char *pFilename = g_pFullFileSystem->FindFirstEx("maps\\*.bsp", "GAME", &m_previousDetectLocalMapFileHandle);
	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(m_previousDetectLocalMapFileHandle))
		{
			pFilename = g_pFullFileSystem->FindNext(m_previousDetectLocalMapFileHandle);
			continue;
		}

		std::string foundName = pFilename;
		//foundName = VarArgs("maps\\%s", pFilename);

		// MAKE THE FILE PATH NICE
		char path_buffer[AA_MAX_STRING];
		Q_strcpy(path_buffer, foundName.c_str());
		V_FixSlashes(path_buffer);
		foundName = path_buffer;
		// FINISHED MAKING THE FILE PATH NICE

		KeyValues* active;
		std::map<std::string, KeyValues*>::iterator it = m_maps.begin();
		while (it != m_maps.end())
		{
			active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(it->second);

			if (!Q_strcmp(active->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file"), foundName.c_str()))
			{
				//g_pFullFileSystem->FindClose(m_previousDetectLocalMapFileHandle);
				bAlreadyExists = true;
				return it->second;
			}

			it++;
		}

		// if we haven't found a map for this yet, let's make one.
		KeyValues* map = new KeyValues("map");
		map->SetInt("generation", 3);

		// add standard info
		//std::string id = g_pAnarchyManager->GenerateUniqueId();

		std::string goodTitle = pFilename;
		size_t found = goodTitle.find(".");
		if (found != std::string::npos)
			goodTitle = goodTitle.substr(0, found);

		std::string id = g_pAnarchyManager->GenerateLegacyHash(goodTitle.c_str());

		map->SetString("local/info/id", id.c_str());
		map->SetInt("local/info/created", 0);
		map->SetString("local/info/owner", "local");
		map->SetInt("local/info/removed", 0);
		map->SetString("local/info/remover", "");
		map->SetString("local/info/alias", "");

		std::string mapName = foundName.substr(0, foundName.length() - 4);
		map->SetString("local/title", mapName.c_str());
		map->SetString("local/keywords", "");
		map->SetString("local/platforms/-KJvcne3IKMZQTaG7lPo/file", foundName.c_str());

		KeyValues* stuffKV = g_pAnarchyManager->GetMetaverseManager()->DetectRequiredWorkshopForMapFile(foundName.c_str());
		if (stuffKV)
		{
			map->SetString("local/platforms/-KJvcne3IKMZQTaG7lPo/workshopIds", stuffKV->GetString("workshopIds"));
			map->SetString("local/platforms/-KJvcne3IKMZQTaG7lPo/mountIds", stuffKV->GetString("mountIds"));
			stuffKV->deleteThis();
		}

		m_maps[id.c_str()] = map;
		bAlreadyExists = false;

		// update any legacy instances that were detected that use this map
		std::string legacyMapName = foundName.substr(0, foundName.length() - 4);
		DevMsg("LEGACY MAP NAME: %s\n", legacyMapName.c_str());
		g_pAnarchyManager->GetInstanceManager()->LegacyMapIdFix(legacyMapName, id);
//		if (pInstance)
	//		pInstance->mapId = id;

		return map;
	}

	g_pFullFileSystem->FindClose(m_previousDetectLocalMapFileHandle);
	return null;
}

KeyValues* C_MetaverseManager::DetectNextMap(bool& bAlreadyExists)
{
	instance_t* pInstance;
	const char *pFilename = g_pFullFileSystem->FindNext(m_previousDetectLocalMapFileHandle);
	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(m_previousDetectLocalMapFileHandle))
		{
			pFilename = g_pFullFileSystem->FindNext(m_previousDetectLocalMapFileHandle);
			continue;
		}

		std::string foundName = pFilename;
		//foundName = VarArgs("maps\\%s", pFilename);

		// MAKE THE FILE PATH NICE
		char path_buffer[AA_MAX_STRING];
		Q_strcpy(path_buffer, foundName.c_str());
		V_FixSlashes(path_buffer);
		foundName = path_buffer;
		// FINISHED MAKING THE FILE PATH NICE

		KeyValues* active;
		std::map<std::string, KeyValues*>::iterator it = m_maps.begin();
		while (it != m_maps.end())
		{
			active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(it->second);

			if (!Q_strcmp(active->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file"), foundName.c_str()))
			{
				//g_pFullFileSystem->FindClose(m_previousDetectLocalMapFileHandle);
				bAlreadyExists = true;
				return it->second;
			}

			it++;
		}

		// if we haven't found a map for this yet, let's make one.
		KeyValues* map = new KeyValues("map");
		map->SetInt("generation", 3);

		// add standard info
		//std::string id = g_pAnarchyManager->GenerateUniqueId();

		std::string goodTitle = pFilename;
		size_t found = goodTitle.find(".");
		if (found != std::string::npos)
			goodTitle = goodTitle.substr(0, found);

		//DevMsg("Generating hash based on: %s\n", goodTitle.c_str());
		std::string id = g_pAnarchyManager->GenerateLegacyHash(goodTitle.c_str());

		map->SetString("local/info/id", id.c_str());
		map->SetInt("local/info/created", 0);
		map->SetString("local/info/owner", "local");
		map->SetInt("local/info/removed", 0);
		map->SetString("local/info/remover", "");
		map->SetString("local/info/alias", "");

		std::string mapName = foundName.substr(0, foundName.length() - 4);
		map->SetString("local/title", mapName.c_str());
		map->SetString("local/keywords", "");
		map->SetString("local/platforms/-KJvcne3IKMZQTaG7lPo/file", foundName.c_str());

		KeyValues* stuffKV = g_pAnarchyManager->GetMetaverseManager()->DetectRequiredWorkshopForMapFile(foundName.c_str());
		if (stuffKV)
		{
			map->SetString("local/platforms/-KJvcne3IKMZQTaG7lPo/workshopIds", stuffKV->GetString("workshopIds"));
			map->SetString("local/platforms/-KJvcne3IKMZQTaG7lPo/mountIds", stuffKV->GetString("mountIds"));
			stuffKV->deleteThis();
		}

		m_maps[id.c_str()] = map;
		bAlreadyExists = false;

		// update any legacy instances that were detected that use this map
		std::string legacyMapName = foundName.substr(0, foundName.length() - 4);
		g_pAnarchyManager->GetInstanceManager()->LegacyMapIdFix(legacyMapName, id);
//		pInstance = g_pAnarchyManager->GetInstanceManager()->FindInstance(foundName.substr(0, foundName.length() - 4));
	//	if (pInstance)
		//	pInstance->mapId = id;

		return map;
	}

	g_pFullFileSystem->FindClose(m_previousDetectLocalMapFileHandle);
	return null;
}

// Legacy, because it has a file path.
KeyValues* C_MetaverseManager::LoadLocalItem(std::string file, std::string filePath)
{

	//DevMsg("Attemping to load item %s from %s\n", file.c_str(), filePath.c_str());
	KeyValues* pItem = new KeyValues("item");
	bool bLoaded;

	if (filePath != "")
	{
		std::string fullFile = filePath + file;
		bLoaded = pItem->LoadFromFile(g_pFullFileSystem, fullFile.c_str(), "");
	}
	else
		bLoaded = pItem->LoadFromFile(g_pFullFileSystem, file.c_str(), "MOD");

	if (!bLoaded)
	{
		pItem->deleteThis();
		pItem = null;
	}
	else
	{
		// TODO: Look up any alias here first!!
		KeyValues* pActive = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pItem);

		std::string id = pActive->GetString("info/id");

		std::vector<std::string>* pDefaultFields = g_pAnarchyManager->GetMetaverseManager()->GetDefaultFields();

		unsigned int max = (*pDefaultFields).size();
		for (unsigned int i = 0; i < max; i++)
		{
			if (!pActive->FindKey((*pDefaultFields)[i].c_str()))
				pActive->SetString((*pDefaultFields)[i].c_str(), "");
		}

		if (!pActive->FindKey("type") || !Q_strcmp(pActive->GetString("type"), ""))
			pActive->SetString("type", AA_DEFAULT_TYPEID);
		
		//DevMsg("Finished loading item\n");

		auto it = m_items.find(id);
		if (it != m_items.end())
		{
			// FIXME: Merg with existing item (keeping in mind that non-legacy items overpower legacy items
			if (it->second->GetBool("loadedFromLegacy"))
			{
				this->SmartMergItemKVs(pItem, it->second);
				it->second->deleteThis();
				it->second = pItem;

				// Then remove the loadedFromLegacy tag, if it exits.
				KeyValues* pTargetKey = pItem->FindKey("loadedFromLegacy");
				if (pTargetKey)
					pItem->RemoveSubKey(pTargetKey);
			}
			else
			{
				this->SmartMergItemKVs(it->second, pItem);
				pItem->deleteThis();
				pItem = it->second;
			}
		}
		else
			m_items[id] = pItem;
	}

	return pItem;
}

unsigned int C_MetaverseManager::LoadAllLocalItems(sqlite3* pDb, std::map<std::string, KeyValues*>* pResponseMap)
{
	if (!pDb)
		pDb = m_db;

	// make it use the new shinnit
	unsigned int count = 0;
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(pDb, "SELECT * from items", -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed: %s\n", sqlite3_errmsg(pDb));

	int length;
	while (sqlite3_step(stmt) == SQLITE_ROW)	// THIS IS WHERE THE LOOP CAN BE BROKEN UP AT!!
	{
		length = sqlite3_column_bytes(stmt, 1);

		if (length == 0)
		{
			DevMsg("WARNING: Zero-byte KeyValues skipped.\n");
			continue;
		}

		KeyValues* pItem = new KeyValues("item");

		CUtlBuffer buf(0, length, 0);
		buf.CopyBuffer(sqlite3_column_blob(stmt, 1), length);
		pItem->ReadAsBinary(buf);
		buf.Purge();

		// TODO: Look up any alias here first!!
		KeyValues* pActive = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pItem);

		count++;

		std::string id = pActive->GetString("info/id");

		std::vector<std::string>* pDefaultFields = g_pAnarchyManager->GetMetaverseManager()->GetDefaultFields();

		unsigned int max = (*pDefaultFields).size();
		for (unsigned int i = 0; i < max; i++)
		{
			if (!pActive->FindKey((*pDefaultFields)[i].c_str()))
				pActive->SetString((*pDefaultFields)[i].c_str(), "");
		}

		if (!pActive->FindKey("type") || !Q_strcmp(pActive->GetString("type"), ""))
			pActive->SetString("type", AA_DEFAULT_TYPEID);

		if (pResponseMap)
			(*pResponseMap)[id] = pItem;
		else
			m_items[id] = pItem;
	}
	sqlite3_finalize(stmt);	// TODO: error checking?  Maybe not needed, if this is like a close() operation.
	return count;
}

KeyValues* C_MetaverseManager::LoadLocalModel(std::string file, std::string filePath)
{
	// make sure sound doesn't stutter
	engine->Sound_ExtraUpdate();

	KeyValues* pModel = new KeyValues("model");
	bool bLoaded;

	if (filePath != "")
	{
		std::string fullFile = filePath + file;
		bLoaded = pModel->LoadFromFile(g_pFullFileSystem, fullFile.c_str(), "");
	}
	else
		bLoaded = pModel->LoadFromFile(g_pFullFileSystem, file.c_str(), "MOD");

	if (!bLoaded)
	{
		pModel->deleteThis();
		pModel = null;
	}
	else
	{
		// TODO: Look up any alias here first!!
		KeyValues* pActive = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pModel);

		std::string id = pActive->GetString("info/id");

		std::vector<std::string> defaultFields;	// Should platform specific fields be expected as well?  Jump off that bridge when you cross it.
		defaultFields.push_back("title");
		defaultFields.push_back("keywords");
		defaultFields.push_back("dynamic");

		unsigned int max = defaultFields.size();
		for (unsigned int i = 0; i < max; i++)
		{
			if (!pActive->FindKey(defaultFields[i].c_str()))
				pActive->SetString(defaultFields[i].c_str(), "");
		}

		m_models[id] = pModel;
	}

	return pModel;
}

unsigned int C_MetaverseManager::LoadAllLocalModels(unsigned int& numDynamic, sqlite3* pDb, std::map<std::string, KeyValues*>* pResponseMap)
{
	if (!pDb)
		pDb = m_db;

	// make it use the new shinnit
	unsigned int count = 0;
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(pDb, "SELECT * from models", -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed: %s\n", sqlite3_errmsg(pDb));

	int length;
	while (sqlite3_step(stmt) == SQLITE_ROW)	// THIS IS WHERE THE LOOP CAN BE BROKEN UP AT!!
	{
		length = sqlite3_column_bytes(stmt, 1);

		if (length == 0)
		{
			DevMsg("WARNING: Zero-byte KeyValues skipped.\n");
			continue;
		}

		KeyValues* pModel = new KeyValues("model");

		CUtlBuffer buf(0, length, 0);
		buf.CopyBuffer(sqlite3_column_blob(stmt, 1), length);
		pModel->ReadAsBinary(buf);
		buf.Purge();

		// TODO: Look up any alias here first!!
		KeyValues* pActive = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pModel);

		count++;

		std::string id = pActive->GetString("info/id");

		std::vector<std::string> defaultFields;	// Should platform specific fields be expected as well?  Jump off that bridge when you cross it.
		defaultFields.push_back("title");
		defaultFields.push_back("keywords");
		defaultFields.push_back("dynamic");

		unsigned int max = defaultFields.size();
		for (unsigned int i = 0; i < max; i++)
		{
			if (!pActive->FindKey(defaultFields[i].c_str()))
				pActive->SetString(defaultFields[i].c_str(), "");
		}


		if (pResponseMap)
			(*pResponseMap)[id] = pModel;
		else
			m_models[id] = pModel;

		if (pActive->GetBool("dynamic"))
			numDynamic++;
	}
	sqlite3_finalize(stmt);	// TODO: error checking?  Maybe not needed, if this is like a close() operation.
	return count;
}

std::string C_MetaverseManager::ResolveLegacyModel(std::string legacyModel)
{
	// iterate through the models
	KeyValues* active;
	for (std::map<std::string, KeyValues*>::iterator it = m_models.begin(); it != m_models.end(); ++it)
	{
		active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(it->second);

		// MAKE THE FILE PATH NICE
		char niceModel[AA_MAX_STRING];
		Q_strcpy(niceModel, legacyModel.c_str());
		V_FixSlashes(niceModel);

		//for (int i = 0; niceModel[i] != '\0'; i++)
		//	niceModel[i] = tolower(niceModel[i]);
		// FINISHED MAKING THE FILE PATH NICE

		if (!Q_stricmp(active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID)), niceModel))
		{
			return it->first;
		}
	}

	return "";
}