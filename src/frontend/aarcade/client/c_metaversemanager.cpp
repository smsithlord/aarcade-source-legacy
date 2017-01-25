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
	m_pSpawningObject = null;
	m_pSpawningObjectEntity = null;
	m_iSpawningRotationAxis = 1;
	m_spawningAngles.x = 0;
	m_spawningAngles.y = 0;
	m_spawningAngles.z = 0;

	// create or open the library.db
	int rc;
	char *error;

	// create the library database
	DevMsg("Create library database...\n");

	rc = sqlite3_open("aarcade_user/library.db", &m_db);
	if (rc)
	{
		DevMsg("Error opening SQLite3 database: %s\n", sqlite3_errmsg(m_db));
		sqlite3_close(m_db);
	//	return;
	}

	// check if the tables exist already
	// (maybe we don't need to if there is proper error handling when WRITING to the tables)
	/*
	// create the tables
	const char *sqlCreateAppsTable = "CREATE TABLE apps (id INTEGER PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(db, sqlCreateAppsTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}

	const char *sqlCreateItemsTable = "CREATE TABLE items (id INTEGER PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(db, sqlCreateItemsTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}

	const char *sqlCreateMapsTable = "CREATE TABLE maps (id INTEGER PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(db, sqlCreateMapsTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}

	const char *sqlCreateModelsTable = "CREATE TABLE models (id INTEGER PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(db, sqlCreateModelsTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}

	const char *sqlCreatePlatformsTable = "CREATE TABLE platforms (id INTEGER PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(db, sqlCreatePlatformsTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}

	const char *sqlCreateTypesTable = "CREATE TABLE types (id INTEGER PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(db, sqlCreateTypesTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}

	const char *sqlCreateInstancesTable = "CREATE TABLE instances (id INTEGER PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(db, sqlCreateInstancesTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}
	*/
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

	sqlite3_close(m_db);
	// TODO: error objects for sqlite must also be cleaned up
}
/*
void C_MetaverseManager::OnWebTabCreated(C_WebTab* pWebTab)
{
	m_pWebTab = pWebTab;
}
*/

void C_MetaverseManager::Update()
{
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

		// FIXME: that stuff should be done server-side so collision boxes are updated properly...
		engine->ServerCmd(VarArgs("setcabpos %i %f %f %f %f %f %f \"%s\";\n", m_pSpawningObjectEntity->entindex(), tr.endpos.x, tr.endpos.y, tr.endpos.z, angles.x, angles.y, angles.z, ""), false);
	}
}

void C_MetaverseManager::OnMountAllWorkshopsCompleted()
{
	// FIXME this junction should take place in the anarchy manager!!

//	/*
	


//	*/

	this->DetectAllLegacyCabinets();

	g_pAnarchyManager->GetWorkshopManager()->OnMountWorkshopSucceed();
//	g_pAnarchyManager->OnMountAllWorkshopsComplete();
}

//void C_MetaverseManager::ImportSteamGame(std::string name, std::string appid)
void C_MetaverseManager::ImportSteamGames(KeyValues* kv)
{
	// 1st save this out
	kv->SaveToFile(g_pFullFileSystem, "steamGames.key", "DEFAULT_WRITE_PATH");

	// now loop through it and add any missing games to the user library (but don't save them until the user makes a change to them.)
	std::string appid;
	std::string name;
	std::string detailsurl;
	std::string screenurl;
	std::string marqueeurl;

	for (KeyValues *sub = kv->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	{
		appid = sub->GetName();
		appid = appid.substr(2);
		name = sub->GetString();
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
		}
		//else
		//	DevMsg("Already have a Steam game w/ id %s\n", appid.c_str());
	}

	kv->deleteThis();
}

void C_MetaverseManager::AddItem(KeyValues* pItem)
{
	KeyValues* active = pItem->FindKey("current");
	if (!active)
		active = pItem->FindKey("local");

	if (active)
	{
		std::string id = VarArgs("%s", active->GetString("info/id"));
		m_items[id] = pItem;
	}
}

void C_MetaverseManager::SaveSQL(const char* tableName, const char* id, KeyValues* kv)
{
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(m_db, VarArgs("REPLACE INTO %s VALUES(\"%s\", ?)", tableName, id), -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: prepare failed: %s\n", sqlite3_errmsg(m_db));

	// SQLITE_STATIC because the statement is finalized before the buffer is freed:
	CUtlBuffer buf;
	kv->WriteAsBinary(buf);

	int size = buf.Size();
	rc = sqlite3_bind_blob(stmt, 1, buf.Base(), size, SQLITE_STATIC);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: bind failed: %s\n", sqlite3_errmsg(m_db));
	else
	{
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
			DevMsg("FATAL ERROR: execution failed: %s\n", sqlite3_errmsg(m_db));
	}
	sqlite3_finalize(stmt);

	DevMsg("Saved instance.\n");
}

void C_MetaverseManager::SaveItem(KeyValues* pItem)
{
	KeyValues* pTargetKey = pItem->FindKey("loadedFromLegacy");
	if (pTargetKey)
		pItem->RemoveSubKey(pTargetKey);

	KeyValues* active = pItem->FindKey("current");
	if (!active)
		active = pItem->FindKey("local");

	// FIXME: SAVING TO .KEY FILES DISABLED FOR MYSQL MIGRATION!!
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(m_db, VarArgs("REPLACE INTO items VALUES(\"%s\", ?)", active->GetString("info/id")), -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: prepare failed: %s\n", sqlite3_errmsg(m_db));

	// SQLITE_STATIC because the statement is finalized before the buffer is freed:
	CUtlBuffer buf;
	pItem->WriteAsBinary(buf);

	int size = buf.Size();
	rc = sqlite3_bind_blob(stmt, 1, buf.Base(), size, SQLITE_STATIC);
	if (rc != SQLITE_OK)
		DevMsg("FATAL ERROR: bind failed: %s\n", sqlite3_errmsg(m_db));
	else
	{
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
			DevMsg("FATAL ERROR: execution failed: %s\n", sqlite3_errmsg(m_db));
	}
	sqlite3_finalize(stmt);
	//return rc;
	//pItem->SaveToFile(g_pFullFileSystem, VarArgs("library/items/%s.key", active->GetString("info/id")), "DEFAULT_WRITE_PATH");
	//DevMsg("Saved item %s to library/items/%s.key\n", active->GetString("title"), active->GetString("info/id"));
}

KeyValues* C_MetaverseManager::CreateItem(KeyValues* pInfo)
{
	//KeyValues* pItem = new KeyValues("item");

	KeyValues* pItem = new KeyValues("item");
	pItem->SetInt("generation", 3);

	// add standard info
	pItem->SetString("local/info/id", g_pAnarchyManager->GenerateUniqueId());
	pItem->SetInt("local/info/created", 0);
	pItem->SetString("local/info/owner", "local");
	pItem->SetInt("local/info/removed", 0);
	pItem->SetString("local/info/remover", "");
	pItem->SetString("local/info/alias", "");

	pItem->SetString("local/type", "-KKa1MHJTls2KqNphWFM");	// FIXME: default type ID?  should be a define!!

	KeyValues* active = pItem->FindKey("local");
	for (KeyValues *sub = pInfo->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		active->SetString(sub->GetName(), sub->GetString());

	return pItem;
}

void C_MetaverseManager::SmartMergItemKVs(KeyValues* pItemA, KeyValues* pItemB, bool bPreferFirst)
{
	KeyValues* pKeeperItem = (bPreferFirst) ? pItemA : pItemB;
	KeyValues* pMergerItem = (bPreferFirst) ? pItemB : pItemA;

	KeyValues* pActiveKeeper = pKeeperItem->FindKey("current");
	if (!pActiveKeeper)
		pActiveKeeper = pKeeperItem->FindKey("local", true);

	KeyValues* pActiveMerger = pMergerItem->FindKey("current");
	if (!pActiveMerger)
		pActiveMerger = pMergerItem->FindKey("local", true);

	if (Q_strcmp(pActiveKeeper->GetString("info/id"), pActiveMerger->GetString("info/id")))
		DevMsg("WARNING: SmargMerg-ing 2 items with different IDs: %s %s\n", pActiveKeeper->GetString("info/id"), pActiveMerger->GetString("info/id"));

	/*
	KeyValues* pItem = new KeyValues("item");
	if (pKeeperItem->GetBool("loadedFromLegacy") && pMergerItem->GetBool("loadedFromLegacy"))
		pItem->SetBool("loadedFromLegacy", true);

	KeyValues* pActive = pItem->FindKey("local", true);
	*/

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

bool C_MetaverseManager::LoadSQLKevValues(const char* tableName, const char* id, KeyValues* kv)
{
	sqlite3_stmt *stmt = NULL;
	DevMsg("loading from table name: %s _id %s\n", tableName, id);
	int rc = sqlite3_prepare(m_db, VarArgs("SELECT * from %s WHERE id = \"%s\"", tableName, id), -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed: %s\n", sqlite3_errmsg(m_db));

	bool bSuccess = false;
	int length;
	if (sqlite3_step(stmt) == SQLITE_ROW)	// THIS IS WHERE THE LOOP CAN BE BROKEN UP AT!!
	{
		length = sqlite3_column_bytes(stmt, 1);

		CUtlBuffer buf(0, length, 0);
		buf.CopyBuffer(sqlite3_column_blob(stmt, 1), length);
		if( kv->ReadAsBinary(buf) )
			bSuccess = true;
	}
	sqlite3_finalize(stmt);	// TODO: error checking?  Maybe not needed, if this is like a close() operation.

	return bSuccess;
}

KeyValues* C_MetaverseManager::LoadLocalItemLegacy(bool& bIsModel, std::string file, std::string filePath, std::string workshopIds, std::string mountIds)
{
	//KeyValues* pItem2 = new KeyValues("item");
	//bIsModel = false;
//	return pItem2;

	KeyValues* pItem = new KeyValues("item");
	bool bLoaded;
	
	std::string fullFile = filePath + file;
	//DevMsg("Try: %s\n", fullFile);
	if (filePath != "")
	{
		//std::string fullFile = filePath + file;
		bLoaded = pItem->LoadFromFile(g_pFullFileSystem, fullFile.c_str(), "");
	}
	else
		bLoaded = pItem->LoadFromFile(g_pFullFileSystem, file.c_str(), "MOD");

//	DevMsg("Here: %s\n", fullFile.c_str());

	bIsModel = false;
	if ( !bLoaded )
	{
		//DevMsg("Failed to load: %s\n", file.c_str());
		pItem->deleteThis();
		pItem = null;
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
			std::string modelFile = pItem->GetString("filelocation");
			size_t foundExt = modelFile.find(".mdl");
			if (foundExt == modelFile.length() - 4)
			{
				bIsModel = true;
				//std::string itemId = g_pAnarchyManager->ExtractLegacyId(file, pItem).c_str();
				std::string itemId = g_pAnarchyManager->GenerateLegacyHash(pItem->GetString("filelocation"));
		//		std::string itemId = g_pAnarchyManager->ExtractLegacyId(std::string(pItem->GetString("filelocation")));
	//			if (itemId == "")
//					itemId = g_pAnarchyManager->ExtractLegacyId(std::string(pItem->GetString("lastmodel")));
				
				pItem->SetString("local/info/id", itemId.c_str());
				std::string goodTitle = pItem->GetString("title");
				if (Q_strcmp(pItem->GetString("group"), ""))
					goodTitle = VarArgs("%s - %s", pItem->GetString("group"), goodTitle.c_str());

				pItem->SetString("local/title", goodTitle.c_str());
				//pItem->SetString("local/keywords", "");
				//pItem->SetInt("local/dynamic", 0);
				pItem->SetString(VarArgs("local/platforms/%s/id", AA_PLATFORM_ID), AA_PLATFORM_ID);
				pItem->SetString(VarArgs("local/platforms/%s/file", AA_PLATFORM_ID), modelFile.c_str());
				pItem->SetString(VarArgs("local/platforms/%s/download", AA_PLATFORM_ID), "");

				pItem->SetString(VarArgs("local/platforms/%s/workshopId", AA_PLATFORM_ID), workshopIds.c_str());
				pItem->SetString(VarArgs("local/platforms/%s/mountId", AA_PLATFORM_ID), mountIds.c_str());

				// models can be loaded right away because they don't depend on anything else, like items do. (items depend on models)
				//DevMsg("Loading model with ID %s and model %s\n", itemId.c_str(), modelFile.c_str());

				auto it = m_models.find(itemId);
				if (it != m_models.end())
				{
					pItem->deleteThis();
					pItem = it->second;
				}
				else
					m_models[itemId] = pItem;
			}
			else
			{
				pItem->SetString("local/info/id", g_pAnarchyManager->ExtractLegacyId(file, pItem).c_str());

				//pItem->SetString("local/title", pItem->GetString("title"));

				std::string goodTitle = pItem->GetString("title");
				if (Q_strcmp(pItem->GetString("group"), ""))
					goodTitle = VarArgs("%s - %s", pItem->GetString("group"), goodTitle.c_str());

				pItem->SetString("local/title", goodTitle.c_str());
				pItem->SetString("local/description", pItem->GetString("description"));
				pItem->SetString("local/file", pItem->GetString("filelocation"));

				// NEEDS RESOLVING!!
				std::string legacyType = pItem->GetString("type");
				std::string resolvedType = this->ResolveLegacyType(legacyType);
				pItem->SetString("local/type", resolvedType.c_str());

				// NEEDS RESOLVING!!
				std::string legacyApp = pItem->GetString("app");
				std::string resolvedApp = this->ResolveLegacyApp(legacyApp);
				pItem->SetString("local/app", resolvedApp.c_str());
				pItem->SetString("local/reference", pItem->GetString("detailsurl"));
				pItem->SetString("local/preview", pItem->GetString("trailerurl"));
				pItem->SetString("local/download", pItem->GetString("downloadurl"));
				//pItem->SetString("local/stream", "");

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


				m_previousLoadLocalItemsLegacyBuffer.push_back(pItem);
				//DevMsg("WIN!\n");
				// TODO: Generate an ID and add this to the library!!
			}
		}
	}

	return pItem;
}

unsigned int C_MetaverseManager::LoadAllLocalItemsLegacy(unsigned int& uNumModels, std::string filePath, std::string workshopIds, std::string mountIds)
{
	uNumModels = 0;

	FileFindHandle_t testFolderHandle;
	std::string fullPath = filePath + "library\\*";
	//DevMsg("Path here is: %s\n", fullPath);
	const char *pFoldername = g_pFullFileSystem->FindFirst(fullPath.c_str(), &testFolderHandle);

	std::vector<KeyValues*> items;

	KeyValues* responseKv;
	bool bIsModel;
	unsigned int modelCount = 0;
	unsigned int count = 0;
	while (pFoldername)
	{
		if (!Q_strcmp(pFoldername, ".") || !Q_strcmp(pFoldername, ".."))
		{
			pFoldername = g_pFullFileSystem->FindNext(testFolderHandle);
			continue;
		}
		
		std::string FolderPath = VarArgs("library\\%s", pFoldername);
		//DevMsg("Folder name: %s%s\n", filePath.c_str(), FolderPath.c_str());
		if (!g_pFullFileSystem->FindIsDirectory(testFolderHandle))
		{
			DevMsg("All items files must be within a subfolder!\n");
			pFoldername = g_pFullFileSystem->FindNext(testFolderHandle);
			continue;
		}

		FileFindHandle_t testFileHandle;
		const char *pFilename = g_pFullFileSystem->FindFirst(VarArgs("%s%s\\*.itm", filePath.c_str(), FolderPath.c_str()), &testFileHandle);

		while (pFilename != NULL)
		{
			if (g_pFullFileSystem->FindIsDirectory(testFileHandle))
			{
				pFilename = g_pFullFileSystem->FindNext(testFileHandle);
				continue;
			}

			// Check real quick if this item is of the format [itemName].[workshop_id].itm, ie. check if it has 2 dots or only 1.
			// PROBABLY STILL REQUIRED FOR LEGACY SUPPORT
	//		std::string foundName = pFilename;
//			foundName = foundName.substr(foundName.find_first_of(".") + 1);

//			if (foundName.find(".") != std::string::npos)
	//			continue;

			std::string foundName = VarArgs("%s\\%s", FolderPath.c_str(), pFilename);
			//DevMsg("Tester here is: %s%s\n", filePath.c_str(), foundName.c_str());
			pFilename = g_pFullFileSystem->FindNext(testFileHandle);

			// MAKE THE FILE PATH NICE
			char path_buffer[AA_MAX_STRING];
			Q_strcpy(path_buffer, foundName.c_str());
			V_FixSlashes(path_buffer);
			/*
			for (int i = 0; path_buffer[i] != '\0'; i++)
				path_buffer[i] = tolower(path_buffer[i]);
			*/
			// FINISHED MAKING THE FILE PATH NICE

			foundName = path_buffer;

			//this->AddItemFile(foundName.c_str());

			bIsModel = false;
			//DevMsg("Try: %s\\%s\n", filePath.c_str(), foundName.c_str());
			responseKv = this->LoadLocalItemLegacy(bIsModel, foundName, filePath, workshopIds, mountIds);
			if ( responseKv )
			{
				// don't actually add us to the m_items yet!!
				items.push_back(responseKv);
				//count++;
			}

			if (bIsModel)
				modelCount++;
		}

		g_pFullFileSystem->FindClose(testFileHandle);

		pFoldername = g_pFullFileSystem->FindNext(testFolderHandle);
	}

	g_pFullFileSystem->FindClose(testFolderHandle);

	//DevMsg("Recounting:\n");
	unsigned int numResponses = items.size();
	unsigned int i;
	unsigned int numVictims;
	unsigned int j;
	KeyValues* active;
	KeyValues* pItem;
	std::vector<KeyValues*> victims;
	for (i = 0; i < numResponses; i++)
	{
		pItem = items[i];

		// now actually resolve the models and add the items!
		// NEEDS RESOLVING!!
		std::string resolvedModel = this->ResolveLegacyModel(pItem->GetString("lastmodel"));

		active = pItem->FindKey("current");
		if (!active)
			active = pItem->FindKey("local", true);

		active->SetString("model", resolvedModel.c_str());

		// remove everything not in local or current or generation
		for (KeyValues *sub = pItem->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		{
			if (Q_strcmp(sub->GetName(), "local") && Q_strcmp(sub->GetName(), "local") && Q_strcmp(sub->GetName(), "generation"))
				victims.push_back(sub);
		}

		numVictims = victims.size();
		for (j = 0; j < numVictims; j++)
			pItem->RemoveSubKey(victims[j]);

		if ( numVictims > 0 )
			victims.clear();

		std::string id = VarArgs("%s", pItem->GetString("local/info/id"));
		m_items[id] = pItem;
		count++;

//		DevMsg("\tCounting \n", )
	}

	while (!items.empty())
		items.pop_back();

	uNumModels = modelCount;
	return numResponses;// count;
}

void C_MetaverseManager::LoadFirstLocalItemLegacy(bool bFastMode, std::string filePath, std::string workshopIds, std::string mountIds)
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
			pItem = this->LoadLocalItemLegacy(bIsModel, foundName, m_previousLoadLocalItemLegacyFilePath, m_previousLocaLocalItemLegacyWorkshopIds, m_previousLoadLocalItemLegacyMountIds);
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
		pItem = this->LoadLocalItemLegacy(bIsModel, foundName, m_previousLoadLocalItemLegacyFilePath, m_previousLocaLocalItemLegacyWorkshopIds, m_previousLoadLocalItemLegacyMountIds);
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
			pItem = this->LoadLocalItemLegacy(bIsModel, foundName, m_previousLoadLocalItemLegacyFilePath, m_previousLocaLocalItemLegacyWorkshopIds, m_previousLoadLocalItemLegacyMountIds);
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
//		m_previousLoadLocalItemsLegacyBuffer.clear();
	}
}

void C_MetaverseManager::ResolveLoadLocalItemLegacyBuffer()
{
	// FIXME This is a huge bottleneck

	// This usually gets done after all  workshop mounts are done, but since this is an after-the-fact one, gotta do it again manually here
	unsigned int numResponses = m_previousLoadLocalItemsLegacyBuffer.size();
	unsigned int i;
	unsigned int j;
	KeyValues* pItem;
	KeyValues* active;
	unsigned int numVictims;
	std::vector<KeyValues*> victims;
	//KeyValues* model;
	//KeyValues* modelActive;
	for (i = 0; i < numResponses; i++)
	{
		pItem = m_previousLoadLocalItemsLegacyBuffer[i];

		// now actually resolve the models and add the items!
		active = pItem->FindKey("current");
		if (!active)
			active = pItem->FindKey("local", true);

		/*
		// NEEDS RESOLVING!!
		std::string resolvedModel = this->ResolveLegacyModel(pItem->GetString("lastmodel"));
		active->SetString("model", resolvedModel.c_str());
		*/

		std::string resolvedModel = g_pAnarchyManager->GenerateLegacyHash(pItem->GetString("lastmodel"));
		active->SetString("model", resolvedModel.c_str());

		// remove everything not in local or current or generation
		for (KeyValues *sub = pItem->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		{
			if (Q_strcmp(sub->GetName(), "local") && Q_strcmp(sub->GetName(), "local") && Q_strcmp(sub->GetName(), "generation"))
				victims.push_back(sub);
		}

		numVictims = victims.size();
		for (j = 0; j < numVictims; j++)
			pItem->RemoveSubKey(victims[j]);

		if (numVictims > 0)
			victims.clear();

		std::string id = VarArgs("%s", pItem->GetString("local/info/id"));

		auto it = m_items.find(id);
		if (it != m_items.end())
		{
			// this is a legacy item, so give the right-of-way to any generation 3 item that was already found.
			KeyValues* pLoadedFromLegacy = it->second->FindKey("isLoadedFromLegacy");
			if (pLoadedFromLegacy && pLoadedFromLegacy->GetBool())
			{
				// merg this legacy item with the other legacy item
				this->SmartMergItemKVs(it->second, pItem);
				pItem->deleteThis();
				pItem = it->second;

				//// FIXME: For now, just delete the old one and let this one overpower it.
				//it->second->deleteThis();
				//m_items.erase(it);
				//m_items[id] = pItem;
			}
			else
			{
				// let the generation 3 item overpower us
				pItem->deleteThis();
				pItem = it->second;
			}

			//KeyValues* pExistingItem = it->second;
		}
		else
			m_items[id] = pItem;
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
		KeyValues* pActive = pType->FindKey("current");
		if (!pActive)
			pActive = pType->FindKey("local");

		if (pActive)
		{
			std::string id = pActive->GetString("info/id");
			m_types[id] = pType;
			//DevMsg("adding type: %s\n", id.c_str());
		}
		else
		{
			pType->deleteThis();
			pType = null;
		}
	}

	return pType;
}

unsigned int C_MetaverseManager::LoadAllLocalTypes(std::string filePath)
{
	// make it use the new shinnit
	unsigned int count = 0;
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(m_db, "SELECT * from types", -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed: %s\n", sqlite3_errmsg(m_db));

	int length;
	while (sqlite3_step(stmt) == SQLITE_ROW)	// THIS IS WHERE THE LOOP CAN BE BROKEN UP AT!!
	{
		length = sqlite3_column_bytes(stmt, 1);

		KeyValues* pType = new KeyValues("type");

		CUtlBuffer buf(0, length, 0);
		buf.CopyBuffer(sqlite3_column_blob(stmt, 1), length);
		pType->ReadAsBinary(buf);

		// TODO: Look up any alias here first!!
		KeyValues* pActive = pType->FindKey("current");
		if (!pActive)
			pActive = pType->FindKey("local");

		if (pActive)
		{
			std::string id = pActive->GetString("info/id");
			m_types[id] = pType;
			count++;
			//DevMsg("adding type: %s\n", id.c_str());
		}
		else
		{
			pType->deleteThis();
			pType = null;
		}
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
		return "-KKa1MHJTls2KqNphWFM";

	// iterate through the types
	KeyValues* active;
	for (std::map<std::string, KeyValues*>::iterator it = m_types.begin(); it != m_types.end(); ++it)
	{
		active = it->second->FindKey("current");
		if (!active)
			active = it->second->FindKey("local");

		if (!Q_stricmp(active->GetString("title"), legacyType.c_str()))
		{
			return it->first;
		}
	}

	return "";
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
		KeyValues* pActive = pApp->FindKey("current");
		if (!pActive)
			pActive = pApp->FindKey("local");

		std::string id = pActive->GetString("info/id");
		m_apps[id] = pApp;
	}

	return pApp;
}

unsigned int C_MetaverseManager::LoadAllLocalApps(std::string filePath)
{
	// make it use the new shinnit
	unsigned int count = 0;
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(m_db, "SELECT * from apps", -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed: %s\n", sqlite3_errmsg(m_db));

	int length;
	while (sqlite3_step(stmt) == SQLITE_ROW)	// THIS IS WHERE THE LOOP CAN BE BROKEN UP AT!!
	{
		length = sqlite3_column_bytes(stmt, 1);

		KeyValues* pApp = new KeyValues("app");

		CUtlBuffer buf(0, length, 0);
		buf.CopyBuffer(sqlite3_column_blob(stmt, 1), length);
		pApp->ReadAsBinary(buf);

		// TODO: Look up any alias here first!!
		KeyValues* pActive = pApp->FindKey("current");
		if (!pActive)
			pActive = pApp->FindKey("local");

		if (pActive)
		{
			std::string id = pActive->GetString("info/id");
			m_types[id] = pApp;
			count++;
			//DevMsg("adding type: %s\n", id.c_str());
		}
		else
		{
			pApp->deleteThis();
			pApp = null;
		}
	}
	sqlite3_finalize(stmt);	// TODO: error checking?  Maybe not needed, if this is like a close() operation.
	return count;
	/*
	unsigned int count = 0;
	FileFindHandle_t testFileHandle;
	const char *pFilename = g_pFullFileSystem->FindFirst(VarArgs("%slibrary\\apps\\*.key", filePath.c_str()), &testFileHandle);

	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(testFileHandle))
		{
			pFilename = g_pFullFileSystem->FindNext(testFileHandle);
			continue;
		}

		std::string foundName = pFilename;
		foundName = VarArgs("library\\apps\\%s", pFilename);
		pFilename = g_pFullFileSystem->FindNext(testFileHandle);

		// MAKE THE FILE PATH NICE
		char path_buffer[AA_MAX_STRING];
		Q_strcpy(path_buffer, foundName.c_str());
		V_FixSlashes(path_buffer);

		for (int i = 0; path_buffer[i] != '\0'; i++)
			path_buffer[i] = tolower(path_buffer[i]);
		// FINISHED MAKING THE FILE PATH NICE

		foundName = path_buffer;
		if (this->LoadLocalApp(foundName, filePath, "MOD"))
			count++;
	}

	g_pFullFileSystem->FindClose(testFileHandle);
	return count;
	*/
}

std::string C_MetaverseManager::ResolveLegacyApp(std::string legacyApp)
{
	if (legacyApp == "")
		return "";

	// iterate through the types
	KeyValues* active;
	for (std::map<std::string, KeyValues*>::iterator it = m_apps.begin(); it != m_apps.end(); ++it)
	{
		active = it->second->FindKey("current");
		if (!active)
			active = it->second->FindKey("local");

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

const char* C_MetaverseManager::GetFirstLibraryEntry(KeyValues*& response, const char* category)
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
	else // models
	{
		m_previousGetModelIterator = m_models.begin();
		response = (m_previousGetModelIterator != m_models.end()) ? m_previousGetModelIterator->second : null;
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

	return queryId.c_str();
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
	else	// models
	{
		if (m_previousGetModelIterator != m_models.end())
			m_previousGetModelIterator++;

		return (m_previousGetModelIterator != m_models.end()) ? m_previousGetModelIterator->second : null;
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
		active = potential->FindKey("current");
		if (!active)
			active = potential->FindKey("local");
		if (active)
		{
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

KeyValues* C_MetaverseManager::FindNextLibraryModel()
{
	// continue the search
	KeyValues* response = null;
	m_previousFindModelIterator++;
	if (m_previousFindModelIterator != m_models.end())
		response = this->FindLibraryModel(m_pPreviousModelSearchInfo, m_previousFindModelIterator);
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

	std::map<std::string, KeyValues*>* pCategoryEntries = (!Q_strcmp(category, "items")) ? &m_items : &m_models;

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
		active = potential->FindKey("current");
		if (!active)
			active = potential->FindKey("local");
		if (active)
		{
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
		active = potential->FindKey("current");
		if (!active)
			active = potential->FindKey("local");
		if (active)
		{
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
	else// if (categoryBuf == "models")
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
	else
	{
		// continue the search
		m_previousFindModelIterator++;
		if (m_previousFindModelIterator != m_items.end())
			response = this->FindLibraryEntry(category, m_pPreviousModelSearchInfo, m_previousFindModelIterator);
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

KeyValues* C_MetaverseManager::GetLibraryApp(std::string id)
{
	std::map<std::string, KeyValues*>::iterator it = m_apps.find(id);
	if (it != m_apps.end())
		return it->second;
	else
		return null;
}

std::map<std::string, std::string>& C_MetaverseManager::DetectAllMapScreenshots()
{
	size_t found;
	std::string id;
	std::map<std::string, std::string>::iterator it;

	// GRAB A LIST OF THE LATEST SCREENSHOT FOR EACH FILE NAME BEFORE HAND, so screenshots can be bound to maps as they are added.
	FileFindHandle_t pFileFindHandle;
	const char *pScreenshotName = g_pFullFileSystem->FindFirstEx("screenshots\\*.jpg", "GAME", &pFileFindHandle);
	while (pScreenshotName != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(pFileFindHandle))
		{
			pScreenshotName = g_pFullFileSystem->FindNext(pFileFindHandle);
			continue;
		}

		// doesn't matter if it already exists, overwrite it with this one
		id = pScreenshotName;

		found = id.find_last_of(".");
		if (found != std::string::npos)
		{
			id = id.substr(0, found - 4);
			m_mapScreenshots[id] = pScreenshotName;
		}

		pScreenshotName = g_pFullFileSystem->FindNext(pFileFindHandle);
	}
	g_pFullFileSystem->FindClose(pFileFindHandle);

	return m_mapScreenshots;
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
			pModel = new KeyValues("model");

			// update us to 3rd generation
			pModel->SetInt("generation", 3);

			// add standard info (except for id)
			pModel->SetInt("local/info/created", 0);
			pModel->SetString("local/info/owner", "local");
			pModel->SetInt("local/info/removed", 0);
			pModel->SetString("local/info/remover", "");
			pModel->SetString("local/info/alias", "");

			modelId = g_pAnarchyManager->GenerateLegacyHash(cat->GetString("model"));
			pModel->SetString("local/info/id", modelId.c_str());

			pModel->SetString("local/title", pModel->GetString("name"));
			pModel->SetString("local/keywords", "");
			pModel->SetInt("local/dynamic", 0);
			pModel->SetString(VarArgs("local/platforms/%s/id", AA_PLATFORM_ID), AA_PLATFORM_ID);
			pModel->SetString(VarArgs("local/platforms/%s/file", AA_PLATFORM_ID), cat->GetString("model"));
			pModel->SetString(VarArgs("local/platforms/%s/download", AA_PLATFORM_ID), "");

			pModel->SetString(VarArgs("local/platforms/%s/workshopId", AA_PLATFORM_ID), "");
			pModel->SetString(VarArgs("local/platforms/%s/mountId", AA_PLATFORM_ID),"");

			// models can be loaded right away because they don't depend on anything else, like items do. (items depend on models)
			DevMsg("Loading cabinet model with ID %s and model %s\n", modelId.c_str(), cat->GetString("model"));
			m_models[modelId] = pModel;
		}

		pFilename = g_pFullFileSystem->FindNext(handle);
	}

	g_pFullFileSystem->FindClose(handle);
	cat->deleteThis();
}

void C_MetaverseManager::DetectAllMaps()
{
	/*
	bool bObsoleteLegacyTester = false;// true;
	if (bObsoleteLegacyTester)
	{
		// detect all saves now // OBSOLETE: save are exported from legacy mode now.  HOWEVER, the exporter only uses the 1st path, it doesn't search for saves in the workshop paths.
		std::string path = "A:\\SteamLibrary\\steamapps\\common\\Anarchy Arcade\\aarcade\\";
		g_pAnarchyManager->ScanForLegacySaveRecursive(path);

		std::string workshopPath;
		unsigned int max = g_pAnarchyManager->GetWorkshopManager()->GetNumDetails();
		for (unsigned int i = 0; i < max; i++)
		{
			SteamUGCDetails_t* pDetails = g_pAnarchyManager->GetWorkshopManager()->GetDetails(i);
			workshopPath = path + "workshop\\" + std::string(VarArgs("%llu", pDetails->m_nPublishedFileId)) + "\\";
			g_pAnarchyManager->ScanForLegacySaveRecursive(workshopPath);
		}
	}
	else
	{
	*/
		// Load all the .key files from the library/instances folder.
		// then add their instances:

		// make it use the new shinnit
		unsigned int count = 0;
		sqlite3_stmt *stmt = NULL;
		int rc = sqlite3_prepare(m_db, "SELECT * from instances", -1, &stmt, NULL);
		if (rc != SQLITE_OK)
			DevMsg("prepare failed: %s\n", sqlite3_errmsg(m_db));

		int length;
		std::string instanceId;
		std::string goodStyleName;
		std::string goodTitle;
		std::string goodLegacyFile;
		KeyValues* kv;
		while (sqlite3_step(stmt) == SQLITE_ROW)	// THIS IS WHERE THE LOOP CAN BE BROKEN UP AT!!
		{
			length = sqlite3_column_bytes(stmt, 1);

			// FIXME: TODO: Detect if the map is from a workshop or mount id too!
			kv = new KeyValues("instance");
			//if (kv->LoadFromFile(g_pFullFileSystem, VarArgs("library\\instances\\%s", pFilename), "MOD"))
			if (this->LoadSQLKevValues("instances", (const char*)sqlite3_column_text(stmt, 0), kv))
			{
				instanceId = kv->GetString("info/id");
				goodStyleName = kv->GetString("info/style");
				goodTitle = kv->GetString("info/title", kv->GetString("info/id"));
				goodLegacyFile = "";
				g_pAnarchyManager->GetInstanceManager()->AddInstance(instanceId, kv->GetString("info/map"), goodTitle, goodLegacyFile, "", "");
				//g_pAnarchyManager->GetInstanceManager()->AddInstance(instanceId, goodStyleName, goodTitle, goodLegacyFile, "", "");
				//g_pAnarchyManager->GetInstanceManager()->AddInstance(instanceId, goodStyleName, goodTitle, goodLegacyFile, "", "");
			}
			kv->deleteThis();
		}
		sqlite3_finalize(stmt);	// TODO: error checking?  Maybe not needed, if this is like a close() operation.
		//return count;
		/*
		DevMsg("NOW ADD THEIR INSTANCES TO THE MANAGER...\n");

		FileFindHandle_t testFileHandle;
		const char *pFilename = g_pFullFileSystem->FindFirstEx("library\\instances\\*.key", "MOD", &testFileHandle);

		std::string instanceId;
		std::string goodStyleName;
		std::string goodTitle;
		std::string goodLegacyFile;
		KeyValues* kv;
		while (pFilename != NULL)
		{
			if (g_pFullFileSystem->FindIsDirectory(testFileHandle))
			{
				pFilename = g_pFullFileSystem->FindNext(testFileHandle);
				continue;
			}

			// FIXME: TODO: Detect if the map is from a workshop or mount id too!
			kv = new KeyValues("instance");
			if (kv->LoadFromFile(g_pFullFileSystem, VarArgs("library\\instances\\%s", pFilename), "MOD"))
			{
				instanceId = kv->GetString("info/id");
				goodStyleName = kv->GetString("info/style");
				goodTitle = kv->GetString("info/title", kv->GetString("info/id"));
				goodLegacyFile = "";
				g_pAnarchyManager->GetInstanceManager()->AddInstance(instanceId, goodStyleName, goodTitle, goodLegacyFile, "", "");
			}

			kv->deleteThis();
			pFilename = g_pFullFileSystem->FindNext(testFileHandle);
		}
		g_pFullFileSystem->FindClose(testFileHandle);
		*/
	//}

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	bool bAlreadyExists;
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
}

void C_MetaverseManager::OnDetectAllMapsCompleted()
{
	DevMsg("Done detecting maps!\n");

	g_pAnarchyManager->GetMetaverseManager()->DetectAllMapScreenshots();
	g_pAnarchyManager->OnDetectAllMapsComplete();
}

KeyValues* C_MetaverseManager::GetActiveKeyValues(KeyValues* entry)
{
	// return null if given a null argument to avoid the if-then cluster fuck of error checking each step of this common task
	if (!entry)
		return null;

	//for (KeyValues *sub = entry->GetFirstSubKey(); sub; sub = sub->GetNextKey())
//		DevMsg("key value: %s %s\n", sub->GetName(), sub->GetString());

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

void C_MetaverseManager::FlagDynamicModels()
{
	std::string buf;
	KeyValues* active;
	std::map<std::string, KeyValues*>::iterator it = m_models.begin();
	while (it != m_models.end())
	{
		active = it->second->FindKey("current");
		if (!active)
			active = it->second->FindKey("local", true);

		buf = active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));
		if (buf.find("models\\cabinets\\") == 0 || buf.find("models/cabinets/") == 0 || buf.find("models\\banners\\") == 0 || buf.find("models/banners/") == 0 || buf.find("models\\frames\\") == 0 || buf.find("models/frames/") == 0 || buf.find("models\\icons\\") == 0 || buf.find("models/icons/") == 0)
		{
			active->SetInt("dynamic", 1);
			DevMsg("Flagged %s as dynamic model\n", buf.c_str());
		}

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
			active = it->second->FindKey("current");
			if (!active)
				active = it->second->FindKey("local", true);

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
			active = it->second->FindKey("current");
			if (!active)
				active = it->second->FindKey("local", true);

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
		KeyValues* pActive = pItem->FindKey("current");
		if (!pActive)
			pActive = pItem->FindKey("local");

		std::string id = pActive->GetString("info/id");
		/*
		if (id == "0ccb412c")
		{
			DevMsg("Loaded an instance of mermaid: %s%s\n", filePath.c_str(), file.c_str());
			//DevMsg("Loaded an instance of mermaid: %s\n", pActive->GetString("screen"));
		}
		*/

		std::vector<std::string> defaultFields;
		defaultFields.push_back("title");
		defaultFields.push_back("description");
		defaultFields.push_back("file");
		defaultFields.push_back("type");
		defaultFields.push_back("app");
		defaultFields.push_back("reference");
		defaultFields.push_back("preview");
		defaultFields.push_back("download");
		defaultFields.push_back("stream");
		defaultFields.push_back("screen");
		defaultFields.push_back("marquee");
		defaultFields.push_back("model");

		unsigned int max = defaultFields.size();
		for (unsigned int i = 0; i < max; i++)
		{
			if (!pActive->FindKey(defaultFields[i].c_str()))
				pActive->SetString(defaultFields[i].c_str(), "");
		}

		if (!pActive->FindKey("type") || !Q_strcmp(pActive->GetString("type"), ""))
			pActive->SetString("type", "-KKa1MHJTls2KqNphWFM");
		
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

unsigned int C_MetaverseManager::LoadAllLocalItems(std::string filePath)
{
	// make it use the new shinnit
	unsigned int count = 0;
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(m_db, "SELECT * from items", -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed: %s\n", sqlite3_errmsg(m_db));

	int length;
	while (sqlite3_step(stmt) == SQLITE_ROW)	// THIS IS WHERE THE LOOP CAN BE BROKEN UP AT!!
	{
		length = sqlite3_column_bytes(stmt, 1);

		KeyValues* pItem = new KeyValues("item");

		CUtlBuffer buf(0, length, 0);
		buf.CopyBuffer(sqlite3_column_blob(stmt, 1), length);
		pItem->ReadAsBinary(buf);

		// TODO: Look up any alias here first!!
		KeyValues* pActive = pItem->FindKey("current");
		if (!pActive)
			pActive = pItem->FindKey("local");

		count++;

		std::string id = pActive->GetString("info/id");

		std::vector<std::string> defaultFields;
		defaultFields.push_back("title");
		defaultFields.push_back("description");
		defaultFields.push_back("file");
		defaultFields.push_back("type");
		defaultFields.push_back("app");
		defaultFields.push_back("reference");
		defaultFields.push_back("preview");
		defaultFields.push_back("download");
		defaultFields.push_back("stream");
		defaultFields.push_back("screen");
		defaultFields.push_back("marquee");
		defaultFields.push_back("model");

		unsigned int max = defaultFields.size();
		for (unsigned int i = 0; i < max; i++)
		{
			if (!pActive->FindKey(defaultFields[i].c_str()))
				pActive->SetString(defaultFields[i].c_str(), "");
		}

		if (!pActive->FindKey("type") || !Q_strcmp(pActive->GetString("type"), ""))
			pActive->SetString("type", "-KKa1MHJTls2KqNphWFM");
		/*
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
		*/
		m_items[id] = pItem;
	}
	sqlite3_finalize(stmt);	// TODO: error checking?  Maybe not needed, if this is like a close() operation.
	return count;

	/*
	unsigned int count = 0;
	FileFindHandle_t testFileHandle;
	const char *pFilename = g_pFullFileSystem->FindFirst(VarArgs("%slibrary\\items\\*.key", filePath.c_str()), &testFileHandle);

	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(testFileHandle))
		{
			pFilename = g_pFullFileSystem->FindNext(testFileHandle);
			continue;
		}

		std::string foundName = pFilename;
		foundName = VarArgs("library\\items\\%s", pFilename);
		pFilename = g_pFullFileSystem->FindNext(testFileHandle);

		// MAKE THE FILE PATH NICE
		char path_buffer[AA_MAX_STRING];
		Q_strcpy(path_buffer, foundName.c_str());
		V_FixSlashes(path_buffer);

		for (int i = 0; path_buffer[i] != '\0'; i++)
			path_buffer[i] = tolower(path_buffer[i]);
		// FINISHED MAKING THE FILE PATH NICE

		foundName = path_buffer;
		if (this->LoadLocalItem(foundName, filePath))
			count++;
	}

	g_pFullFileSystem->FindClose(testFileHandle);
	return count;
	*/
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
		KeyValues* pActive = pModel->FindKey("current");
		if (!pActive)
			pActive = pModel->FindKey("local");

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

unsigned int C_MetaverseManager::LoadAllLocalModels(std::string filePath)
{
	// make it use the new shinnit
	unsigned int count = 0;
	sqlite3_stmt *stmt = NULL;
	int rc = sqlite3_prepare(m_db, "SELECT * from models", -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed: %s\n", sqlite3_errmsg(m_db));

	int length;
	while (sqlite3_step(stmt) == SQLITE_ROW)	// THIS IS WHERE THE LOOP CAN BE BROKEN UP AT!!
	{
		length = sqlite3_column_bytes(stmt, 1);

		KeyValues* pModel = new KeyValues("model");

		CUtlBuffer buf(0, length, 0);
		buf.CopyBuffer(sqlite3_column_blob(stmt, 1), length);
		pModel->ReadAsBinary(buf);

		// TODO: Look up any alias here first!!
		KeyValues* pActive = pModel->FindKey("current");
		if (!pActive)
			pActive = pModel->FindKey("local");

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

		m_models[id] = pModel;
		/*
		// TODO: Look up any alias here first!!
		KeyValues* pActive = pModel->FindKey("current");
		if (!pActive)
			pActive = pModel->FindKey("local");

		if (pActive)
		{
			std::string id = pActive->GetString("info/id");
			m_types[id] = pModel;
			count++;
			//DevMsg("adding type: %s\n", id.c_str());
		}
		else
		{
			pModel->deleteThis();
			pModel = null;
		}
		*/
	}
	sqlite3_finalize(stmt);	// TODO: error checking?  Maybe not needed, if this is like a close() operation.
	return count;
	/*
	unsigned int count = 0;
	FileFindHandle_t testFileHandle;
	const char *pFilename = g_pFullFileSystem->FindFirst(VarArgs("%slibrary\\models\\*.key", filePath.c_str()), &testFileHandle);

	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(testFileHandle))
		{
			pFilename = g_pFullFileSystem->FindNext(testFileHandle);
			continue;
		}

		std::string foundName = pFilename;
		foundName = VarArgs("library\\models\\%s", pFilename);
		pFilename = g_pFullFileSystem->FindNext(testFileHandle);

		// MAKE THE FILE PATH NICE
		char path_buffer[AA_MAX_STRING];
		Q_strcpy(path_buffer, foundName.c_str());
		V_FixSlashes(path_buffer);

		for (int i = 0; path_buffer[i] != '\0'; i++)
			path_buffer[i] = tolower(path_buffer[i]);
		// FINISHED MAKING THE FILE PATH NICE

		foundName = path_buffer;
		if (this->LoadLocalModel(foundName, filePath))
			count++;
	}

	g_pFullFileSystem->FindClose(testFileHandle);
	return count;
	*/
}

std::string C_MetaverseManager::ResolveLegacyModel(std::string legacyModel)
{
	// iterate through the models
	KeyValues* active;
	for (std::map<std::string, KeyValues*>::iterator it = m_models.begin(); it != m_models.end(); ++it)
	{
		active = it->second->FindKey("current");
		if (!active)
			active = it->second->FindKey("local");

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