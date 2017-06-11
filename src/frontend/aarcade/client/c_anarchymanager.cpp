#include "cbase.h"
#include "aa_globals.h"
#include "c_anarchymanager.h"
//#include "c_systemtime.h"
#include "WinBase.h"
#include <cctype>
#include <algorithm>
#include "c_browseslate.h"
//#include <regex>
#include "../../sqlite/include/sqlite/sqlite3.h"
//#include "mathlib/mathlib.h"
//#include <math.h>
#include <time.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_AnarchyManager g_AnarchyManager;
extern C_AnarchyManager* g_pAnarchyManager(&g_AnarchyManager);

C_AnarchyManager::C_AnarchyManager() : CAutoGameSystemPerFrame("C_AnarchyManager")
{
	DevMsg("AnarchyManager: Constructor\n");

	m_tabMenuFile = "taskMenu.html";	// OBSOLETE!!
	m_bIsShuttingDown = false;
	m_bIsHoldingPrimaryFire = false;
	m_pHoverGlowEntity = null;
	m_pLastNearestObjectToPlayerLook = null;

	m_bIsDisconnecting = false;

	m_iLastDynamicMultiplyer = -1;
	m_pPicMipConVar = null;
	m_pWeaponsEnabledConVar = null;

	// FIXME: ONLY WORKS FOR ME
	//m_legacyFolder = "A:\\SteamLibrary\\steamapps\\common\\Anarchy Arcade\\aarcade\\";
	m_legacyFolder = "";

	m_state = AASTATE_NONE;
	m_iState = 0;
	m_bSuspendEmbedded = false;
	m_bIncrementState = false;
	m_bPaused = false;
	m_pCanvasManager = null;
	//m_pWebManager = null;
	//m_pLoadingManager = null;
	m_pLibretroManager = null;
	m_pSteamBrowserManager = null;
	m_pMetaverseManager = null;
	m_pBackpackManager = null;
	m_pWindowManager = null;
	m_pInputManager = null;
	m_pSelectedEntity = null;
	m_pMountManager = null;
	m_pInstanceManager = null;
	m_dLastGenerateIdTime = 0;
	m_lastGeneratedChars = "000000000000";

	m_pNextLoadInfo = new nextLoadInfo_t();
	m_pNextLoadInfo->instanceId = "";
	m_pNextLoadInfo->position = "";
	m_pNextLoadInfo->rotation = "";
}

C_AnarchyManager::~C_AnarchyManager()
{
	DevMsg("AnarchyManager: Destructor\n");
}

bool C_AnarchyManager::Init()
{
	DevMsg("AnarchyManager: Init\n");

	KeyValues* legacyLog = new KeyValues("legacy");
	if (legacyLog->LoadFromFile(g_pFullFileSystem, "legacy_log.key", "MOD"))
	{
		m_legacyFolder = legacyLog->GetString("path");

		size_t found = m_legacyFolder.find("\\\\");
		while (found != std::string::npos)
		{
			m_legacyFolder.replace(found, 2, "\\");
			found = m_legacyFolder.find("\\\\");
		}
	}
	legacyLog->deleteThis();

	std::string workshopDir = engine->GetGameDirectory();	// just use the game directory to find workshop content normally.
	if (workshopDir == "d:\\projects\\aarcade-source\\game\\frontend")
		workshopDir = m_legacyFolder;

	workshopDir = workshopDir.substr(0, workshopDir.find_last_of("\\"));
	workshopDir = workshopDir.substr(0, workshopDir.find_last_of("\\"));
	workshopDir = workshopDir.substr(0, workshopDir.find_last_of("\\"));
	workshopDir = workshopDir.substr(0, workshopDir.find_last_of("\\"));
	workshopDir += "\\workshop\\content\\266430\\";
	m_workshopFolder = workshopDir;

	std::string aarcadeUserFolder = engine->GetGameDirectory();
	size_t found = aarcadeUserFolder.find_last_of("/\\");
	aarcadeUserFolder = aarcadeUserFolder.substr(0, found + 1);
	aarcadeUserFolder += std::string("aarcade_user");
	m_aarcadeUserFolder = aarcadeUserFolder;

	g_pFullFileSystem->CreateDirHierarchy("resource\\ui\\html", "DEFAULT_WRITE_PATH");

	return true;
}

void C_AnarchyManager::PostInit()
{
	DevMsg("AnarchyManager: PostInit\n");

	/*
	// allow for 2 instances of the Source engine to run
	// NOTE: There are conflicts with older Source engine games that use the default VGUI main menu, but other than that they work fine as well.
	bool keepGoing;

	// Disable mutexes
	keepGoing = true;
	while (keepGoing)
	{
		HANDLE myHandle = OpenMutex(MUTEX_ALL_ACCESS, true, "hl2_singleton_mutex");

		if (myHandle)
		{
			if (!ReleaseMutex(myHandle))
				myHandle = OpenMutex(MUTEX_ALL_ACCESS, true, "hl2_singleton_mutex");
			else
			{
				CloseHandle(myHandle);
				keepGoing = false;
			}
		}
		else
			keepGoing = false;
	}

	keepGoing = true;
	while (keepGoing)
	{
		HANDLE myHandle = OpenMutex(MUTEX_ALL_ACCESS, true, "ValvePlatformUIMutex");

		if (myHandle)
		{
			if (!ReleaseMutex(myHandle))
				myHandle = OpenMutex(MUTEX_ALL_ACCESS, true, "ValvePlatformUIMutex");
			else
			{
				CloseHandle(myHandle);
				keepGoing = false;
			}
		}
		else
			keepGoing = false;
	}


	//	keepGoing = true;
	//	while (keepGoing)
	{
		HANDLE myHandle = OpenMutex(MUTEX_ALL_ACCESS, true, "ValvePlatformWaitMutex");

		//		if (!ReleaseMutex(myHandle))
		//			myHandle = OpenMutex(MUTEX_ALL_ACCESS, true, "ValvePlatformWaitMutex");
		//		else
		//		{

		if (myHandle)
		{
			ReleaseMutex(myHandle);
			CloseHandle(myHandle);
		}
		//			keepGoing = false;
		//		}
	}
	*/
}

void C_AnarchyManager::OnStartup()
{
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	pHudBrowserInstance->Select();
	pHudBrowserInstance->Focus();
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pHudBrowserInstance);
}

void C_AnarchyManager::OnUpdateLibraryVersionCallback()
{
	int iLibraryVersion = m_pMetaverseManager->ExtractLibraryVersion();
	if (iLibraryVersion == -1)
		iLibraryVersion = 0;

	if (iLibraryVersion < AA_LIBRARY_VERSION)
	{
		DevMsg("Making a backup then converting your library from version %i to %i\n", iLibraryVersion, (iLibraryVersion + 1));
		if (m_pMetaverseManager->ConvertLibraryVersion(0, 1))
		{
			DevMsg("Done updating library.  Backup copy placed in the backups folder just in case.\n");
			//this->OnReadyToLoadUserLibrary();
		}
		else
			DevMsg("CRITICAL ERROR: Could not convert library version! Aborting! You should make a copy of the newest backup in aarcade_user/backups and replace the aarcade_user/library.db file with the backup and try again. If it still fails, let me know on during a Twitch stream and I'll help figure things out with you! - SM Sith Lord\n");
	}

	this->OnStartupCallback(true);
}

void C_AnarchyManager::OnStartupCallback(bool bForceDefaultAdd)
{
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	
	// Now start loading stuff in... (if this is our 1st time here...)
	m_pMetaverseManager->Init();

	// check if library version is obsolete
	int iLibraryVersion = m_pMetaverseManager->ExtractLibraryVersion();
	if (iLibraryVersion == -1)
		iLibraryVersion = 0;
	//DevMsg("CRITICAL ERROR: Could not extract library version from library.db!\n");

	if (iLibraryVersion < AA_LIBRARY_VERSION)
	{
		//pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Backing up & updating your library...", "userlibrary", "0", "0", "5", "defaultLibraryReadyCallback");
		pHudBrowserInstance->AddHudLoadingMessage("", "", "Backing Up & Updating Your Library", "updatelibraryversion", "", "", "", "updateLibraryVersionCallback");
		return;
	}

	bool bNeedsDefault = m_pMetaverseManager->IsEmptyDb();
	if (bNeedsDefault)
	{
		//pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Adding Default Library", "defaultlibrary", "", "4", "+", "addNextDefaultLibraryCallback");

		// the current DB is already created, but empty.
		// we just need to add stuff to it.
		m_pMetaverseManager->AddDefaultTables();
	}

	if (bNeedsDefault || iLibraryVersion < AA_LIBRARY_VERSION || bForceDefaultAdd)
	{	
		//this->OnAddNextDefaultLibraryCallback();
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Default Apps", "defaultapps", "", "", "+0");
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Default Cabinets", "defaultcabinets", "", "", "+0");
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Default Maps", "defaultmaps", "", "", "+0");
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Default Models", "defaultmodels", "", "", "+0");
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Default Types", "defaulttypes", "", "", "+0");
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Adding Default Library", "defaultlibrary", "", "5", "+0", "addNextDefaultLibraryCallback");
	}
	else
	{
		/*
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Types", "usertypes", "", "", "0");
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Cabinets", "usercabinets", "", "", "0");
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Models", "usermodels", "", "", "0");
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Apps", "userapps", "", "", "0");
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Items", "useritems", "", "", "0");
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Instances", "userinstances", "", "", "0");
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading User Library", "userlibrary", "0", "0", "5", "defaultLibraryReadyCallback");
		*/

		this->OnDefaultLibraryReady();
	}
}

void C_AnarchyManager::OnAddNextDefaultLibraryCallback()
{
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");

	addDefaultLibraryContext_t* context = m_pMetaverseManager->GetAddDefaultLibraryContext();

	if (!context)
	{
		context = new addDefaultLibraryContext_t();
		context->pDb = null;
		context->numApps = 0;
		context->numCabinets = 0;
		context->numMaps = 0;
		context->numModels = 0;
		context->numTypes = 0;
		context->kv = null;
		context->state = 0;

		m_pMetaverseManager->SetAddDefaultLibraryToDbIterativeContext(context);
	}

	// remember which value gets incremeneted
	unsigned int oldState = context->state;
	unsigned int oldNumApps = context->numApps;
	unsigned int oldNumCabinets = context->numCabinets;
	unsigned int oldNumMaps = context->numMaps;
	unsigned int oldNumModels = context->numModels;
	unsigned int oldNumTypes = context->numTypes;

	m_pMetaverseManager->AddDefaultLibraryToDbIterative(context);

	std::string callbackName = (context->state == 1) ? "" : "addNextDefaultLibraryCallback";

	if ( oldNumApps != context->numApps )
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Default Apps", "defaultapps", "", "", "+", callbackName);
	else if (oldNumCabinets != context->numCabinets)
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Default Cabinets", "defaultcabinets", "", "", "+", callbackName);
	else if (oldNumMaps != context->numMaps)
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Default Maps", "defaultmaps", "", "", "+", callbackName);
	else if (oldNumModels != context->numModels)
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Default Models", "defaultmodels", "", "", "+", callbackName);
	else if (oldNumTypes != context->numTypes)
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Default Types", "defaulttypes", "", "", "+", callbackName);

	// for now...
	std::string curState = "0";
	if (oldState != context->state)
	{
		if (context->state == 3)
			curState = "1";
		else if (context->state == 4)
			curState = "2";
		else if (context->state == 5)
			curState = "3";
		else if (context->state == 6)
			curState = "4";
		else if (context->state == 1)
			curState = "5";

		if (context->state != 1)
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Adding Default Library", "defaultlibrary", "", "5", curState);
	}

	if (context->state == 1)
	{
		m_pMetaverseManager->DeleteAddDefaultLibraryContext(context);
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Adding Default Library", "defaultlibrary", "", "5", "5", "defaultLibraryReadyCallback");
		//this->OnDefaultLibraryReady();
	}
}

void C_AnarchyManager::OnReadyToLoadUserLibrary()
{
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");

	unsigned int uCount;
	std::string num;


	// TODO: All of these should be incremental (per-group)
	// And continue starting up
	uCount = g_pAnarchyManager->GetMetaverseManager()->LoadAllLocalTypes();
	num = VarArgs("%u", uCount);
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Types", "usertypes", "", "", num);

	unsigned int uNumDynamic = 0;
	uCount = g_pAnarchyManager->GetMetaverseManager()->LoadAllLocalModels(uNumDynamic);
	num = VarArgs("%u", uNumDynamic);
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Cabinets", "usercabinets", "", "", num);
	num = VarArgs("%u", uCount - uNumDynamic);
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Models", "usermodels", "", "", num);

	uCount = g_pAnarchyManager->GetMetaverseManager()->LoadAllLocalApps();
	num = VarArgs("%u", uCount);
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Apps", "userapps", "", "", num);

	uCount = g_pAnarchyManager->GetMetaverseManager()->LoadAllLocalItems();
	num = VarArgs("%u", uCount);
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Items", "useritems", "", "", num);

	uCount = g_pAnarchyManager->GetMetaverseManager()->LoadAllLocalInstances();
	num = VarArgs("%u", uCount);
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Instances", "userinstances", "", "", num);

	//pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading User Library", "userlibrary", "0", "6", "6");

	// time to add in some search paths
	if (m_legacyFolder != "")
	{
		g_pFullFileSystem->AddSearchPath(m_legacyFolder.c_str(), "MOD", PATH_ADD_TO_TAIL);
		g_pFullFileSystem->AddSearchPath(m_legacyFolder.c_str(), "GAME", PATH_ADD_TO_TAIL);

		std::string workshopMapsPath = m_legacyFolder + std::string("workshop\\workshopmaps\\");
		g_pFullFileSystem->AddSearchPath(workshopMapsPath.c_str(), "GAME", PATH_ADD_TO_TAIL);

		std::string workshopFile;
		FileFindHandle_t findHandle;
		const char *pFilename = g_pFullFileSystem->FindFirstEx(VarArgs("%sworkshop\\*", m_legacyFolder.c_str()), "", &findHandle);
		while (pFilename != NULL)
		{
			workshopFile = m_legacyFolder + std::string("workshop\\") + std::string(pFilename);

			if (workshopFile.find(".vpk") == workshopFile.length() - 4)
			{
				DevMsg("Adding %s to the search paths.\n", workshopFile.c_str());
				g_pFullFileSystem->AddSearchPath(workshopFile.c_str(), "GAME", PATH_ADD_TO_TAIL);
			}

			pFilename = g_pFullFileSystem->FindNext(findHandle);
		}
		g_pFullFileSystem->FindClose(findHandle);

		std::string customFolder;
		pFilename = g_pFullFileSystem->FindFirstEx(VarArgs("%scustom\\*", m_legacyFolder.c_str()), "", &findHandle);
		while (pFilename != NULL)
		{
			if (Q_strcmp(pFilename, ".") && Q_strcmp(pFilename, "..") && g_pFullFileSystem->FindIsDirectory(findHandle))
			{
				customFolder = m_legacyFolder + std::string("custom\\") + std::string(pFilename);
				DevMsg("Adding %s to the search paths.\n", customFolder.c_str());
				g_pFullFileSystem->AddSearchPath(customFolder.c_str(), "GAME", PATH_ADD_TO_TAIL);
			}

			pFilename = g_pFullFileSystem->FindNext(findHandle);
		}
		g_pFullFileSystem->FindClose(findHandle);
	}

	// Create the mount manager & initialize it
	m_pMountManager = new C_MountManager();
	m_pMountManager->Init();
	m_pMountManager->LoadMountsFromKeyValues("mounts.txt");

	// Initialize the backpack manager (it already exists)
	m_pBackpackManager->Init();

	// Create the workshop manager & initialize it
	m_pWorkshopManager = new C_WorkshopManager();
	m_pWorkshopManager->Init();
}

void C_AnarchyManager::OnDefaultLibraryReadyCallback()
{
	this->OnDefaultLibraryReady();
}

void C_AnarchyManager::OnDefaultLibraryReady()
{
	/*
	// check if library version is obsolete
	int iLibraryVersion = m_pMetaverseManager->ExtractLibraryVersion();
	if (iLibraryVersion == -1)
		iLibraryVersion = 0;
		//DevMsg("CRITICAL ERROR: Could not extract library version from library.db!\n");

	if (iLibraryVersion < AA_LIBRARY_VERSION)
	{
		DevMsg("Making a backup then converting your library from version %i to %i\n", iLibraryVersion, (iLibraryVersion + 1));
		if (m_pMetaverseManager->ConvertLibraryVersion(0, 1))
		{
			DevMsg("Done updating library.  Backup copy placed in the backups folder just in case.\n");
			this->OnReadyToLoadUserLibrary();
		}
		else
			DevMsg("CRITICAL ERROR: Could not convert library version! Aborting! You should make a copy of the newest backup in aarcade_user/backups and replace the aarcade_user/library.db file with the backup and try again. If it still fails, let me know on during a Twitch stream and I'll help figure things out with you! - SM Sith Lord\n");
	}
	else
	*/
		//this->OnReadyToLoadUserLibrary();

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	/*
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Types", "usertypes", "", "", "0");
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Cabinets", "usercabinets", "", "", "0");
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Models", "usermodels", "", "", "0");
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Apps", "userapps", "", "", "0");
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Items", "useritems", "", "", "0");
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "User Instances", "userinstances", "", "", "0");
	*/
	//pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading User Library", "userlibrary", "0", "6", "0", "readyToLoadUserLibraryCallback");

	pHudBrowserInstance->AddHudLoadingMessage("", "", "Loading User Library...", "userlibrary", "", "", "", "readyToLoadUserLibraryCallback");
}

void C_AnarchyManager::Shutdown()
{
	DevMsg("AnarchyManager: Shutdown\n");
	m_bIsShuttingDown = true;
	/*
	if (m_pWebManager)
	{
		delete m_pWebManager;
		m_pWebManager = null;
	}
	*/

//	delete m_pLoadingManager;
	//m_pLoadingManager = null;

	if (m_pInputManager)
		m_pInputManager->DeactivateInputMode(true);

	if (m_pLibretroManager)
	{
		delete m_pLibretroManager;
		m_pLibretroManager = null;
	}

	if (m_pSteamBrowserManager)
	{
		delete m_pSteamBrowserManager;
		m_pSteamBrowserManager = null;
	}

	if (m_pMountManager)
	{
		delete m_pMountManager;
		m_pMountManager = null;
	}

	if (m_pWorkshopManager)
	{
		delete m_pWorkshopManager;
		m_pWorkshopManager = null;
	}

	if (m_pMetaverseManager)
	{
		delete m_pMetaverseManager;
		m_pMetaverseManager = null;
	}

	if (m_pInstanceManager)
	{
		delete m_pInstanceManager;
		m_pInstanceManager = null;
	}

	if (m_pAwesomiumBrowserManager)
	{
		delete m_pAwesomiumBrowserManager;
		m_pAwesomiumBrowserManager = null;
	}

	if (m_pInputManager)
	{
		//m_pInputManager->DeactivateInputMode(true);
		delete m_pInputManager;
		m_pInputManager = null;
	}

	DevMsg("AnarchyManager: Finished Shutdown\n");

	//g_pFullFileSystem->RemoveAllSearchPaths();	// doesn't make shutdown faster and causes warnings about failing to write cfg/server_blacklist.txt
}

void C_AnarchyManager::LevelInitPreEntity()
{
	DevMsg("AnarchyManager: LevelInitPreEntity\n");
	m_bIsDisconnecting = false;

	m_instanceId = m_pNextLoadInfo->instanceId;// m_nextInstanceId;

	C_AwesomiumBrowserInstance* pImagesBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("images");
	pImagesBrowserInstance->ResetImagesSession();

	this->SetTabMenuFile("taskMenu.html");

	DevMsg("Finished resetting image session.\n");
}

void C_AnarchyManager::LevelInitPostEntity()
{
	DevMsg("AnarchyManager: LevelInitPostEntity\n");

	//if (m_instanceId != "")
		//g_pAnarchyManager->GetInstanceManager()->LoadInstance(m_instanceId);
	if (m_pNextLoadInfo->instanceId != "")
		g_pAnarchyManager->GetInstanceManager()->LoadInstance(m_pNextLoadInfo->instanceId, m_pNextLoadInfo->position, m_pNextLoadInfo->rotation);
	else
		DevMsg("Map loaded with no instance chosen yet.  FIXME: Display a menu to choose an instance.\n");

	g_pAnarchyManager->SetNextLoadInfo();

	m_bSuspendEmbedded = false;

	engine->ClientCmd("sv_infinite_aux_power 1; mp_timelimit 0; sv_timeout 300; cl_timeout 300;\n");

//	engine->ClientCmd("r_drawothermodels 1;");
}

void C_AnarchyManager::LevelShutdownPreClearSteamAPIContext()
{
	DevMsg("AnarchyManager: LevelShutdownPreClearSteamAPIContext\n");
}

void C_AnarchyManager::LevelShutdownPreEntity()
{
	//engine->ClientCmd("r_drawothermodels 0;");	// FIXME: obsolete??

	DevMsg("AnarchyManager: LevelShutdownPreEntity\n");
	m_bSuspendEmbedded = true;
	
	C_BaseEntity* pEntity = this->GetSelectedEntity();
	if (pEntity)
		this->DeselectEntity();

	C_AwesomiumBrowserInstance* pImagesBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("images");
	pImagesBrowserInstance->ResetImagesSession();

///////	// all instances other than HUD and IMAGES should be removed at this time (until cross-map background item play gets re-enabled.)
	m_pCanvasManager->LevelShutdownPreEntity();

	/*
	C_WebTab* pWebTab = m_pWebManager->GetSelectedWebTab();
	if (pWebTab)
		m_pWebManager->DeselectWebTab(pWebTab);

	m_pWebManager->LevelShutdownPreEntity();
	*/
}

void C_AnarchyManager::LevelShutdownPostEntity()
{
	DevMsg("AnarchyManager: LevelShutdownPostEntity\n");

	if (m_pInstanceManager)
		m_pInstanceManager->LevelShutdownPostEntity();

	m_instanceId = "";	// wtf is this??  (its the name of the currently loaded instance, which *should* actually be held inside of the isntance manager.

	// Clear out the simple images
	// all instances other than HUD and IMAGES should be removed at this time (until cross-map background item play gets re-enabled.)
	m_pCanvasManager->LevelShutdownPostEntity();
}

void C_AnarchyManager::OnSave()
{
	DevMsg("AnarchyManager: OnSave\n");
}

void C_AnarchyManager::OnRestore()
{
	DevMsg("AnarchyManager: OnRestore\n");
}
void C_AnarchyManager::SafeRemoveIfDesired()
{
	//DevMsg("AnarchyManager: SafeRemoveIfDesired\n");
}

bool C_AnarchyManager::IsPerFrame()
{
	DevMsg("AnarchyManager: IsPerFrame\n");
	return true;
}

void C_AnarchyManager::PreRender()
{
	//DevMsg("AnarchyManager: PreRender\n");
	if(m_state == AASTATE_RUN)
		this->CheckPicMip();
}

void C_AnarchyManager::IncrementState()
{
	DevMsg("Increment state called.\n");

	if (m_bIncrementState)
		DevMsg("CRITICAL ERROR: State attempted to increment while it was still waiting for the previous increment!\n");

	m_bIncrementState = true;
}

void C_AnarchyManager::Update(float frametime)
{
	if (m_bIncrementState)
	{
		m_bIncrementState = false;

		switch (m_state)
		{
			case AASTATE_NONE:
				DevMsg("Incrementing to state AASTATE_INPUTMANAGER\n");
				m_state = AASTATE_INPUTMANAGER;
				break;

			case AASTATE_INPUTMANAGER:
				DevMsg("Incrementing to state AASTATE_CANVASMANAGER\n");
				m_state = AASTATE_CANVASMANAGER;
				break;

			case AASTATE_CANVASMANAGER:
				DevMsg("Incrementing to state AASTATE_LIBRETROMANAGER\n");
				m_state = AASTATE_LIBRETROMANAGER;
				break;

			case AASTATE_LIBRETROMANAGER:
				DevMsg("Incrementing to state AASTATE_STEAMBROWSERMANAGER\n");
				m_state = AASTATE_STEAMBROWSERMANAGER;
				break;

			case AASTATE_STEAMBROWSERMANAGER:
				DevMsg("Incrementing to state AASTATE_AWESOMIUMBROWSERMANAGER\n");
				m_state = AASTATE_AWESOMIUMBROWSERMANAGER;
				break;

			case AASTATE_AWESOMIUMBROWSERMANAGER:
				DevMsg("Incrementing to state AASTATE_AWESOMIUMBROWSERMANAGERWAIT\n");
				m_state = AASTATE_AWESOMIUMBROWSERMANAGERWAIT;
				break;

			case AASTATE_AWESOMIUMBROWSERMANAGERWAIT:
				DevMsg("Incrementing to state AASTATE_AWESOMIUMBROWSERMANAGERHUD\n");
				m_state = AASTATE_AWESOMIUMBROWSERMANAGERHUD;
				break;

			case AASTATE_AWESOMIUMBROWSERMANAGERHUD:
				DevMsg("Incrementing to state AASTATE_AWESOMIUMBROWSERMANAGERHUDWAIT\n");
				m_state = AASTATE_AWESOMIUMBROWSERMANAGERHUDWAIT;
				break;

			case AASTATE_AWESOMIUMBROWSERMANAGERHUDWAIT:
				DevMsg("Incrementing to state AASTATE_AWESOMIUMBROWSERMANAGERHUDINIT\n");
				m_state = AASTATE_AWESOMIUMBROWSERMANAGERHUDINIT;
				break;

			case AASTATE_AWESOMIUMBROWSERMANAGERHUDINIT:
				DevMsg("Incrementing to state AASTATE_AWESOMIUMBROWSERMANAGERIMAGES\n");
				m_state = AASTATE_AWESOMIUMBROWSERMANAGERIMAGES;
				break;

			case AASTATE_AWESOMIUMBROWSERMANAGERIMAGES:
				DevMsg("Incrementing to state AASTATE_AWESOMIUMBROWSERMANAGERIMAGESWAIT\n");
				m_state = AASTATE_AWESOMIUMBROWSERMANAGERIMAGESWAIT;
				break;

			case AASTATE_AWESOMIUMBROWSERMANAGERIMAGESWAIT:
				DevMsg("Incrementing to state AASTATE_AWESOMIUMBROWSERMANAGERIMAGESINIT\n");
				m_state = AASTATE_AWESOMIUMBROWSERMANAGERIMAGESINIT;
				break;

			case AASTATE_AWESOMIUMBROWSERMANAGERIMAGESINIT:
				DevMsg("Incrementing to state AASTATE_RUN\n");
				m_state = AASTATE_RUN;
				break;
		}
	}

	switch (m_state)
	{
		case AASTATE_RUN:
			if (m_bPaused)	// FIXME: You might want to let the web manager do its core logic, but don't render anything.
				return;	// TODO: Nothing else but texture proxy & callback-induced code needs to worry about paused mode now.

			//DevMsg("Float: %f\n", frametime);	// deltatime
			//DevMsg("Float: %i\n", gpGlobals->framecount);	// numframes total

			if (m_pCanvasManager)
				m_pCanvasManager->Update();

			if (m_pLibretroManager)
				m_pLibretroManager->Update();

			if (m_pSteamBrowserManager)
				m_pSteamBrowserManager->Update();

			if (m_pAwesomiumBrowserManager)
				m_pAwesomiumBrowserManager->Update();

			if (m_pInstanceManager)
				m_pInstanceManager->Update();

			if (m_pMetaverseManager)
				m_pMetaverseManager->Update();

			if (m_pBackpackManager)
				m_pBackpackManager->Update();

			if (m_pInputManager)
				m_pInputManager->Update();

			//g_pAnarchyManager->CheckPicMip();
			/*
			if (m_pWebManager)
				m_pWebManager->Update();
			*/

			//DevMsg("AnarchyManager: Update\n");
			break;

		case AASTATE_INPUTMANAGER:
			m_pInputManager = new C_InputManager();	// then wait for state change
			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_CANVASMANAGER:
			m_pCanvasManager = new C_CanvasManager();	// then wait for state change
			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_LIBRETROMANAGER:
			m_pLibretroManager = new C_LibretroManager();
			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_STEAMBROWSERMANAGER:
			m_pSteamBrowserManager = new C_SteamBrowserManager();
			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGER:
			m_pAwesomiumBrowserManager = new C_AwesomiumBrowserManager();
			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERWAIT:
			m_pCanvasManager->Update();
			m_pAwesomiumBrowserManager->Update();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERHUD:
			m_pAwesomiumBrowserManager->CreateAwesomiumBrowserInstance("hud", "asset://ui/hud.html", "AArcade HUD", true);
			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERHUDWAIT:
			m_pCanvasManager->Update();
			m_pAwesomiumBrowserManager->Update();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERHUDINIT:
			DevMsg("Finished initing HUD.\n");

			m_pInstanceManager = new C_InstanceManager();
			m_pMetaverseManager = new C_MetaverseManager();
			//pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Meta Scrapers", "loadmetascrapers", "", "", "69", "loadNextMetaScraper");
			m_pMetaverseManager->UpdateScrapersJS();	// make sure everything that hud.js loads is ready right away, to avoid cache issues

			//m_pMetaverseManager->Init();	// try to delay this until after startup
			m_pBackpackManager = new C_BackpackManager();
			m_pInputManager = new C_InputManager();

			// insta-startup the Window Manager for now, because it's just a placeholder until actual in-game deskto rendering can be done.
			m_pWindowManager = new C_WindowManager();
			m_pWindowManager->Init();

			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERIMAGES:
			m_pAwesomiumBrowserManager->CreateAwesomiumBrowserInstance("images", "asset://ui/imageLoader.html", "AArcade Image Renderer", true);	// defaults to asset://ui/blank.html	// does this need to be created here????
			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERIMAGESWAIT:
			//if (m_pCanvasManager)
			m_pCanvasManager->Update();

			m_pAwesomiumBrowserManager->Update();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERIMAGESINIT:
			DevMsg("Finished initing IMAGES.\n");

			// auto-load aarcade stuff
			g_pAnarchyManager->RunAArcade();

			this->IncrementState();
			break;
	}
}

void C_AnarchyManager::CheckPicMip()
{
	int multiplyer = this->GetDynamicMultiplyer();
	if (m_iLastDynamicMultiplyer != multiplyer)
	{
		if (m_iLastDynamicMultiplyer != -1)
		{
			// this is NOT the first time the mip quality has been detected.
			DevMsg("Mip has changed!\n");	// DOES NOT WORK
		}

		m_iLastDynamicMultiplyer = multiplyer;
	}
}

int C_AnarchyManager::GetDynamicMultiplyer()
{
	if (!m_pPicMipConVar)
		m_pPicMipConVar = cvar->FindVar("mat_picmip");

	int multiplyer = 1;
	int mip = m_pPicMipConVar->GetInt();
	if (mip == 2)
		multiplyer = 4;
	else if (mip == 1)
		multiplyer = 2;

	return multiplyer;

	/*
	if (m_iDynamicMultiplyer == -1)
		this->CalculateDynamicMultiplyer();
	
	return m_iDynamicMultiplyer;
	*/
}

//void C_AnarchyManager::CalculateDynamicMultiplyer()
//{
	/*
	if (!m_pPicMipConVar)
		m_pPicMipConVar = cvar->FindVar("mat_picmip");

	int multiplyer = 1;
	int mip = m_pPicMipConVar->GetInt();
	if (mip == 2)
		multiplyer = 4;
	else if (mip == 1)
		multiplyer = 2;

	m_iDynamicMultiplyer = multiplyer;
	*/
//}

void C_AnarchyManager::SpecialReady(C_AwesomiumBrowserInstance* pInstance)
{
	std::string id = pInstance->GetId();
	/*
	auto it = m_specialInstances.find(id);
	if (it != m_specialInstances.end())
		it->second = true;

	// check if ALL specials are ready
	bool bAllReady = true;

	it = m_specialInstances.begin();
	while (it != m_specialInstances.end())
	{
		if (!it->second)
		{
			bAllReady = false;
			break;
		}

		it++;
	}
	*/
	if (id == "hud" && AASTATE_AWESOMIUMBROWSERMANAGERHUDWAIT)	// is this too early??
		g_pAnarchyManager->IncrementState();
	else if (id == "images" && AASTATE_AWESOMIUMBROWSERMANAGERIMAGESWAIT)
		g_pAnarchyManager->IncrementState();
}

#include "vgui/IInput.h"
#include "ienginevgui.h"
bool C_AnarchyManager::HandleUiToggle()
{
	// ignore the start button on the gamepad if we are NOT the focused window
	HWND myHWnd = FindWindow(null, "AArcade: Source");
	//HWND myHWnd = GetActiveWindow();
	//HWND topHWnd = GetTopWindow(null);
	HWND foregroundWindow = GetForegroundWindow();
	//if (myHWnd != topHWnd)
	if (myHWnd != foregroundWindow)
		return true;

	if (m_bPaused)
	{
		this->Unpause();

		if (!engine->IsInGame())
			engine->ClientCmd("main_menu\n");

		return true;
	}

	// handle escape if in pause mode (ignore it)
	if (!engine->IsInGame())
	{
		// GOOD MAIN MENU EMBEDDED APP ESCAPE BINDS AS OF 9/13/2016

		if (m_pSteamBrowserManager)
		{
			C_SteamBrowserInstance* pInstance = m_pSteamBrowserManager->GetSelectedSteamBrowserInstance();
			if (pInstance && pInstance->GetId().find("scrape") == 0)	// web tabs starting with scrape in their ID get auto-deleted when input mode closes.
				m_pInputManager->DeactivateInputMode(true);
			else
			{
				//if (m_pInputManager->GetMainMenuMode() && m_pInputManager->GetInputMode() && m_pInputManager->GetFullscreenMode() && pInstance && pInstance->GetTexture() && pInstance->GetTexture() == m_pInputManager->GetInputCanvasTexture())
				if (pInstance && g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance)
				{
					m_pSteamBrowserManager->DestroySteamBrowserInstance(pInstance);
					m_pInputManager->SetEmbeddedInstance(null);
					return true;
				}
			}
			return true;
		}

		if (m_pLibretroManager && !vgui::input()->IsKeyDown(KEY_XBUTTON_START))
		{
			C_LibretroInstance* pInstance = m_pLibretroManager->GetSelectedLibretroInstance();
			//if (m_pInputManager->GetMainMenuMode() && m_pInputManager->GetInputMode() && m_pInputManager->GetFullscreenMode() && pInstance && pInstance->GetTexture() && pInstance->GetTexture() == m_pInputManager->GetInputCanvasTexture())
			if (pInstance && g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance)
			{
				m_pLibretroManager->DestroyLibretroInstance(pInstance);
				m_pInputManager->SetEmbeddedInstance(null);
				return true;
			}
		}

		if (m_pAwesomiumBrowserManager)
		{
			C_AwesomiumBrowserInstance* pInstance = m_pAwesomiumBrowserManager->GetSelectedAwesomiumBrowserInstance();
			//if (m_pInputManager->GetMainMenuMode() && m_pInputManager->GetInputMode() && m_pInputManager->GetFullscreenMode() && pInstance && pInstance->GetTexture() && pInstance->GetTexture() == m_pInputManager->GetInputCanvasTexture())
			if (pInstance && g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance )
			{
				//if (pInstance->GetId() == "hud" && )// || pInstance->GetId() == "images" )
				if (engine->IsInGame() || pInstance->GetId() != "hud" )
				{
					m_pAwesomiumBrowserManager->DestroyAwesomiumBrowserInstance(pInstance);
					m_pInputManager->SetEmbeddedInstance(null);
					return true;
				}
				else if (!engine->IsInGame() && pInstance->GetId() == "hud" )
				{
					g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

						//C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
						//pHudBrowserInstance->SetUrl("asset://ui/welcome.html");
						//g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true);
				}
			}
		}

		return false;
	}

	// update this code block when joypad input gets restricted to the selected input slate instance!!
	// (currently it will assume that a selected cabinet with a libretro instances ALWAYS wants joypad input!!
	// ignore if its a libretro instance
	C_BaseEntity* pEntity = this->GetSelectedEntity();
	if (pEntity)
	{
		C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);
		if (pShortcut)
		{
			std::vector<C_EmbeddedInstance*> embeddedInstances;
			pShortcut->GetEmbeddedInstances(embeddedInstances);

			C_LibretroInstance* pLibretroInstance;
			unsigned int max = embeddedInstances.size();
			for (unsigned int i = 0; i < max; i++)
			{
				pLibretroInstance = dynamic_cast<C_LibretroInstance*>(m_pInputManager->GetEmbeddedInstance());
				if (pLibretroInstance && vgui::input()->IsKeyDown(KEY_XBUTTON_START))
					return true;
			}
		}
	}

	if (m_pInputManager->GetInputMode())
	{
		if (this->GetMetaverseManager()->GetSpawningObjectEntity())
		{
			g_pAnarchyManager->DeactivateObjectPlacementMode(false);

			// undo changes AND cancel
			C_PropShortcutEntity* pShortcut = g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity();
			g_pAnarchyManager->DeactivateObjectPlacementMode(false);

			//std::string id = pShortcut->GetObjectId();
			//g_pAnarchyManager->GetInstanceManager()->ResetObjectChanges(pShortcut);

			// "save" cha
			//m_pInstanceManager->ApplyChanges(id, pShortcut);
			DevMsg("CHANGES REVERTED\n");
		}
		else
		{
			// handle escape if in fullscreen input mode (drop out of fullscreen mode)
			if ((!m_pInputManager->GetFullscreenMode() || !this->GetSelectedEntity() || m_pInputManager->GetWasForceInputMode()) || (this->GetSelectedEntity() && m_pInputManager->GetFullscreenMode()))
				m_pInputManager->DeactivateInputMode(true);
			else
				m_pInputManager->SetFullscreenMode(false);
		}

		return true;
	}
	else if (!m_pInputManager->GetInputMode() && engine->IsInGame() )
	{
		// handle escape if not in input mode & map is loaded (display the main menu)
		//engine->IsInGame()
		//engine->IsPaused()
		if (!enginevgui->IsGameUIVisible())
		{
			C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");

			//DevMsg("DISPLAY MAIN MENU\n");
			if (m_pSelectedEntity)
			{
				DevMsg("Deselecting entity.\n");
				this->DeselectEntity("asset://ui/welcome.html");
			}
			else
				pHudBrowserInstance->SetUrl("asset://ui/welcome.html");

			DevMsg("Trying to activate input mode.\n");
			m_pInputManager->ActivateInputMode(true, true);
			return false;
		}
	}

	return false;
}

bool C_AnarchyManager::HandleCycleToNextWeapon()
{
	if (this->GetMetaverseManager()->GetSpawningObject())
	{
		g_pAnarchyManager->GetInputManager()->OnMouseWheeled(1);
		return true;
	}
	else if (m_bIsHoldingPrimaryFire)
	{
		g_pAnarchyManager->GetInputManager()->OnMouseWheeled(1);
		return true;
	}

	return false;
}

bool C_AnarchyManager::HandleCycleToPrevWeapon()
{
	if (this->GetMetaverseManager()->GetSpawningObject())
	{
		g_pAnarchyManager->GetInputManager()->OnMouseWheeled(-1);
		return true;
	}
	else if (this->m_bIsHoldingPrimaryFire)
	{
		g_pAnarchyManager->GetInputManager()->OnMouseWheeled(-1);
		return true;
	}

	return false;
}

void C_AnarchyManager::Pause()
{
	m_bPaused = true;
	//this->HardPause();
}

void C_AnarchyManager::Unpause()
{
	//this->WakeUp();
	m_bPaused = false;

	m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud")->SetUrl("asset://ui/default.html");
	m_pInputManager->DeactivateInputMode(true);

	if (cvar->FindVar("broadcast_mode")->GetBool())
	{
		// Write this live URL out to the save file.
		std::string XSPlitLiveFolder = cvar->FindVar("broadcast_folder")->GetString();
		if (XSPlitLiveFolder != "")
		{
			if (cvar->FindVar("broadcast_auto_game")->GetBool())
				this->WriteBroadcastGame("Anarchy Arcade");

			// Also update a JS file to force the page to re-load
			FileHandle_t hFile = g_pFullFileSystem->Open(VarArgs("%s\\vote.js", XSPlitLiveFolder.c_str()), "a+", "");
			if (hFile)
			{
				std::string code = "gAnarchyTV.OnAArcadeCommand(\"finishPlaying\");\n";
				g_pFullFileSystem->Write(code.c_str(), code.length(), hFile);
				g_pFullFileSystem->Close(hFile);
			}
		}
	}
}

bool C_AnarchyManager::AlphabetSafe(std::string text, std::string in_alphabet)
{
	std::string alphabet = (in_alphabet != "") ? in_alphabet : " 1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ`-=[]\\;',./~!@#$%^&*()_+{}|:<>?";
	char textBuf[AA_MAX_STRING];
	Q_strcpy(textBuf, text.c_str());

	unsigned int i = 0;
	char tester = textBuf[i];
	while (tester)
	{
		if (alphabet.find(tester) == std::string::npos)
			return false;

		i++;
		tester = textBuf[i];
	}

	return true;
}

bool C_AnarchyManager::PrefixSafe(std::string text)
{
	// check for web
	if (text.find("http://") == 0 || text.find("https://") == 0 || text.find("steam://") == 0)
		return true;

	// check for integer-only value
	if (!Q_strcmp(VarArgs("%llu", Q_atoui64(text.c_str())), text.c_str()))
		return true;

	// check for single driveletter synatx
	std::string driveAlphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if (driveAlphabet.find(text.at(0)) != std::string::npos && text.at(1) == ':' && (text.at(2) == '\\' || text.at(2) == '/'))
		return true;

	/*
	std::regex re("[a-zA-Z]:[\\\\/].*");
	std::smatch match;
	if (std::regex_search(text, match, re) && match.size() > 1)
		return true;
	*/

	// NOT web, NOT steam app ID, and INVALID driveletter syntax
	return false;
}

bool C_AnarchyManager::DirectorySafe(std::string text)
{
	// directories do not begin with a blank space or a % symbol
	if (text.at(0) == ' ' || text.at(0) == '%' )
		return false;

	// directories also need to start with a safe character (that is out of the scope of this method.  AlphabetSafe can always be called to test that.
	//if (!this->AlphabetSafe(text.substr(0, 1)))
	//	 return false;

	// - The following sequences should not be allowed: /.., \..
	if (text.find("/..") != std::string::npos || text.find("\\..") != std::string::npos)
		return false;

	// - The following folders should be blacklisted: driveletter:/windows, driveletter:\windows
	if (text.length() > 7)
	{
		if (!Q_stricmp(text.substr(3, 7).c_str(), "windows"))
			return false;
	}

	return true;
}

void C_AnarchyManager::AddSubKeysToKeys(KeyValues* kv, KeyValues* targetKV)
{
	if (!kv)
		return;

	for (KeyValues *sub = kv->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	{
		if (sub->GetFirstSubKey())
		{
			KeyValues* key = targetKV->FindKey(sub->GetName(), true);
			AddSubKeysToKeys(sub, targetKV);
		}
		else
			targetKV->SetString(sub->GetName(), sub->GetString());
	}
}

void C_AnarchyManager::LoadMapCommand(std::string mapId, std::string instanceId, std::string position, std::string rotation, std::string screenshotId)
{
	m_pMetaverseManager->SetLoadingScreenshotId(screenshotId);

	KeyValues* map = g_pAnarchyManager->GetMetaverseManager()->GetMap(mapId);
	KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(map);

	std::string mapName = active->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file");
	mapName = mapName.substr(0, mapName.length() - 4);

	std::string title = "Unnamed (" + mapName + ")";

	if (instanceId == "")
		instanceId = g_pAnarchyManager->GetInstanceManager()->CreateBlankInstance(0, null, "", mapId, title);

	g_pAnarchyManager->SetNextLoadInfo(instanceId, position, rotation);
	//g_pAnarchyManager->SetNextInstanceId(instanceId);

	//g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
	//C_AwesomiumBrowserInstance* pHudInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
	//std::string url = "asset://ui/loading.html?map=" + mapId + "&instance=" + instanceId;
	//pHudInstance->SetUrl(url);

	engine->ClientCmd(VarArgs("map \"%s\"\n", mapName.c_str()));
}

bool C_AnarchyManager::WeaponsEnabled()
{
	if (!m_pWeaponsEnabledConVar)
		return false;
	else
		return m_pWeaponsEnabledConVar->GetBool();
}

bool C_AnarchyManager::ExecutableSafe(std::string text)
{
	// RULES:
	// 1. [text] must only consist of characters in the safe alphabet.
	// 2. [text] should be required to start with: http://, https://, steam://, driveletter:/, driveletter:\, or JUST an integer (to indicate a Steam App ID)
	// 3. The following sequences should not be allowed: /.., \..
	// 4. The following folders should be blacklisted: driveletter:/windows, driveletter:\windows
	// 5. Environment variable paths are not allowed: %APPDATA%, etc.
	// 6. Directories cannot begin with a blank space.

	return (this->PrefixSafe(text) && this->DirectorySafe(text));
}

uint64 C_AnarchyManager::GetTimeNumber()
{
	time_t currentrawtime = time(NULL);
	tm* timeinfo = localtime(&currentrawtime);
	return Q_atoui64(VarArgs("%llu", currentrawtime));

	/* whats this for?
	uint64 bigtime = currentrawtime;
	int lowerHalf = (int)((bigtime << 32) >> 32);
	int upperHalf = (int)((bigtime >> 32) << 32);

	time_t constructedTime = lowerHalf | (upperHalf << 32);

	tm* constructedtimeinfo = localtime(&constructedTime);
	DevMsg("Constructed time is: %s\n", asctime(constructedtimeinfo));
	DevMsg("As a uint64 it is: %llu\n", constructedTime);
	asctime(timeinfo)
	*/
}

std::string C_AnarchyManager::GetTimeString()
{
	time_t currentrawtime = time(NULL);
	tm* timeinfo = localtime(&currentrawtime);
	std::string timeString = VarArgs("%s", asctime(timeinfo));
	return timeString;
}

C_SteamBrowserInstance* C_AnarchyManager::AutoInspect(KeyValues* pItemKV, std::string tabId, C_PropShortcutEntity* pShortcut)
{
	std::string itemId = pItemKV->GetString("info/id");
	std::string gooodTabTitle = (tabId == "") ? "auto" + itemId : tabId;

	std::string uri = "file://";
	uri += engine->GetGameDirectory();
	uri += "/resource/ui/html/autoInspectItem.html?id=" + encodeURIComponent(itemId) + "&title=" + encodeURIComponent(pItemKV->GetString("title")) + "&screen=" + encodeURIComponent(pItemKV->GetString("screen")) + "&marquee=" + encodeURIComponent(pItemKV->GetString("marquee")) + "&preview=" + encodeURIComponent(pItemKV->GetString("preview")) + "&reference=" + encodeURIComponent(pItemKV->GetString("reference")) + "&file=" + encodeURIComponent(pItemKV->GetString("file"));

	// FIXME: Need to allow HTTP redirection (302).
	//DevMsg("Test URI is: %s\n", uri.c_str());	// FIXME: Might want to make the slashes in the game path go foward.

	int entIndex = (pShortcut) ? pShortcut->entindex() : -1;

	C_SteamBrowserInstance* pSteamBrowserInstance = m_pSteamBrowserManager->CreateSteamBrowserInstance();
	pSteamBrowserInstance->Init(gooodTabTitle, uri, "Newly selected item...", null, entIndex);
	pSteamBrowserInstance->SetOriginalItemId(itemId);	// FIXME: do we need to do this for original entindex too???
	//pSteamBrowserInstance->SetOriginalEntIndex(pShortcut->entindex());	// probably NOT needed??

	return pSteamBrowserInstance;
	//pEmbeddedInstance = pSteamBrowserInstance;
}

void C_AnarchyManager::WriteBroadcastGame(std::string gameTitle)
{
	cvar->FindVar("broadcast_game")->SetValue(gameTitle.c_str());

	std::string broadcastFolder = cvar->FindVar("broadcast_folder")->GetString();
	FileHandle_t hFile = g_pFullFileSystem->Open(VarArgs("%s\\game.txt", broadcastFolder.c_str()), "w+", "");
	if (hFile)
	{
		std::string xml = "";
		xml += "<div class=\"response\">\n";
		xml += "\t<activetitle class=\"activetitle\">";

		std::string xmlBuf = gameTitle;

		// Make it XML safe
		size_t found = xmlBuf.find("&");
		while (found != std::string::npos)
		{
			xmlBuf.replace(found, 1, "&amp;");
			found = xmlBuf.find("&", found + 5);
		}

		found = xmlBuf.find("<");
		while (found != std::string::npos)
		{
			xmlBuf.replace(found, 1, "&lt;");
			found = xmlBuf.find("<", found + 4);
		}

		found = xmlBuf.find(">");
		while (found != std::string::npos)
		{
			xmlBuf.replace(found, 1, "&gt;");
			found = xmlBuf.find(">", found + 4);
		}

		xml += xmlBuf;

		xml += "</activetitle>\n";

		xml += "</div>";

		g_pFullFileSystem->Write(xml.c_str(), xml.length(), hFile);
		g_pFullFileSystem->Close(hFile);
	}
}

launchErrorType_t C_AnarchyManager::LaunchItem(std::string id)
{
	/*
	// these items need improvements on how they are launched (they didn't work):
	Launching Item:
	Executable: C:\Users\Owner\Desktop\launcher - Shortcut.lnk
	Directory:
	Master Commands:

	ALSO roms with full file locations as their file instead of just the short filename.
	*/
	DevMsg("LAUNCH THE SHIT!\n");

	// genereate the executable, executableDirectory, and masterCommands values.
	// then give these to the actual launch method.

	bool bUnknownError = false;

	// user resolvable errors to catch
	bool bItemGood = false;
	bool bItemFileGood = false;
	bool bItemPathAllowed = false;
	bool bAppGood = false;
	bool bAppExecutableGood = false;
	bool bAppFilepathGood = false;
	bool bAtLeastOneAppFilepathExists = false;
	bool bReadyToActuallyLaunch = false;

	// required fields for ArcadeCreateProcess
	std::string executable;
	std::string executableDirectory;
	std::string masterCommands;

	// other fields used to generate the required fields
	bool bHasApp = false;
	bool bHasAppFilepath = false;
	KeyValues* item = null;	// the KV of the item being used.
	KeyValues* itemActive = null;	// the active node of the item KV.
	std::string file;
	std::string composedFile;
	KeyValues* app = null;
	KeyValues* appActive = null;
	std::string appExecutable;
	std::string appFilepath;
	std::string appCommands;

	launchErrorType_t errorType = NONE;
	/*
	if (!bItemGood)
	errorType = ITEM_NOT_FOUND;
	else if (!bItemFileGood)
	errorType = ITEM_FILE_NOT_FOUND;
	else if (!bItemPathAllowed)
	errorType = ITEM_FILE_PATH_RESTRICTED;
	else if (!bAppGood)
	errorType = APP_NOT_FOUND;
	else if (!bAppExecutableGood)
	errorType = APP_FILE_NOT_FOUND;
	else if (!bAppFilepathGood)
	errorType = APP_PATH_NOT_FOUND;
	*/

	// attempt to get the item
	item = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(id);
	if (item)
	{
		// if there is an item, attempt to get the active node kv
		itemActive = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(item);
		bItemGood = true;

		// if there is an active node kv for the item, attempt to get file
		file = itemActive->GetString("file");
		if (file != "")
		{
			// does it have an app?
			if (Q_strcmp(itemActive->GetString("app"), ""))
			{
				bHasApp = true;

				app = g_pAnarchyManager->GetMetaverseManager()->GetLibraryApp(itemActive->GetString("app"));
				appActive = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(app);
				bAppGood = true;

				// if there is an app, attempt to get its executable
				appExecutable = appActive->GetString("file");
				if (appExecutable != "")
				{
					bAppExecutableGood = true;

					// just grab the FIRST filepath for now.
					// FIXME: Need to keep searching through filepaths until the item's file is found inside of one.
					// Note: Apps are not required to have a filepath specified.
					std::string testFile;
					std::string testPath;
					KeyValues* filepaths = appActive->FindKey("filepaths", true);
					for (KeyValues *sub = filepaths->GetFirstSubKey(); sub; sub = sub->GetNextKey())
					{
						// true if even 1 filepath exists for the app, even if it is not found on the local PC.
						// (because in that case the local user probably needs to specify a correct location for it.)
						bHasAppFilepath = true;

						testPath = sub->GetString("path");

						// test if this path exists
						// FIXME: always assume it exists for now
						//if (true)
						if (g_pFullFileSystem->FileExists(testPath.c_str()))
						{
							bAtLeastOneAppFilepathExists = true;

							// test if the file exists inside of this filepath
							testFile = testPath + file;

							// FIXME: always assume the file exists in this path for now.
							if (g_pFullFileSystem->FileExists(testFile.c_str()))
							{
								composedFile = testFile;
								appFilepath = testPath;
								bAppFilepathGood = true;
								bItemFileGood = true;
								break;
							}
						}
					}

					// resolve the composedFile now
					if (!bAppFilepathGood)
						composedFile = file;

					// generate the commands
					// try to apply a command format
					appCommands = appActive->GetString("commandformat");
					if (appCommands != "")
					{
						// if the app has a command syntax, replace item variables with their values.

						// replace $FILE with active->GetString("file")
						// replace $QUOTE with a double quote
						// replace $SHORTFILE with active->GetString("file")'s filename only
						// replace etc.

						size_t found;

						found = appCommands.find("$FILE");
						if (found != std::string::npos)
						{
							if (this->AlphabetSafe(composedFile) && this->DirectorySafe(composedFile))
							{
								while (found != std::string::npos)
								{
									appCommands.replace(found, 5, composedFile);
									found = appCommands.find("$FILE");
								}
							}
							else
								errorType = UNKNOWN_ERROR;
						}

						found = appCommands.find("$QUOTE");
						while (found != std::string::npos)
						{
							appCommands.replace(found, 6, "\"");
							found = appCommands.find("$QUOTE");
						}
					}
					else
					{
						// otherwise, apply the default Windows command syntax for "open with"
						appCommands = "\"" + composedFile + "\"";
					}
				}
			}

			if (!bAppExecutableGood)
				composedFile = file;

			if (!bAppFilepathGood)
			{
				// check if the file exists.
				// (file could also be an HTTP or STEAM protocol address at this point.)

				// are we a web address missing an http?
				if (composedFile.find("www.") == 0)
					composedFile = "http://" + composedFile;

				// is it a steam appid?
				if (!Q_strcmp(VarArgs("%llu", Q_atoui64(composedFile.c_str())), composedFile.c_str()))
				{
					composedFile = "steam://run/" + composedFile;
					bItemFileGood = true;
				}
				else if (composedFile.find("http") == 0)
					bItemFileGood = true;
				else if (g_pFullFileSystem->FileExists(composedFile.c_str()))
					bItemFileGood = true;
				//else if (true)	// check if local file exists // FIXME: assume it always exists for now
					//bItemFileGood = true;
			}
		}
	}

	// if the security scrubs of the item's variables referenced by the app's launch syntax failed, we ALREADY have an error.
	if (errorType != NONE)
		return errorType;

	// all variables are resolved, now do some logic.
	if (bItemGood)
	{
		// check for a good app first, because that determines if the item file can be resolved.
		if (bHasApp)
		{
			if (bAppGood)
			{
				if (bAppExecutableGood)
				{
					if (!bHasAppFilepath || bAtLeastOneAppFilepathExists)
					{
						if (bItemFileGood)
						{
							// executable
							executable = appExecutable;

							// executableDirectory
							std::string dir = appExecutable;
							size_t found = dir.find_last_of("/\\");
							if (found != std::string::npos)
								executableDirectory = dir.substr(0, found + 1);

							// masterCommands
							std::string shortAppFile = appExecutable;
							found = shortAppFile.find_last_of("/\\");
							if (found != std::string::npos)
								shortAppFile = shortAppFile.substr(found + 1);

							masterCommands = "\"" + shortAppFile + "\" " + appCommands;
							bReadyToActuallyLaunch = true;
						}
						else
						{
							DevMsg("USER-RESOLVABLE-LAUNCH-ERROR: Show it, bra.\n");
							errorType = ITEM_FILE_NOT_FOUND;
							//return;
						}
					}
					else
					{
						DevMsg("USER-RESOLVABLE-LAUNCH-ERROR: Show it, bra.\n");
						errorType = APP_PATH_NOT_FOUND;
						//return;
					}
				}
				else
				{
					DevMsg("USER-RESOLVABLE-LAUNCH-ERROR: Show it, bra.\n");
					errorType = APP_FILE_NOT_FOUND;
					//return;
				}
			}
			else
			{
				DevMsg("USER-RESOLVABLE-LAUNCH-ERROR: Show it, bra.\n");
				errorType = APP_NOT_FOUND;
				//return;
			}
		}
		else
		{
			if (bItemFileGood)
			{
				// doesn't use an app
				executable = composedFile;
				executableDirectory = "";
				masterCommands = "";
				bReadyToActuallyLaunch = true;
			}
			else
			{
				DevMsg("USER-RESOLVABLE-LAUNCH-ERROR: Show it, bra.\n");
				errorType = ITEM_FILE_NOT_FOUND;
				//return;
			}
		}
	}
	else
	{
		DevMsg("USER-RESOLVABLE-LAUNCH-ERROR: Show it, bra.\n");
		errorType = ITEM_NOT_FOUND;
		//return;
	}

	// all regular user-correctable errors would have been detected by now...
	if (errorType != NONE)
		return errorType;

	// perform security checks now that all variables are resolved
	// NOTE: masterCommands is generated from locally defined launch commands in the user's APP
	// NOTE: executableDirectory is generated locally from the user's APP.  It is the same as the front of the executable provided by the user's APP.
	// NOTE: executable potentially comes from the ITEM
	// NOTE: malicious items might try to break out of the trusted app command syntax, so strict formatting in AcradeCreateProcess is required.
	// NOTE: Untrusted item values being referenced by trusted app command syntax should be scrubbed through the alphabet & checked for breakout payloads.
	if (!this->ExecutableSafe(executable))
		errorType = UNKNOWN_ERROR;

	if (errorType == NONE && bReadyToActuallyLaunch)
	{
		if (g_pAnarchyManager->GetInputManager()->GetInputMode())
			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

		// deslect any entity
		if (g_pAnarchyManager->GetSelectedEntity())
			g_pAnarchyManager->DeselectEntity("none");

		// clear the embedded instance (to stop YT videos from playing, for example)
		/*
		C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
		C_EmbeddedInstance* pOldEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
		if (pOldEmbeddedInstance && pOldEmbeddedInstance != pHudBrowserInstance)
		{
		//g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
		//g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, null);
		//g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(pHudBrowserInstance);
		pOldEmbeddedInstance->Close();
		}
		else
		*/

		g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, null);

		// pause AArcade
		g_pAnarchyManager->Pause();

		// Write this live URL out to the save file.
		if (cvar->FindVar("broadcast_mode")->GetBool())
		{
			// Write this live URL out to the save file.
			std::string XSPlitLiveFolder = cvar->FindVar("broadcast_folder")->GetString();
			if (XSPlitLiveFolder != "")
			{
				if (cvar->FindVar("broadcast_auto_game")->GetBool())
					this->WriteBroadcastGame(std::string(itemActive->GetString("title")));

				// Also update a JS file
				FileHandle_t hFile = g_pFullFileSystem->Open(VarArgs("%s\\vote.js", XSPlitLiveFolder.c_str()), "a+", "");
				if (hFile)
				{
					std::string code = "gAnarchyTV.OnAArcadeCommand(\"startPlaying\", \"";
					code += itemActive->GetString("info/id");
					code += "\");\n";
					g_pFullFileSystem->Write(code.c_str(), code.length(), hFile);
					g_pFullFileSystem->Close(hFile);
				}
			}
		}

		// launch the item
		g_pAnarchyManager->ArcadeCreateProcess(executable, executableDirectory, masterCommands);
	}
	//else
	//	DevMsg("ERROR: Could not launch item.\n");

	return errorType;
}

void C_AnarchyManager::BeginImportSteamGames()
{
	if (!this->IsInitialized())
	{
		DevMsg("Not initialized.  Aborting Steam games import.\n");
		return;
	}

	// Scan user profile.
	// 1. Activate input mode.
	// 2. Navigate to the user's games list on their Steam profile in the in-game Steamworks browser.
	// 3. Notify & instruct the user if their profile is set to private, otherwise have an "IMPORT" button appear.
	// 4. Import all games from their list into a KeyValues file ownedGames.key
	// 5. Load all entries from ownedGames.key as items, but do not automatically save them out until the user modifies them.

	this->GetInputManager()->DeactivateInputMode(true);

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	if (g_pAnarchyManager->GetSelectedEntity())
		g_pAnarchyManager->DeselectEntity("asset://ui/overlay.html");
	else
		pHudBrowserInstance->SetUrl("asset://ui/overlay.html");

	CSteamID sid = steamapicontext->SteamUser()->GetSteamID();
	std::string profileUrl = "http://www.steamcommunity.com/profiles/" + std::string(VarArgs("%llu", sid.ConvertToUint64())) + "/games/?tab=all";

	C_SteamBrowserInstance* pSteamBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->CreateSteamBrowserInstance();
	pSteamBrowserInstance->SetActiveScraper("importSteamGames", "", "");

	std::string id = "scrape" + std::string(g_pAnarchyManager->GenerateUniqueId());

	pSteamBrowserInstance->Init(id, profileUrl, "Steam Game Importer", null);
	pSteamBrowserInstance->Focus();
	pSteamBrowserInstance->Select();
	g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(pSteamBrowserInstance);
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pSteamBrowserInstance);
}

size_t ExecuteProcess(std::string FullPathToExe, std::string Parameters)
{
	size_t iMyCounter = 0, iReturnVal = 0, iPos = 0;
	DWORD dwExitCode = 0;
	std::string sTempStr = "";

	/* - NOTE - You should check here to see if the exe even exists */

	/* Add a space to the beginning of the Parameters */
	if (Parameters.size() != 0)
	{
		if (Parameters[0] != L' ')
		{
			Parameters.insert(0, " ");
		}
	}

	/* The first parameter needs to be the exe itself */
	sTempStr = FullPathToExe;
	iPos = sTempStr.find_last_of("/\\");
	sTempStr.erase(0, iPos + 1);
	Parameters = sTempStr.append(Parameters);

	/* CreateProcessW can modify Parameters thus we allocate needed memory */
	char* pwszParam = new char[Parameters.size() + 1];
	if (pwszParam == 0)
	{
		return 1;
	}
	const char* pchrTemp = Parameters.c_str();
	Q_strcpy(pwszParam, pchrTemp);
	//wcscpy_s(pwszParam, Parameters.size() + 1, pchrTemp);

	/* CreateProcess API initialization */
	//STARTUPINFOW siStartupInfo;
	STARTUPINFO siStartupInfo;
	PROCESS_INFORMATION piProcessInfo;
	memset(&siStartupInfo, 0, sizeof(siStartupInfo));
	memset(&piProcessInfo, 0, sizeof(piProcessInfo));
	siStartupInfo.cb = sizeof(siStartupInfo);

	if (CreateProcess(FullPathToExe.c_str(),
		pwszParam, 0, 0, false,
		CREATE_DEFAULT_ERROR_MODE, 0, 0,
		&siStartupInfo, &piProcessInfo) != false)
	{
		/* Watch the process. */
		//dwExitCode = WaitForSingleObject(piProcessInfo.hProcess, (SecondsToWait * 1000));
	}
	else
	{
		/* CreateProcess failed */
		iReturnVal = GetLastError();
	}


	DevMsg("Done\n");
	/* Free memory */
	delete[]pwszParam;
	pwszParam = 0;

	/* Release handles */
	CloseHandle(piProcessInfo.hProcess);
	CloseHandle(piProcessInfo.hThread);

	return iReturnVal;
}

void C_AnarchyManager::ArcadeCreateProcess(std::string executable, std::string executableDirectory, std::string masterCommands)
{

	//DevMsg("Launching Item: \n\tExecutable: %s\n\tDirectory: %s\n\tMaster Commands: %s\n", executable.c_str(), executableDirectory.c_str(), masterCommands.c_str());
	//size_t result = ExecuteProcess(executable, masterCommands);
	//DevMsg("Finished launching item.\n");

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	// set the size of the structures
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	DevMsg("Launching Item: \n\tExecutable: %s\n\tDirectory: %s\n\tMaster Commands: %s\n", executable.c_str(), executableDirectory.c_str(), masterCommands.c_str());

	bool bIsDirectlyExecutable = false;

	/*
	bool bIsDirectlyExecutable = false;

	std::string executableExtensions = "::exe::";
	std::string fileName = executable;
	std::string fileExtension = "";
	size_t found = fileName.find_last_of("/\\");
	if ( found != std::string::npos )
	{
		fileName = fileName.substr(found + 1);

		found = fileName.find_last_of(".");
		if (found != std::string::npos && found < fileName.length() - 1)
		{
			fileExtension = fileName.substr(found + 1);
			fileExtension = "::" + fileExtension + "::";
			//fileName = fileName.substr(0, found);
			if (executableExtensions.find(fileExtension) != std::string::npos)
			{
				if (g_pFullFileSystem->FileExists(executable.c_str()))
				{
					fileName = fileName + " ";
					bIsDirectlyExecutable = true;
					Q_strcpy(pCommands, fileName.c_str());
					DevMsg("Using %s as the filename\n", fileName.c_str());
				}
			}
		}
	}

	if ( !bIsDirectlyExecutable )
		Q_strcpy(pCommands, masterCommands.c_str());
	*/

	if (!bIsDirectlyExecutable && executableDirectory == "" && masterCommands == "")
	{
		bool bUseKodi = cvar->FindVar("kodi")->GetBool();
		bool bIsKodiFileExtension = false;
		if (bUseKodi)
		{
			// Check for Kodi files
			std::vector<std::string> kodiFileExtensions;
			kodiFileExtensions.push_back(".avi");
			kodiFileExtensions.push_back(".mpg");
			kodiFileExtensions.push_back(".mp4");
			kodiFileExtensions.push_back(".mpeg");
			kodiFileExtensions.push_back(".vob");
			kodiFileExtensions.push_back(".mkv");

			bIsKodiFileExtension = false;
			unsigned int length = executable.length();
			unsigned int max = kodiFileExtensions.size();
			for (unsigned int i = 0; i < max; i++)
			{
				if (executable.find(kodiFileExtensions[i]) == length - kodiFileExtensions[i].length())
				{
					bIsKodiFileExtension = true;
					break;
				}
			}
		}

		if (bUseKodi && bIsKodiFileExtension)
		{
			DevMsg("Launch option A\n");
			std::string bufLocationString = executable;

			size_t found = bufLocationString.find("\\");
			while (found != std::string::npos)
			{
				bufLocationString[found] = '/';
				found = bufLocationString.find("\\");
			}

			std::string kodiIP = cvar->FindVar("kodi_ip")->GetString();
			std::string kodiPort = cvar->FindVar("kodi_port")->GetString();
			std::string kodiUser = cvar->FindVar("kodi_user")->GetString();
			std::string kodiPassword = cvar->FindVar("kodi_password")->GetString();

			if (kodiIP == "")
				DevMsg("ERROR: You need to go into options and set your Kodi IP first.\n");
			else if (kodiPort == "")
				DevMsg("ERROR: You need to go into options and set your Kodi PORT first.\n");
			else if (kodiUser == "")
				DevMsg("ERROR: You need to go into options and set your Kodi USER first.\n");
			else if (kodiPassword == "")
				DevMsg("ERROR: You need to go into options and set your Kodi PASSWORD first.\n");
			else
			{
				std::string kodiGetRequest = kodiUser + std::string(":") + kodiPassword + std::string("@") + kodiIP + std::string(":") + kodiPort;
				DevMsg("Thing is: %s\n", kodiGetRequest.c_str());
				C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
				std::vector<std::string> params;
				params.push_back(kodiGetRequest);
				params.push_back(executable);
				pHudBrowserInstance->DispatchJavaScriptMethod("kodiListener", "play", params);
			}
		}
		else
		{
			DevMsg("Launch option B: %s\n", executable.c_str());
			//g_pVGuiSystem->ShellExecuteA("open", executable.c_str());

			// NOW DO THE ACTUAL LAUNCHING STUFF
			// old-style bat launching
			FileHandle_t launch_file = filesystem->Open("Arcade_Launcher.bat", "w", "EXECUTABLE_PATH");

			if (!launch_file)
			{
				Msg("Error creating ArcadeLauncher.bat!\n");
				return;
			}

			bool bCommandIsURL = false;

			bool DoNotPause = false;
			std::string goodExecutable = "\"" + executable + "\"";
			filesystem->FPrintf(launch_file, "%s:\n", goodExecutable.substr(1, 1).c_str());
			filesystem->FPrintf(launch_file, "cd \"%s\"\n", goodExecutable.substr(1, goodExecutable.find_last_of("/\\", goodExecutable.find("\"", 1)) - 1).c_str());
			filesystem->FPrintf(launch_file, "START \"Launching item...\" %s", goodExecutable.c_str());
			//filesystem->FPrintf(launch_file, "START \"Launching item...\" %s", masterCommands.c_str());
			filesystem->Close(launch_file);
			system("Arcade_Launcher.bat");
		}
	}
	else
	{
		DevMsg("Launch option C\n");

		char pCommands[AA_MAX_STRING];
		Q_strcpy(pCommands, masterCommands.c_str());

		// start the program up
		CreateProcess(executable.c_str(),   // the path
			pCommands,        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,//CREATE_DEFAULT_ERROR_MODE,              //0 // No creation flags
			NULL,           // Use parent's environment block
			executableDirectory.c_str(),           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi);           // Pointer to PROCESS_INFORMATION structure
	}
	DevMsg("Finished launching item.\n");
}

void C_AnarchyManager::RunAArcade()
{
	if (!this->IsInitialized())
	{
		C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
		pHudBrowserInstance->SetUrl("asset://ui/startup.html");
	}
	else
	{
		C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
		pHudBrowserInstance->SetUrl("asset://ui/welcome.html");
		this->GetInputManager()->ActivateInputMode(true, true, pHudBrowserInstance);

		m_bSuspendEmbedded = false;
	}
}

void C_AnarchyManager::HudStateNotify()
{
	DevMsg("HudStateNotify\n");
	std::vector<std::string> params;

	// isFullscreen
	params.push_back(VarArgs("%i", (g_pAnarchyManager->GetInputManager()->GetFullscreenMode())));

	// isHudPinned
	params.push_back(VarArgs("%i", (g_pAnarchyManager->GetInputManager()->GetWasForceInputMode())));

	// isMapLoaded
	params.push_back(VarArgs("%i", engine->IsInGame()));

	// isObjectSelected (any object)
	params.push_back(VarArgs("%i", (g_pAnarchyManager->GetSelectedEntity() != null)));

	// isItemSelected (any item)
	int entIndex = -1;
	int isItemSelected = 0;
	C_PropShortcutEntity* pShortcut = null;
	C_BaseEntity* pEntity = g_pAnarchyManager->GetSelectedEntity();
	if (pEntity)
	{
		pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);
		if (pShortcut && pShortcut->GetItemId() != "")
			isItemSelected = 1;
	}
	params.push_back(VarArgs("%i", isItemSelected));

	// isMainMenu
	params.push_back(VarArgs("%i", (g_pAnarchyManager->GetInputManager()->GetMainMenuMode())));

	// url
	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
	if (pEmbeddedInstance && pEmbeddedInstance != pHudBrowserInstance)
	{
		params.push_back(pEmbeddedInstance->GetURL());

		if (pShortcut && pEmbeddedInstance->GetOriginalEntIndex() == pShortcut->entindex())
			entIndex = pShortcut->entindex();
	}
	else
		params.push_back("");

	// isSelectedObject
	if (pShortcut && pEmbeddedInstance && pEmbeddedInstance->GetOriginalEntIndex() == pShortcut->entindex())
		params.push_back("1");
	else
		params.push_back("0");

	// embeddedInstanceType
	std::string embeddedType = "Unknown";
	if (pEmbeddedInstance)
	{
		C_LibretroInstance* pLibretroInstance = dynamic_cast<C_LibretroInstance*>(pEmbeddedInstance);
		if (pLibretroInstance)
			embeddedType = "Libretro";

		C_SteamBrowserInstance* pSteamBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(pEmbeddedInstance);
		if (pSteamBrowserInstance)
			embeddedType = "SteamworksBrowser";

		C_AwesomiumBrowserInstance* pAwesomiumBrowserInstance = dynamic_cast<C_AwesomiumBrowserInstance*>(pEmbeddedInstance);
		if (pAwesomiumBrowserInstance)
			embeddedType = "AwesomiumBrowser";
	}

	params.push_back(embeddedType);
	
	pHudBrowserInstance->DispatchJavaScriptMethod("arcadeHud", "onActivateInputMode", params);
}

void C_AnarchyManager::ShowTaskMenu()
{
	if (g_pAnarchyManager->GetSelectedEntity())
		g_pAnarchyManager->TaskRemember();
	//g_pAnarchyManager->DeselectEntity();

	//if (!enginevgui->IsGameUIVisible())
	//{
		C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
		//pHudBrowserInstance->SetUrl(VarArgs("asset://ui/%s", this->GetTabMenuFile().c_str()));//taskMenu.html");
		pHudBrowserInstance->SetUrl("asset://ui/welcome.html?tab=tasks");
		m_pInputManager->ActivateInputMode(true, true);
	//}
}

void C_AnarchyManager::TeleportToScreenshot(std::string id, bool bDeactivateInputMode)
{
	KeyValues* pScreenshotKV = g_pAnarchyManager->GetMetaverseManager()->GetScreenshot(id);
	if (pScreenshotKV)
	{
		if (bDeactivateInputMode)
			this->GetInputManager()->DeactivateInputMode(true);

		C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
		engine->ServerCmd(VarArgs("teleport_player %i %s %s\n", pPlayer->entindex(), pScreenshotKV->GetString("body/position", "0 0 0"), pScreenshotKV->GetString("body/rotation", "0 0 0")), true);
	}
}

bool C_AnarchyManager::TakeScreenshot(bool bCreateBig, std::string id)
{
	//int thumbWidth = 512;
	//int thumbHeight = 512;
	int thumbWidth = 240;
	int thumbHeight = 135;
	int bigWidth = 1920;
	int bigHeight = 1080;

	std::string mapShotsFolder = "shots";
	g_pFullFileSystem->CreateDirHierarchy(mapShotsFolder.c_str(), "DEFAULT_WRITE_PATH");

	std::string mapName = g_pAnarchyManager->MapName();
	if (mapName != "")
	{
		std::string goodId = id;
		std::string mapShotFile;
		std::string mapShotBigFile;
		std::string mapShotInfoFile;
		std::string relativeMapShotFile;
		std::string relativeMapShotBigFile;
		std::string relativeMapShotInfoFile;

		if (goodId == "")
		{
			// find a filename for this new screenshot
			unsigned int screenshotNumber = 0;

			goodId = mapName + std::string(VarArgs("%i", screenshotNumber));

			mapShotFile = goodId + ".tga";
			mapShotBigFile = goodId + "_big.tga";
			mapShotInfoFile = goodId + ".txt";

			relativeMapShotFile = mapShotsFolder + "\\" + mapShotFile;
			relativeMapShotBigFile = mapShotsFolder + "\\" + mapShotBigFile;
			relativeMapShotInfoFile = mapShotsFolder + "\\" + mapShotInfoFile;

			while (g_pFullFileSystem->FileExists(relativeMapShotFile.c_str(), "DEFAULT_WRITE_PATH") || g_pFullFileSystem->FileExists(relativeMapShotBigFile.c_str(), "DEFAULT_WRITE_PATH") || g_pFullFileSystem->FileExists(relativeMapShotInfoFile.c_str(), "DEFAULT_WRITE_PATH"))
			{
				screenshotNumber++;

				goodId = mapName + std::string(VarArgs("%i", screenshotNumber));

				mapShotFile = goodId + ".tga";
				relativeMapShotFile = mapShotsFolder + "\\" + mapShotFile;

				mapShotBigFile = goodId + "_big.tga";
				relativeMapShotBigFile = mapShotsFolder + "\\" + mapShotBigFile;

				mapShotInfoFile = goodId + ".txt";
				relativeMapShotInfoFile = mapShotsFolder + "\\" + mapShotInfoFile;
			}
		}
		else
		{
			mapShotFile = goodId + ".tga";
			mapShotBigFile = goodId + "_big.tga";
			mapShotInfoFile = goodId + ".txt";

			relativeMapShotFile = mapShotsFolder + "\\" + mapShotFile;
			relativeMapShotBigFile = mapShotsFolder + "\\" + mapShotBigFile;
			relativeMapShotInfoFile = mapShotsFolder + "\\" + mapShotInfoFile;

			// if files already exist for this screenshot id, remote them
			if (g_pFullFileSystem->FileExists(relativeMapShotFile.c_str(), "DEFAULT_WRITE_PATH"))
				g_pFullFileSystem->RemoveFile(relativeMapShotFile.c_str(), "DEFAULT_WRITE_PATH");

			if (g_pFullFileSystem->FileExists(relativeMapShotBigFile.c_str(), "DEFAULT_WRITE_PATH"))
				g_pFullFileSystem->RemoveFile(relativeMapShotBigFile.c_str(), "DEFAULT_WRITE_PATH");

			if (g_pFullFileSystem->FileExists(relativeMapShotInfoFile.c_str(), "DEFAULT_WRITE_PATH"))
				g_pFullFileSystem->RemoveFile(relativeMapShotInfoFile.c_str(), "DEFAULT_WRITE_PATH");
		}

		// create the info file
		C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();

		bool bNeedsAdd = false;
		KeyValues* pInfoKV = this->GetMetaverseManager()->GetScreenshot(goodId);
		if (!pInfoKV)
		{
			pInfoKV = new KeyValues("screenshot");
			bNeedsAdd = true;
		}

		uint64 timeNumber = g_pAnarchyManager->GetTimeNumber();
		pInfoKV->SetString("created", VarArgs("%llu", timeNumber));
		pInfoKV->SetString("id", goodId.c_str());

		// FIXME: This helper function should be generalized because it is also used in awesomiumjshandlers.cpp in the "getWorldInfo" handler
		instance_t* instance = g_pAnarchyManager->GetInstanceManager()->GetInstance(g_pAnarchyManager->GetInstanceId());
		pInfoKV->SetString("instance/id", instance->id.c_str());
		pInfoKV->SetString("instance/mapId", instance->mapId.c_str());
		pInfoKV->SetString("instance/title", instance->title.c_str());
		pInfoKV->SetString("instance/file", instance->file.c_str());
		pInfoKV->SetString("instance/workshopIds", instance->workshopIds.c_str());
		pInfoKV->SetString("instance/mountIds", instance->mountIds.c_str());

		KeyValues* pMapKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetMap(instance->mapId));
		KeyValues* dummyKV = pInfoKV->FindKey("map", true);
		this->AddSubKeysToKeys(pMapKV, dummyKV);

		Vector origin = C_BasePlayer::GetLocalPlayer()->EyePosition();
		QAngle angles = C_BasePlayer::GetLocalPlayer()->EyeAngles();

		char buf[AA_MAX_STRING];
		Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", origin.x, origin.y, origin.z);
		pInfoKV->SetString("camera/position", buf);

		Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", angles.x, angles.y, angles.z);
		pInfoKV->SetString("camera/rotation", buf);

		Vector bodyOrigin = pPlayer->GetAbsOrigin();
		QAngle bodyAngles = pPlayer->GetAbsAngles();

		Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", bodyOrigin.x, bodyOrigin.y, bodyOrigin.z);
		pInfoKV->SetString("body/position", buf);

		Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", bodyAngles.x, bodyAngles.y, bodyAngles.z);
		pInfoKV->SetString("body/rotation", buf);

		if (pInfoKV->SaveToFile(g_pFullFileSystem, relativeMapShotInfoFile.c_str(), "DEFAULT_WRITE_PATH"))
		{
			clientdll->WriteSaveGameScreenshotOfSize(relativeMapShotFile.c_str(), thumbWidth, thumbHeight, false, false);
			if (bCreateBig)
				clientdll->WriteSaveGameScreenshotOfSize(relativeMapShotBigFile.c_str(), bigWidth, bigHeight, false, false);

			if (bNeedsAdd)
				this->GetMetaverseManager()->AddScreenshot(pInfoKV);

			Msg("Shot saved.\n");
			return true;
		}
		//pInfoKV->deleteThis();	// don't delete us cuz we are already auto-loaded by the metaverse manager now!
	}

	return false;
}

void C_AnarchyManager::HideScreenshotMenu()
{
	m_pInputManager->DeactivateInputMode(true);
}

void C_AnarchyManager::ShowScreenshotMenu()
{
	if (g_pAnarchyManager->GetSelectedEntity())
		g_pAnarchyManager->TaskRemember();

	C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
	pHudBrowserInstance->SetUrl("asset://ui/screenshotMenu.html");
	m_pInputManager->ActivateInputMode(true, true);
}

void C_AnarchyManager::HideTaskMenu()
{
	m_pWindowManager->DoPendingSwitch();	// in case the user clicked on SwitchToWindowInstance while looking at the task manager, we must wait until he releases the menu button.
	m_pInputManager->DeactivateInputMode(true);
}

void C_AnarchyManager::ObsoleteLegacyCommandReceived()
{
	DevMsg("Obsolete LEGACY command detected! Attempting to correct your keybinds for REDUX...\n");

	if (g_pFullFileSystem->FileExists("config/config.cfg", "DEFAULT_WRITE_PATH"))
	{
		// make a uniquely named config_redux.cfg to make sure we exec the right one
		CUtlBuffer buf;
		if (filesystem->ReadFile("config/config.cfg", "DEFAULT_WRITE_PATH", buf))
		{
			filesystem->WriteFile("config/config_redux.cfg", "DEFAULT_WRITE_PATH", buf);
			engine->ClientCmd("exec config_redux\n");
		}
		buf.Purge();
	}
	else
	{
		engine->ClientCmd("exec config_default_redux\n");
	}
}

void C_AnarchyManager::StartHoldingPrimaryFire()
{
	if (m_bIsHoldingPrimaryFire)
		return;

	m_bIsHoldingPrimaryFire = true;

	C_AwesomiumBrowserInstance* pHudInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	//pHudInstance->SetUrl("asset://ui/primaryMenu.html");
	pHudInstance->SetUrl("asset://ui/buildMode.html?mode=select");
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, false, null, false);
}

void C_AnarchyManager::StopHoldingPrimaryFire()
{
	if (!m_bIsHoldingPrimaryFire)
		return;
	
	m_bIsHoldingPrimaryFire = false;

	C_BaseEntity* pGlowEntity = m_pHoverGlowEntity;
	g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
	if (pGlowEntity)
	{
		//if (m_pSelectedEntity)	// NOTE: Nothing should be selected at this point, because a fullscreen HUD menu is what initiates this method.
//			g_pAnarchyManager->DeselectEntity();

		//g_pAnarchyManager->AttemptSelectEntity(pGlowEntity);	// Instead of selecting the entity, we're making the build mode context appear instead.
		//if (m_pSelectedEntity == pGlowEntity)
		//{
			/* have the player look at the entity, if possible.
			C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
			Vector lookPos = pGlowEntity->GetAbsOrigin();
			pPlayer->Eye
			//lookPos.
			*/
		//}

		//if (g_pAnarchyManager->GetSelectedEntity())
			//g_pAnarchyManager->DeselectEntity("asset://ui/buildModeContext.html");
		//else
		C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
		pHudBrowserInstance->SetUrl(VarArgs("asset://ui/buildModeContext.html?entity=%i", pGlowEntity->entindex()));

		g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pHudBrowserInstance);
	}
}

// IMPORTANT NOTE: Key values loaded with LoadFromFile forget the data types for their fields and if a string is just numbers, it gets turned into a number instead of a string after loading.
// This matters if the number started with a 0, because leading zeros get removed for numbers.
// So to catch this, additional checks must be performed on ID's read from KeyValues files.
bool C_AnarchyManager::CompareLoadedFromKeyValuesFileId(const char* testId, const char* baseId)
{
	int intBaseId = Q_atoi(baseId);
	if (!Q_stricmp(testId, baseId) || (intBaseId != 0 && Q_atoi(testId) == intBaseId))//(Q_strcmp(VarArgs("%i", intBaseId), baseId) && intBaseId == intBaseId == Q_atoi(testId)))
		return true;
	else
		return false;
}

void C_AnarchyManager::Feedback(std::string type)
{
	std::string discussionsUrl = "http://steamcommunity.com/app/266430/discussions/5/";
	std::string suggestionsUrl = "http://steamcommunity.com/app/266430/discussions/6/";
	std::string bugsUrl = "http://steamcommunity.com/app/266430/discussions/4/";
	std::string trelloUrl = "https://trello.com/b/PLcyQaio";

	std::string goodUrl = "";
	if (type == "discussions")
		goodUrl = discussionsUrl;
	else if (type == "suggestions")
		goodUrl = suggestionsUrl;
	else if (type == "bugs")
		goodUrl = bugsUrl;
	else if (type == "trello")
		goodUrl = trelloUrl;

	if (goodUrl != "")
		steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage(goodUrl.c_str());
}

void C_AnarchyManager::HardPause()
{
	//m_bPausePending = true;
	engine->ClientCmd("stopsound");
	materials->ReleaseResources();
	materials->Flush(true);
	materials->EvictManagedResources();
}

void C_AnarchyManager::WakeUp()
{
	materials->ReacquireResources();
}

void C_AnarchyManager::TaskClear()
{
	if (this->GetSelectedEntity())
		this->DeselectEntity();

	m_pCanvasManager->CloseAllInstances();
}

void C_AnarchyManager::TaskRemember()
{
	C_BaseEntity* pEntity = g_pAnarchyManager->GetSelectedEntity();
	if (pEntity)
	{
		C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);

		std::vector<C_EmbeddedInstance*> embeddedInstances;
		pShortcut->GetEmbeddedInstances(embeddedInstances);

		C_EmbeddedInstance* pEmbeddedInstance;
		C_EmbeddedInstance* testerInstance;
		unsigned int i;
		unsigned int size = embeddedInstances.size();
		for (i = 0; i < size; i++)
		{
			pEmbeddedInstance = embeddedInstances[i];
			if (pEmbeddedInstance->GetId() == "images")
			{
				testerInstance = g_pAnarchyManager->GetCanvasManager()->FindEmbeddedInstance("auto" + pShortcut->GetItemId());
				if (testerInstance && testerInstance->GetTexture())
				{
					//g_pAnarchyManager->GetCanvasManager()->SetDisplayInstance(testerInstance);
					g_pAnarchyManager->DeselectEntity("", false);
					break; // only put the 1st embedded instance on continous play
				}
			}
		}
	}
}

void C_AnarchyManager::SetSlaveScreen(bool bVal)
{
	C_PropShortcutEntity* pShortcut = null;
	C_BaseEntity* pEntity = this->GetSelectedEntity();
	if (pEntity)
		pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);

	if (pShortcut)
	{
		object_t* pObject = this->GetInstanceManager()->GetInstanceObject(pShortcut->GetObjectId());
		if (pObject)
		{
			//pObject->slave = bVal;
			pShortcut->SetSlave(bVal);
			this->GetInstanceManager()->ApplyChanges(pShortcut);	// will also update the object
		}
	}
}

void C_AnarchyManager::PostRender()
{
//	SteamAPI_RunCallbacks();
	//DevMsg("AnarchyManager: PostRender\n");

	if (engine->IsInGame() && m_pCanvasManager)
		m_pCanvasManager->CleanupTextures();
}

// Over 15x faster than: (int)floor(value)
/*
inline int Floor2Int(float a)
{
	int RetVal;
#if defined( __i386__ )
	// Convert to int and back, compare, subtract one if too big
	__m128 a128 = _mm_set_ss(a);
	RetVal = _mm_cvtss_si32(a128);
	__m128 rounded128 = _mm_cvt_si2ss(_mm_setzero_ps(), RetVal);
	RetVal -= _mm_comigt_ss(rounded128, a128);
#else
	RetVal = static_cast<int>(floor(a));
#endif
	return RetVal;
}
*/
#include <chrono>
void C_AnarchyManager::GenerateUniqueId(char* result)
{
	std::string PUSH_CHARS = "-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";

	double now = std::chrono::system_clock::now().time_since_epoch().count();
	now = floor(now / 64.0);
	now = floor(now / 64.0);

	while (now <= m_dLastGenerateIdTime)
		now++;

	bool duplicateTime = (now == m_dLastGenerateIdTime);
	m_dLastGenerateIdTime = now;

	//char* timeStampChars[8];
	std::string timeStampChars = "00000000";
	for (unsigned int i = 8; i > 0; i--)
	{
		timeStampChars.replace(i - 1, 1, 1, PUSH_CHARS.at(fmod(now, 64.0)));
		now = floor(now / 64.0);
	}

	if (now != 0)
	{
		DevMsg("ERROR: We should have converted the entire timestamp. %f\n", now);
	}

	std::string id = timeStampChars;
	if (!duplicateTime)
	{
		for (unsigned int i = 0; i < 12; i++)
			m_lastGeneratedChars.replace(i, 1, 1, (char)floor(random->RandomFloat() * 64.0L));
	}
	else
	{
		// If the timestamp hasn't changed since last push, use the same random number, except incremented by 1.
		unsigned int i;
		for (i = 11; i >= 0 && m_lastGeneratedChars.at(i) == 63; i--)
			m_lastGeneratedChars.replace(i, 1, 1, (char)0);

		m_lastGeneratedChars.replace(i, 1, 1, (char)(m_lastGeneratedChars.at(i) + 1));
	}

	for (unsigned int i = 0; i < 12; i++)
	{
		id += PUSH_CHARS.at(m_lastGeneratedChars.at(i));
	}

	if (id.length() != 20)
		DevMsg("ERROR: Lngth should be 20.\n");

	Q_strcpy(result, id.c_str());

	//return VarArgs("%s", id.c_str());	// works on instance menu	// GETS ALL CALLS DURING SAME TICK
	return;// id.c_str();	// works on library browser menu	// GETS CALLS ON DIFFERENT TICKS
}

const char* C_AnarchyManager::GenerateUniqueId()
{
	std::string PUSH_CHARS = "-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";

	double now = std::chrono::system_clock::now().time_since_epoch().count();
	now = floor(now / 64.0);
	now = floor(now / 64.0);

	while (now <= m_dLastGenerateIdTime)
		now++;

	bool duplicateTime = (now == m_dLastGenerateIdTime);
	m_dLastGenerateIdTime = now;

	std::string timeStampChars = "00000000";
	for (unsigned int i = 8; i > 0; i--)
	{
		timeStampChars.replace(i-1, 1, 1, PUSH_CHARS.at(fmod(now, 64.0)));
		now = floor(now / 64.0);
	}

	if (now != 0)
	{
		DevMsg("ERROR: We should have converted the entire timestamp. %f\n", now);
	}

	std::string id = timeStampChars;
	if (!duplicateTime)
	{
		for (unsigned int i = 0; i < 12; i++)
				m_lastGeneratedChars.replace(i, 1, 1, (char)floor(random->RandomFloat() * 64.0L));
	}
	else
	{
		// If the timestamp hasn't changed since last push, use the same random number, except incremented by 1.
		unsigned int i;
		for (i = 11; i >= 0 && m_lastGeneratedChars.at(i) == 63; i--)
			m_lastGeneratedChars.replace(i, 1, 1, (char)0);

		m_lastGeneratedChars.replace(i, 1, 1, (char)(m_lastGeneratedChars.at(i) + 1));
	}

	for (unsigned int i = 0; i < 12; i++)
	{
		id += PUSH_CHARS.at(m_lastGeneratedChars.at(i));
	}

	if (id.length() != 20)
		DevMsg("ERROR: Lngth should be 20.\n");

	return VarArgs("%s", id.c_str());	// works on instance menu	// GETS ALL CALLS DURING SAME TICK
	//return id.c_str();	// works on library browser menu	// GETS CALLS ON DIFFERENT TICKS
}

const char* C_AnarchyManager::GenerateUniqueId2()
{
	std::string PUSH_CHARS = "-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";

	double now = std::chrono::system_clock::now().time_since_epoch().count();
	now = floor(now / 64.0);
	now = floor(now / 64.0);

	while (now <= m_dLastGenerateIdTime)
		now++;

	bool duplicateTime = (now == m_dLastGenerateIdTime);
	m_dLastGenerateIdTime = now;

	std::string timeStampChars = "00000000";
	for (unsigned int i = 8; i > 0; i--)
	{
		timeStampChars.replace(i - 1, 1, 1, PUSH_CHARS.at(fmod(now, 64.0)));
		now = floor(now / 64.0);
	}

	if (now != 0)
	{
		DevMsg("ERROR: We should have converted the entire timestamp. %f\n", now);
	}

	std::string id = timeStampChars;
	if (!duplicateTime)
	{
		for (unsigned int i = 0; i < 12; i++)
			m_lastGeneratedChars.replace(i, 1, 1, (char)floor(random->RandomFloat() * 64.0L));
	}
	else
	{
		// If the timestamp hasn't changed since last push, use the same random number, except incremented by 1.
		unsigned int i;
		for (i = 11; i >= 0 && m_lastGeneratedChars.at(i) == 63; i--)
			m_lastGeneratedChars.replace(i, 1, 1, (char)0);

		m_lastGeneratedChars.replace(i, 1, 1, (char)(m_lastGeneratedChars.at(i) + 1));
	}

	for (unsigned int i = 0; i < 12; i++)
	{
		id += PUSH_CHARS.at(m_lastGeneratedChars.at(i));
	}

	if (id.length() != 20)
		DevMsg("ERROR: Lngth should be 20.\n");

	//return VarArgs("%s", id.c_str());	// works on instance menu	// GETS ALL CALLS DURING SAME TICK
	return id.c_str();	// works on library browser menu	// GETS CALLS ON DIFFERENT TICKS
}

std::string C_AnarchyManager::ExtractLegacyId(std::string itemFile, KeyValues* item)
{
	std::string alphabet = "0123456789abcdef";

	std::string nameSnip = "";
	bool bPassed = true;

	size_t found = itemFile.find(":");
	if (found != std::string::npos)
		bPassed = false;

	if (bPassed)
	{
		found = itemFile.find_last_of("/\\");
		if (found == std::string::npos)
			bPassed = false;
	}

	if (bPassed)
	{
		nameSnip = itemFile.substr(found + 1);
		found = nameSnip.find_first_of(".");

		if (found == std::string::npos)
			bPassed = false;
		else
		{
			nameSnip = nameSnip.substr(0, found);

			unsigned int nameSnipLength = nameSnip.length();
			if (nameSnipLength != 8)
				bPassed = false;
			else
			{
				unsigned int i;
				for (i = 0; i < nameSnipLength; i++)
				{
					found = alphabet.find(nameSnip[i]);
					if (found == std::string::npos)
					{
						bPassed = false;
						break;
					}
				}
			}
		}
	}
	
	//// generate a legacy ID based on the filelocation if given an item to work with
//	if (!bPassed && item)
	//	nameSnip = this->GenerateLegacyHash(item->GetString("filelocation"));

	if (!bPassed)
		nameSnip = "";

	return nameSnip;
}

const char* C_AnarchyManager::GenerateCRC32Hash(const char* text)
{
	char protectedHash[9];

	CRC32_t protectedCRC = CRC32_ProcessSingleBuffer((void*)text, strlen(text));
	Q_snprintf(protectedHash, 9, "%08x", protectedCRC);

	return VarArgs("%s", protectedHash);
}

const char* C_AnarchyManager::GenerateLegacyHash(const char* text)
{
	char input[AA_MAX_STRING];
	Q_strcpy(input, text);

	// Convert it to lowercase & change all slashes to back-slashes
	V_FixSlashes(input);
	for (int i = 0; input[i] != '\0'; i++)
		input[i] = tolower(input[i]);

	char lower[AA_MAX_STRING];
	unsigned m_crc = 0xffffffff;

	int inputLength = strlen(input);
	for (int i = 0; i < inputLength; i++)
	{
		lower[i] = tolower(input[i]);
	}

	for (int i = 0; i < inputLength; i++)
	{
		unsigned c = lower[i];
		m_crc ^= (c << 24);

		for (int j = 0; j < 8; j++)
		{
			const unsigned FLAG = 0x80000000;
			if ((m_crc & FLAG) == FLAG)
			{
				m_crc = (m_crc << 1) ^ 0x04C11DB7;
			}
			else
			{
				m_crc <<= 1;
			}
		}
	}

	return VarArgs("%08x", m_crc);
}

/*
var PUSH_CHARS = '-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz';

var now = new Date().getTime();
var duplicateTime = (now === this.lastPushTime);
this.lastPushTime = now;

var timeStampChars = new Array(8);
for (var i = 7; i >= 0; i--) {
timeStampChars[i] = PUSH_CHARS.charAt(now % 64);
// NOTE: Can't use << here because javascript will convert to int and lose the upper bits.
now = Math.floor(now / 64);
}
if (now !== 0) throw new Error('We should have converted the entire timestamp.');

var id = timeStampChars.join('');

if (!duplicateTime) {
for (i = 0; i < 12; i++) {
this.lastRandChars[i] = Math.floor(Math.random() * 64);
}
} else {
// If the timestamp hasn't changed since last push, use the same random number, except incremented by 1.
for (i = 11; i >= 0 && this.lastRandChars[i] === 63; i--) {
this.lastRandChars[i] = 0;
}
this.lastRandChars[i]++;
}
for (i = 0; i < 12; i++) {
id += PUSH_CHARS.charAt(this.lastRandChars[i]);
}
if(id.length != 20) throw new Error('Length should be 20.');

return id;
*/

void C_AnarchyManager::Disconnect()
{
	// called when a player clicks the LEAVE button on the main menu.  However, there are other ways they could disconnect w/o clicking that.
	m_bIsDisconnecting = true;
	engine->ClientCmd("disconnect;\n");


	//if (m_bIsDisconnecting)//&& !Q_strcmp(this->MapName(), "")
	//{
		//m_bIsDisconnecting = false;
		//this->RunAArcade();
		//this->HandleUiToggle();
	//}
}

void C_AnarchyManager::AnarchyStartup()
{
	DevMsg("AnarchyManager: AnarchyStartup\n");
	m_bIncrementState = true;

	m_pWeaponsEnabledConVar = cvar->FindVar("r_drawviewmodel");

	ConVar* pConVar = cvar->FindVar("engine_no_focus_sleep");
	m_oldEngineNoFocusSleep = pConVar->GetString();
	pConVar->SetValue(0);

	//m_state = AASTATE_INPUTMANAGER;
	/*
	m_pInstanceManager = new C_InstanceManager();
	m_pMetaverseManager = new C_MetaverseManager();
	m_pInputManager = new C_InputManager();
	m_pWebManager = new C_WebManager();
	m_pWebManager->Init();
	*/
}

/*
void C_AnarchyManager::OnWebManagerReady()
{
	C_WebTab* pHudWebTab = m_pWebManager->GetHudWebTab();
	g_pAnarchyManager->GetWebManager()->SelectWebTab(pHudWebTab);
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true);

	unsigned int uCount;
	std::string num;
	
	// And continue starting up
	uCount = m_pMetaverseManager->LoadAllLocalTypes();
	num = VarArgs("%u", uCount);
	pHudWebTab->AddHudLoadingMessage("progress", "", "Loading Types", "locallibrarytypes", "0", num, num);

	 //= m_pMetaverseManager->LoadAllLocalTypes();
	//std::string num = VarArgs("%u", uItemCount);
//	pHudWebTab->AddHudLoadingMessage("progress", "", "Loading Types", "locallibrarytypes", "0", num, num);

	uCount = m_pMetaverseManager->LoadAllLocalModels();
	num = VarArgs("%u", uCount);
	pHudWebTab->AddHudLoadingMessage("progress", "", "Loading Models", "locallibrarymodels", "0", num, num);
	
	//uItemCount = m_pMetaverseManager->LoadAllLocalApps();

	// load ALL local apps
	KeyValues* app = m_pMetaverseManager->LoadFirstLocalApp("MOD");
	if (app)
		pHudWebTab->AddHudLoadingMessage("progress", "", "Loading Apps", "locallibraryapps", "", "", "+", "loadNextLocalAppCallback");
	else
		this->OnLoadAllLocalAppsComplete();

}
*/

bool C_AnarchyManager::OnSteamBrowserCallback(unsigned int unHandle)
{
	/*
	C_SteamBrowserInstance* pInstance = m_pSteamBrowserManager->FindDefunctInstance(unHandle);
	if (pInstance)
	{
		g_pAnarchyManager->GetSteamBrowserManager()->DestroyDefunctInstance(pInstance);
		return false;
	}
	*/

	return true;
}

bool C_AnarchyManager::AttemptSelectEntity(C_BaseEntity* pTargetEntity)
{
	if (!g_pAnarchyManager->IsInitialized() )
		return false;

	C_PropShortcutEntity* pShortcut = g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity();
	if (pShortcut)
	{
		// finished positioning & choosing model, ie: changes confirmed
		DevMsg("CHANGES CONFIRMED\n");
		g_pAnarchyManager->DeactivateObjectPlacementMode(true);

		m_pInstanceManager->ApplyChanges(pShortcut);

		return SelectEntity(pShortcut);
	}
	else
	{
		C_BaseEntity* pEntity = null;
		if (!pTargetEntity)
		{
			C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
			if (!pPlayer)
				return false;

			if (pPlayer->GetHealth() <= 0)
				return false;

			// fire a trace line
			trace_t tr;
			Vector forward;
			pPlayer->EyeVectors(&forward);
			UTIL_TraceLine(pPlayer->EyePosition(), pPlayer->EyePosition() + forward * MAX_COORD_RANGE, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

			if( tr.fraction != 1.0 && tr.DidHitNonWorldEntity() )
				pEntity = tr.m_pEnt;
		}
		else
			pEntity = pTargetEntity;

		// only allow prop shortcuts
		if (pEntity)
			pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);

		if (pShortcut)
		{
			if (m_pSelectedEntity && pEntity == m_pSelectedEntity)
			{
				//m_pInputManager->SetFullscreenMode(true);
				C_EmbeddedInstance* pEmbeddedInstance = m_pInputManager->GetEmbeddedInstance();
				m_pInputManager->ActivateInputMode(true, m_pInputManager->GetMainMenuMode(), pEmbeddedInstance);
			}
			else
				return SelectEntity(pEntity);
		}
		else
		{
			if (m_pSelectedEntity)
				return DeselectEntity();
			else
				return false;
		}
	}

	return false;
}

// from http://www.zedwood.com/article/cpp-urlencode-function
#include <iostream>
#include <sstream>
std::string C_AnarchyManager::encodeURIComponent(const std::string &s)
{
	static const char lookup[] = "0123456789abcdef";
	std::stringstream e;
	for (int i = 0, ix = s.length(); i<ix; i++)
	{
		const char& c = s[i];
		if ((48 <= c && c <= 57) ||//0-9
			(65 <= c && c <= 90) ||//abc...xyz
			(97 <= c && c <= 122) || //ABC...XYZ
			(c == '-' || c == '_' || c == '.' || c == '~')
			)
		{
			e << c;
		}
		else
		{
			e << '%';
			e << lookup[(c & 0xF0) >> 4];
			e << lookup[(c & 0x0F)];
		}
	}
	return e.str();
}

bool C_AnarchyManager::SelectEntity(C_BaseEntity* pEntity)
{
//	DevMsg("DISABLED FOR TESTING!\n");
//	return true;
//	/*
	if (m_pSelectedEntity)
		DeselectEntity();

//	m_pWebManager->GetHudWebTab()->SetUrl("asset://ui/blank.html");

	m_pSelectedEntity = pEntity;
	AddGlowEffect(pEntity);

	

		//pMaterials[x]->ColorModulate(255, 0, 0);
	//pMaterials[x]->GetPreviewImage

	std::string itemId;
	std::string tabTitle;
	std::string uri;
	KeyValues* item;
	KeyValues* active;
	//C_PropShortcutEntity* pShortcut;
	//C_WebTab* pWebTab;
	//C_WebTab* pSelectedWebTab;
	C_EmbeddedInstance* pEmbeddedInstance;
	C_EmbeddedInstance* pSelectedEmbeddedInstance;

	C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);
	std::vector<C_EmbeddedInstance*> embeddedInstances;
	pShortcut->GetEmbeddedInstances(embeddedInstances);

	unsigned int i;
	unsigned int size = embeddedInstances.size();
	for (i = 0; i < size; i++)
	{
		pEmbeddedInstance = embeddedInstances[i];
		if (!pEmbeddedInstance)
			continue;

		if (pEmbeddedInstance->GetId() == "hud")
			continue;

		bool bImagesAndHandled = false;
		if (pEmbeddedInstance->GetId() == "images")
		{
			pShortcut = dynamic_cast<C_PropShortcutEntity*>(m_pSelectedEntity);
			if (pShortcut)
			{
				tabTitle = "auto" + pShortcut->GetItemId();
				pEmbeddedInstance = m_pCanvasManager->FindEmbeddedInstance(tabTitle);// this->GetWebManager()->FindWebTab(tabTitle);
				if (!pEmbeddedInstance)
				{
					itemId = pShortcut->GetItemId();
					item = m_pMetaverseManager->GetLibraryItem(itemId);
					if (item)
					{
						active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(item);

						/*
						std::string uri = "asset://ui/autoInspectItem.html?id=" + encodeURIComponent(itemId) + "&screen=" + encodeURIComponent(active->GetString("screen")) + "&marquee=" + encodeURIComponent(active->GetString("marquee")) + "&preview=" + encodeURIComponent(active->GetString("preview")) + "&reference=" + encodeURIComponent(active->GetString("reference")) + "&file=" + encodeURIComponent(active->GetString("file"));
						WebURL url = WebURL(WSLit(uri.c_str()));
						*/

						//std::string dumbUrl = "http://smarcade.net/dlcv2/view_youtube.php?id=";
						//std::string dumbUrl = active->GetString("file");

						// If this is a video file, play it in libretro instead of the browser
						bool bDoAutoInspect = true;
						if (m_pLibretroManager->GetInstanceCount() == 0)
						{
							std::string coreFile = "";
							std::string file = "";

							// check for compatible files for ffmpeg
							if (coreFile == "")
							{
								std::string exts = "::mpg::mpeg::avi::mp4::mkv::";
								std::string fileExt = active->GetString("file");

								size_t found = fileExt.find_last_of(".");
								if (found != std::string::npos)
									fileExt = fileExt.substr(found + 1);

								found = exts.find("::" + fileExt + "::");
								if (found != std::string::npos )
									coreFile = "ffmpeg_libretro.dll";
							}
							/* DISABLED UNTIL EVERYTHING IS READY TO GO WITH LIBRETRO SUPPORT
							// check for compatible types for libretro cores.
							if (coreFile == "")
							{
								std::map<std::string, std::string> typeToLibretroCore;
								typeToLibretroCore["snes"] = "snes9x_libretro.dll";
								//typeToLibretroCore["nes"] = "bsnes_performance_libretro.dll";
								typeToLibretroCore["nes"] = "fceumm_libretro.dll";
								typeToLibretroCore["arcade"] = "mame_libretro.dll";
								typeToLibretroCore["gameboy advance"] = "vba_next_libretro.dll";
								typeToLibretroCore["gameboy color"] = "vba_next_libretro.dll";
								typeToLibretroCore["gameboy"] = "vba_next_libretro.dll";
								typeToLibretroCore["gba"] = "vba_next_libretro.dll";
								typeToLibretroCore["gbc"] = "vba_next_libretro.dll";
								typeToLibretroCore["gb"] = "vba_next_libretro.dll";

								KeyValues* type = m_pMetaverseManager->GetLibraryType(active->GetString("type"));
								if (type)
								{
									KeyValues* activeType = type->FindKey("current");
									if (!activeType)
										activeType = type->FindKey("local", true);

									std::string typeTitle = activeType->GetString("title");
									if (typeTitle != "")
									{
										// convert to lowercase
										int len = typeTitle.length() + 1;
										char* pTemp = (char*)stackalloc(len);
										Q_strncpy(pTemp, typeTitle.c_str(), len);
										Q_strnlwr(pTemp, len);
										typeTitle = pTemp;
										stackfree(pTemp);

										// loop through the user's types looking for literal title matches (because type ID's could be different for every user)
										auto it = typeToLibretroCore.begin();
										//size_t foundTypeTitle;
										while (it != typeToLibretroCore.end())
										{
											//foundTypeTitle = typeTitle.find(it->first);
											//if (foundTypeTitle != std::string::npos)
											if ( typeTitle == it->first )
											{
												coreFile = it->second;
												break;
											}

											it++;
										}
									}
								}
							}
							*/

							// use the core, if one is found
							if (coreFile != "" )//&& g_pFullFileSystem->FileExists(active->GetString("file")))
							{
								file = active->GetString("file");

								bool bFileIsGood = false;
								// is the path relative to the app?
								size_t found = file.find(":");
								if (found == 1 && g_pFullFileSystem->FileExists(file.c_str()))
								{
									bFileIsGood = true;
								}
								else
								{
									// does it have an app?
									if (Q_strcmp(active->GetString("app"), ""))
									{
										bool bHasApp = true;
										KeyValues* app = g_pAnarchyManager->GetMetaverseManager()->GetLibraryApp(active->GetString("app"));
										KeyValues* appActive = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(app);
										bool bHasAppFilepath = false;
										bool bAtLeastOneAppFilepathExists = false;

										// just grab the FIRST filepath for now.
										// FIXME: Need to keep searching through filepaths until the item's file is found inside of one.
										// Note: Apps are not required to have a filepath specified.
										std::string testFile;
										std::string testPath;
										KeyValues* filepaths = appActive->FindKey("filepaths", true);
										for (KeyValues *sub = filepaths->GetFirstSubKey(); sub; sub = sub->GetNextKey())
										{
											// true if even 1 filepath exists for the app, even if it is not found on the local PC.
											// (because in that case the local user probably needs to specify a correct location for it.)
											bHasAppFilepath = true;

											testPath = sub->GetString("path");

											// test if this path exists
											// FIXME: always assume it exists for now
											if (true)
											{
												bAtLeastOneAppFilepathExists = true;

												// test if the file exists inside of this filepath
												testFile = testPath + file;

												// FIXME: always assume the file exists in this path for now.
												if (true)
												{
													file = testFile;
													bFileIsGood = true;
													break;
												}
											}
										}
									}
								}

								if (bFileIsGood)
								{
									C_LibretroInstance* pLibretroInstance = m_pLibretroManager->CreateLibretroInstance();
									pLibretroInstance->Init(tabTitle, VarArgs("%s - Libretro", active->GetString("title", "Untitled")), pShortcut->entindex());
									DevMsg("Setting game to: %s\n", file.c_str());
									pLibretroInstance->SetOriginalGame(file);
									pLibretroInstance->SetOriginalItemId(itemId);
									if (!pLibretroInstance->LoadCore(coreFile))	// FIXME: elegantly revert back to autoInspect if loading the core failed!
										DevMsg("ERROR: Failed to load core: %s\n", coreFile.c_str());
									pEmbeddedInstance = pLibretroInstance;
									bDoAutoInspect = false;
								}
							}
						}

						if ( bDoAutoInspect)
						{
							std::string uri = "file://";
							uri += engine->GetGameDirectory();
							uri += "/resource/ui/html/autoInspectItem.html?id=" + encodeURIComponent(itemId) + "&title=" + encodeURIComponent(active->GetString("title")) + "&screen=" + encodeURIComponent(active->GetString("screen")) + "&marquee=" + encodeURIComponent(active->GetString("marquee")) + "&preview=" + encodeURIComponent(active->GetString("preview")) + "&reference=" + encodeURIComponent(active->GetString("reference")) + "&file=" + encodeURIComponent(active->GetString("file"));

							// FIXME: Might want to make the slashes in the game path go foward.  Also, need to allow HTTP redirection (302).
							//DevMsg("Test URI is: %s\n", uri.c_str());

							C_SteamBrowserInstance* pSteamBrowserInstance = m_pSteamBrowserManager->CreateSteamBrowserInstance();
							pSteamBrowserInstance->Init(tabTitle, uri, "Newly selected item...", null, pShortcut->entindex());
							pSteamBrowserInstance->SetOriginalItemId(itemId);	// FIXME: do we need to do this for original entindex too???
							//pSteamBrowserInstance->SetOriginalEntIndex(pShortcut->entindex());	// probably NOT needed??


							pEmbeddedInstance = pSteamBrowserInstance;
						}
					}
					else
					{
						// the item specified by the shortcut was not found
						// by doing NOTHING, it lets you select the object but not bring up any menus on it
					}
				}
			}
		}
		else
		{
			pSelectedEmbeddedInstance = m_pInputManager->GetEmbeddedInstance();// m_pWebManager->GetSelectedWebTab();
			if (pSelectedEmbeddedInstance)
			{
				pSelectedEmbeddedInstance->Deselect();
				m_pInputManager->SetEmbeddedInstance(null);
				//m_pWebManager->DeselectWebTab(pSelectedEmbeddedInstance);
			}
		}

		if (pEmbeddedInstance)
		{
			pSelectedEmbeddedInstance = m_pInputManager->GetEmbeddedInstance();
			if (pSelectedEmbeddedInstance && pSelectedEmbeddedInstance != pEmbeddedInstance)
			{
				//m_pWebManager->DeselectWebTab(pSelectedEmbeddedInstance);
				pSelectedEmbeddedInstance->Deselect();
				m_pInputManager->SetEmbeddedInstance(null);

				//m_pWebManager->SelectWebTab(pWebTab);
				pEmbeddedInstance->Select();
				m_pInputManager->SetEmbeddedInstance(pEmbeddedInstance);
			}
			else if (!pSelectedEmbeddedInstance)
			{
				//if (pEmbeddedInstance)
				//{
				pEmbeddedInstance->Select();
				m_pInputManager->SetEmbeddedInstance(pEmbeddedInstance);
				//}



			}
		}
		else
		{
			DevMsg("ERROR: No embedded instance!!\n");
			DevMsg("ID on this item is: %s", pShortcut->GetItemId().c_str());
		}
		//C_AwesomiumBrowserInstance* pHudWebTab = this->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");

		//m_pWebManager->SelectWebTab(pWebTab);

		//break;
	}

	C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
	pHudBrowserInstance->SetUrl("asset://ui/overlay.html");
	//g_pAnarchyManager->HudStateNotify();	// because input is not always request when an object is selected?
	return true;
	//*/
}

bool C_AnarchyManager::DeselectEntity(std::string nextUrl, bool bCloseInstance)
{
	C_EmbeddedInstance* pEmbeddedInstance = m_pInputManager->GetEmbeddedInstance();
	C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
	//C_WebTab* pWebTab = m_pWebManager->GetSelectedWebTab();
	if (pEmbeddedInstance && pEmbeddedInstance != pHudBrowserInstance)
	{	
		//pEmbeddedInstance->Deselect();
		//m_pInputManager->SetEmbeddedInstance(null);

		// ALWAYS close the selected web tab when de-selecting entities. (this has to be accounted for or changed when the continous play button gets re-enabled!)
		if (bCloseInstance && pEmbeddedInstance != pHudBrowserInstance)
			pEmbeddedInstance->Close();	// FIXME: (maybe) This might cause blank.html to be loaded into the HUD layer because the entity that initiated all this is still set as m_pSelectedEntity at this point...
	}

	if (nextUrl != "" && nextUrl != "none" )
	{
		if (nextUrl != "")
			pHudBrowserInstance->SetUrl(nextUrl);
		else
			pHudBrowserInstance->SetUrl("asset://ui/overlay.html");
	}

	RemoveGlowEffect(m_pSelectedEntity);
	m_pSelectedEntity = null;
	return true;
}

void C_AnarchyManager::AddHoverGlowEffect(C_BaseEntity* pEntity)
{
	g_pAnarchyManager->RemoveLastHoverGlowEffect();
	m_pHoverGlowEntity = pEntity;
	engine->ServerCmd(VarArgs("addhovergloweffect %i", pEntity->entindex()), false);
}

void C_AnarchyManager::RemoveLastHoverGlowEffect()
{
	if (m_pHoverGlowEntity)
	{
		engine->ServerCmd(VarArgs("removehovergloweffect %i", m_pHoverGlowEntity->entindex()), false);
		m_pHoverGlowEntity = null;
	}
}

void C_AnarchyManager::AddGlowEffect(C_BaseEntity* pEntity)
{
	engine->ServerCmd(VarArgs("addgloweffect %i", pEntity->entindex()), false);
}

void C_AnarchyManager::RemoveGlowEffect(C_BaseEntity* pEntity)
{
	engine->ServerCmd(VarArgs("removegloweffect %i", pEntity->entindex()), false);
}

void C_AnarchyManager::ShowFileBrowseMenu(std::string browseId)
{
	BrowseSlate->Create(enginevgui->GetPanel(PANEL_ROOT), browseId);
}

void C_AnarchyManager::OnBrowseFileSelected(std::string browseId, std::string response)
{
	// only the HUD web view brings up the file browse menu so far...
	std::vector<std::string> params;
	params.push_back(browseId);
	params.push_back(response);

	C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
	pHudBrowserInstance->DispatchJavaScriptMethod("arcadeHud", "onBrowseFileSelected", params);
}

void C_AnarchyManager::OnWorkshopManagerReady()
{
	DevMsg("C_AnarchyManager::OnWorkshopManagerReady\n");
//	DevMsg("DISABLED FOR TESTING!\n");
//	return;
	///*

	C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
	// mount ALL workshops

	if (m_pWorkshopManager->IsEnabled())
	{
		/*
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Skipping Gen 1 Legacy Workshop Subscriptions", "skiplegacyworkshops", "", "", "0");
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Workshop Models", "workshoplibrarymodels", "", "", "0");
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Workshop Items", "workshoplibraryitems", "", "", "0");
		*/
	}
	else
	{
		pHudBrowserInstance->AddHudLoadingMessage("", "", "Skipping All Workshop", "workshopskip", "", "", "");
	}

	m_pWorkshopManager->MountFirstWorkshop();
	//*/
}

void C_AnarchyManager::ScanForLegacySave(std::string path, std::string searchPath, std::string workshopIds, std::string mountIds, C_Backpack* pBackpack)
{
	// Legacy saves come from:
	//	1. Legacy Workshop Backpacks (GEN1)
	//	2. Legacy Workshop Subscriptions (GEN2)
	//	3. Mounted Legacy Folder (GEN1 & GEN2)

	// Legacy saves can be in:
	//	1. saves/maps/[MAP_NAME].[ADDON_ID].set (GEN1)
	//	2. maps/[BSP_NAME].[ADDON_ID].set (GEN2) (potentially a NODE)
	// This function gets called for both of those folder locations.

	// detect any .set files
	//std::string file;
	//KeyValues* kv = new KeyValues("instance");
	FileFindHandle_t findHandle;
	const char *pFilename = g_pFullFileSystem->FindFirstEx(VarArgs("%s*.set", path.c_str()), searchPath.c_str(), &findHandle);
	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(findHandle))
		{
			pFilename = g_pFullFileSystem->FindNext(findHandle);
			continue;
		}

		std::string instanceId = g_pAnarchyManager->GenerateLegacyHash(pFilename);
		std::string filename = pFilename;
		std::string file = path + filename;

		// Does this instance already exist?
		bool bConsumed;
		instance_t* pInstance = g_pAnarchyManager->GetInstanceManager()->GetInstance(instanceId);
		if (pInstance)
		{
			DevMsg("Skipping consumption of legacy save file because it already existed in user library: %s\n", file.c_str());
			bConsumed = false;
		}
		else
			bConsumed = g_pAnarchyManager->GetInstanceManager()->ConsumeLegacyInstance(instanceId, filename, path, searchPath, workshopIds, mountIds, pBackpack);

		pFilename = g_pFullFileSystem->FindNext(findHandle);
	}

//	kv->Clear();
	g_pFullFileSystem->FindClose(findHandle);
}

void C_AnarchyManager::ScanForLegacySaveRecursive(std::string path, std::string searchPath, std::string workshopIds, std::string mountIds, C_Backpack* pBackpack)
{
	std::string legacyPathA = path + "maps\\";
	std::string legacyPathB = path + "saves\\maps\\";
	this->ScanForLegacySave(legacyPathA, searchPath, workshopIds, mountIds, pBackpack);
	this->ScanForLegacySave(legacyPathB, searchPath, workshopIds, mountIds, pBackpack);
}

void C_AnarchyManager::ShowEngineOptionsMenu()
{
	//// FIXME: If a map is loaded, input mode can be deactivated, but if at main menu that might be weird.
	//if (engine->IsInGame())
	//	g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

	engine->ClientCmd("gamemenucommand OpenOptionsDialog\n");
}

void C_AnarchyManager::DeactivateObjectPlacementMode(bool confirm)
{
	C_PropShortcutEntity* pShortcut = m_pMetaverseManager->GetSpawningObjectEntity();
	if (pShortcut)
	{
		if (confirm)
		{
			engine->ServerCmd(VarArgs("makenonghost %i;\n", pShortcut->entindex()), false);
			/*
			pShortcut->SetRenderColorA(255);
			pShortcut->SetRenderMode(kRenderNormal);

			// make the prop solid
			pShortcut->SetSolid(SOLID_VPHYSICS);
			pShortcut->SetSize(-Vector(100, 100, 100), Vector(100, 100, 100));
			pShortcut->SetMoveType(MOVETYPE_VPHYSICS);

			if (pShortcut->CreateVPhysics())
			{
				IPhysicsObject *pPhysics = pShortcut->VPhysicsGetObject();
				if (pPhysics)
				{
					pPhysics->EnableMotion(false);
				}
			}
			*/
		}
		else
		{
			// CANCEL
			this->GetMetaverseManager()->SetSpawningObjectEntity(null);
			this->GetMetaverseManager()->SetSpawningObject(null);

			//object_t* pObject = this->GetInstanceManager()->GetInstanceObject(pShortcut->GetObjectId());
			//if ( pObject )
			//	this->GetInstanceManager()->ResetObjectChanges(pShortcut);
			//else
			//this->GetInstanceManager()->RemoveEntity(pShortcut);
			this->GetInstanceManager()->ResetObjectChanges(pShortcut);	// this will also delete the object if there's nothing to revert to!!
		}

		m_pMetaverseManager->SetSpawningObjectEntity(null);
	}

	m_pMetaverseManager->SetSpawningObject(null);

	//C_AwesomiumBrowserInstance* pHudInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
	//pHudInstance->SetUrl("asset://ui/default.html");

	m_pInputManager->DeactivateInputMode(true);
}

void C_AnarchyManager::ActivateObjectPlacementMode(C_PropShortcutEntity* pShortcut, const char* mode)
{
	m_pInstanceManager->AdjustObjectScale(pShortcut->GetModelScale());
	m_pMetaverseManager->ResetSpawningAngles();
	m_pMetaverseManager->SetSpawningRotationAxis(1);
	m_pMetaverseManager->SetSpawningObjectEntity(pShortcut);

	std::string objectId = pShortcut->GetObjectId();
	//DevMsg("At placement id is: %s\n", objectId.c_str());
	object_t* theObject = m_pInstanceManager->GetInstanceObject(objectId);
	//DevMsg("Object val: %s\n", theObject->itemId.c_str());
	m_pMetaverseManager->SetSpawningObject(theObject);

	//std::string moreParams = "";
	if (!Q_strcmp(mode, "move"))
	{
		//SetLibraryBrowserContext(std::string category, std::string id, std::string search, std::string filter)
		std::string category = "";
		KeyValues* itemKv = m_pMetaverseManager->GetLibraryItem(pShortcut->GetItemId());
		if (itemKv)
			category = "items";
		else
		{
			// no item found, probably a model
			// FIXME: This is just ASSUMING that it is a model.
			category = "models";
			//moreParams = "&title=" + 
			//KeyValues* modelKv = m_pMetaverseManager->GetLibraryModel(pShortcut->GetItemId());
			//if (modelKv)
			//	category = "models";
		}
		m_pMetaverseManager->SetLibraryBrowserContext(category, std::string("automove"), "", "");
		engine->ServerCmd(VarArgs("makeghost %i;\n", pShortcut->entindex()), false);
		/*	// do this stuff server-side now
		pShortcut->SetSolid(SOLID_NONE);
		pShortcut->SetSize(-Vector(100, 100, 100), Vector(100, 100, 100));
		//SetRenderMode(kRenderTransTexture);
		pShortcut->SetRenderMode(kRenderTransColor);
		pShortcut->SetRenderColorA(160);
		*/
	}

	C_AwesomiumBrowserInstance* pHudInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	//pHudInstance->SetUrl("asset://ui/cabinetSelect.html");
	//std::string category = this->GetMetaverseManager()->GetLibraryBrowserContext("category");
	//pHudInstance->SetUrl(VarArgs("asset://ui/buildMode.html?mode=spawn&category=%s", category.c_str()));
	pHudInstance->SetUrl(VarArgs("asset://ui/buildMode.html?mode=%s", mode));
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, false, null, false);

	/*
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
	*/
}

void C_AnarchyManager::OnMountAllWorkshopsComplete()
{
	if (!m_pMountManager)	// it is our first time here
	{
		// SHOULD NEVER BE HTERE!!!!!
		// OBSOLETE!!!! 
		m_pMountManager = new C_MountManager();
		m_pMountManager->Init();
		m_pMountManager->LoadMountsFromKeyValues("mounts.txt");

		m_pBackpackManager->Init();

		m_pWorkshopManager = new C_WorkshopManager();
		m_pWorkshopManager->Init();
	}
	else
	{
		m_pBackpackManager->ActivateAllBackpacks();
		this->GetMetaverseManager()->DetectAllMaps();
	}
}

void C_AnarchyManager::OnRebuildSoundCacheCallback()
{
	DevMsg("Reinitializing sound system (so sounds work in addon maps)...\n");

	// restart the sound system so that mounted paths can play sounds
	engine->ClientCmd("snd_restart");

	ConVar* pConVar = cvar->FindVar("engine_no_focus_sleep");
	pConVar->SetValue(m_oldEngineNoFocusSleep.c_str());

	C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->GetSelectedAwesomiumBrowserInstance();
	//pHudBrowserInstance->SetUrl("asset://ui/welcome.html");
	pHudBrowserInstance->SetUrl("asset://ui/betasplash.html");
	g_pAnarchyManager->SetInitialized(true);
}

void C_AnarchyManager::OnDetectAllMapsComplete()
{
	//DevMsg("DISABLED FOR TESTING!\n");
	//return;
	///*

	DevMsg("Starting Libretro...\n");

	m_pLibretroManager = new C_LibretroManager();

	if (m_iState < 1)
	{
		m_iState = 1;

		// iterate through all models and assign the dynamic property to them
		// FIXME: THIS SHOULD BE DONE UPON MODEL IMPORT/LOADING!!
		m_pMetaverseManager->FlagDynamicModels();
		m_pMetaverseManager->DetectAllLegacyCabinets();

		// this is where steamGames.key could be auto-scanned to make sure all Steam games exist in the library, if wanted.
		if (false && g_pFullFileSystem->FileExists("steamGames.key", "DEFAULT_WRITE_PATH"))
		{
			KeyValues* kv = new KeyValues("steamgames");
			kv->LoadFromFile(g_pFullFileSystem, "steamGames.key", "DEFAULT_WRITE_PATH");
			g_pAnarchyManager->GetMetaverseManager()->ImportSteamGames(kv);
		}

		DevMsg("Initializing sound system...\n");
		C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->GetSelectedAwesomiumBrowserInstance();
		pHudBrowserInstance->AddHudLoadingMessage("", "", "Initializing Sound System...", "rebuildsound", "", "", "", "rebuildSoundCacheCallback");
	}
	else
	{
		DevMsg("Done again!!\n");
	}
	//*/
}

void C_AnarchyManager::Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters)
{
	std::string safeStr = str;
	std::transform(safeStr.begin(), safeStr.end(), safeStr.begin(), ::tolower);

	// Skip delimiters at beginning.
	std::string::size_type lastPos = safeStr.find_first_not_of(delimiters, 0);

	// Find first "non-delimiter".
	std::string::size_type pos = safeStr.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));

		// Skip delimiters.  Note the "not_of"
		lastPos = safeStr.find_first_not_of(delimiters, pos);

		// Find next "non-delimiter"
		pos = safeStr.find_first_of(delimiters, lastPos);
	}
}

/*
void C_AnarchyManager::ReleaseFileBrowseParams()
{
	if (m_pFileParams)
	{
		delete m_pFileParams;
		m_pFileParams = null;
	}
}
*/

void C_AnarchyManager::xCastSetLiveURL()
{
	//if (!m_pXSplitLiveConVar->GetBool())
	//	return;

	//DevMsg("Do anarchybot stuff\n");
	std::string XSPlitLiveFolder = cvar->FindVar("broadcast_folder")->GetString();
	if (XSPlitLiveFolder == "")
		return;

	std::string xml = "";
	xml += "<div class=\"response\">\n";

	KeyValues* active = null;
	C_BaseEntity* pEntity = this->GetSelectedEntity();
	if (pEntity)
	{
		C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);
		if (pShortcut)
		{
			KeyValues* pItem = m_pMetaverseManager->GetLibraryItem(pShortcut->GetItemId());
			if (pItem)
				active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pItem);
		}
	}
	else
	{
		// check for other open instances
		// TODO: This code should also be used for slave screens to show other instances!!

		std::vector<C_EmbeddedInstance*> embeddedInstances;
		m_pLibretroManager->GetAllInstances(embeddedInstances);
		m_pSteamBrowserManager->GetAllInstances(embeddedInstances);
		m_pAwesomiumBrowserManager->GetAllInstances(embeddedInstances);

		C_EmbeddedInstance* pEmbeddedInstance;
		std::string originalItemId;
		unsigned int i;
		unsigned int size = embeddedInstances.size();
		for (i = 0; i < size; i++)
		{
			pEmbeddedInstance = embeddedInstances[i];
			if (!pEmbeddedInstance)
				continue;

			// ignore special instances
			if (pEmbeddedInstance->GetId() == "hud" || pEmbeddedInstance->GetId() == "images")
				continue;

			// check for an original item id
			originalItemId = pEmbeddedInstance->GetOriginalItemId();
			if (originalItemId != "")
			{
				// WE HAVE FOUND AN EMBEDDED INSTANCE THAT WAS ORIGINALLY CREATED BY AN ITEM!!

				// make sure we can find an item for it
				KeyValues* pItem = m_pMetaverseManager->GetLibraryItem(originalItemId);
				if (pItem)
				{
					active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pItem);

					if ( active )
						break;
				}
			}
		}
	}

	if (active)
	{
		xml += "\t<item class=\"item_container\">\n";

		// automatically add all root-level keys & values
		for (KeyValues *sub = active->GetFirstValue(); sub; sub = sub->GetNextValue())
		{
			if (sub->GetFirstSubKey() || !Q_strcmp(sub->GetName(), "") || !Q_strcmp(sub->GetString(), ""))
				continue;

			xml += "\t\t<";
			xml += sub->GetName();
			xml += ">";

			std::string xmlBuf = sub->GetString();
			size_t found = xmlBuf.find("&");
			while (found != std::string::npos)
			{
				xmlBuf.replace(found, 1, "&amp;");
				found = xmlBuf.find("&", found + 5);
			}

			found = xmlBuf.find("<");
			while (found != std::string::npos)
			{
				xmlBuf.replace(found, 1, "&lt;");
				found = xmlBuf.find("<", found + 4);
			}

			found = xmlBuf.find(">");
			while (found != std::string::npos)
			{
				xmlBuf.replace(found, 1, "&gt;");
				found = xmlBuf.find(">", found + 4);
			}

			xml += xmlBuf;

			xml += "</";
			xml += sub->GetName();
			xml += ">\n";
		}

		// do some extra work to get the best possible image results
		std::string bestScreenImage = active->GetString("screen");
		if (bestScreenImage == "" || bestScreenImage.find(":") == 1 || bestScreenImage.find("http") != 0)
			bestScreenImage = active->GetString("marquee");
		if (bestScreenImage == "" || bestScreenImage.find(":") == 1 || bestScreenImage.find("http") != 0)
			bestScreenImage = active->GetString("file");
		if (bestScreenImage == "" || bestScreenImage.find(":") == 1 || bestScreenImage.find("http") != 0)
			bestScreenImage = "noimage.png";

		xml += "\t\t<screen2use>";
		xml += bestScreenImage;
		xml += "</screen2use>\n";

		std::string bestMarqueeImage = active->GetString("marquee");
		if (bestMarqueeImage == "" || bestMarqueeImage.find(":") == 1 || bestMarqueeImage.find("http") != 0)
			bestMarqueeImage = active->GetString("screen");
		if (bestMarqueeImage == "" || bestMarqueeImage.find(":") == 1 || bestMarqueeImage.find("http") != 0)
			bestMarqueeImage = active->GetString("file");
		if (bestMarqueeImage == "" || bestMarqueeImage.find(":") == 1 || bestMarqueeImage.find("http") != 0)
			bestMarqueeImage = "noimage.png";

		xml += "\t\t<marquee2use>";
		xml += bestMarqueeImage;
		xml += "</marquee2use>\n";

		xml += "\t\t<screenurl>";
		xml += active->GetString("screen");
		xml += "</screenurl>\n";

		xml += "\t\t<marqueeurl>";
		xml += active->GetString("marquee");
		xml += "</marqueeurl>\n";

		xml += "\t\t<bestimageurl>";
		if (bestMarqueeImage != "")
			xml += bestMarqueeImage;
		else
			xml += bestScreenImage;
		xml += "</bestimageurl>\n";


		xml += "\t\t<isremember>0</isremember>\n";
		xml += "\t</item>\n";
	}

	xml += "</div>";

	// Write this live URL out to the save file.
	FileHandle_t hFile = g_pFullFileSystem->Open(VarArgs("%s\\liveItems.xml", XSPlitLiveFolder.c_str()), "w", "");

	if (hFile)
	{
		//DevMsg("Writing to liveItems:\n");
		//DevMsg("%s\n", xml.c_str());

		/*
		// null terminate
		int len = xml.length();
		char* stuff = new char[xml.length() + 1];
		Q_strcpy(stuff, xml.c_str());
		stuff[len] = 0;	// null terminator
		*/

		//std::stringstream buffer;
		//buffer << "Text" << std::endl;

		g_pFullFileSystem->FPrintf(hFile, "%s", xml.c_str());
		g_pFullFileSystem->Close(hFile);
	}

	// Also update a JS file to force the page to re-load
	hFile = g_pFullFileSystem->Open(VarArgs("%s\\vote.js", XSPlitLiveFolder.c_str()), "a+", "");
	if (hFile)
	{
		std::string selectedItemTitle = "";
		if (active)
			selectedItemTitle = active->GetString("info/id");

		std::string rememberedItemTitle = "";
		//if (pRememberItemKV)
		//	rememberedItemTitle = pRememberItemKV->GetString("itemfilehash");

		std::string code = "gAnarchyTV.OnAArcadeCommand(\"selection\", \"";
		code += selectedItemTitle;
		code += "\", \"";
		code += rememberedItemTitle;
		code += "\");\n";

		//g_pFullFileSystem->FPrintf(hFile, VarArgs("var currentTick = \"%i\";\n", gpGlobals->tickcount));
		g_pFullFileSystem->Write(code.c_str(), code.length(), hFile);

		g_pFullFileSystem->Close(hFile);
	}

	//DevMsg("Done doing anarchybot stuff!!\n");
}

void C_AnarchyManager::TestSQLite2()
{
	DevMsg("LOAD SQL TEST...\n");
	int rc;
	char *error;

	// Open Database
	DevMsg("Opening MyDb.db ...\n");

	sqlite3 *db;
	rc = sqlite3_open("MyDb.db", &db);
	if (rc)
	{
		DevMsg("Error opening SQLite3 database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
		//return 1;
	}
	else
	{
		DevMsg("Opened MyDb.db.\n");
	}

	// Execute SQL
	sqlite3_stmt *stmt = NULL;
	rc = sqlite3_prepare(db, "SELECT * from MyTable", -1, &stmt, NULL);
	//rc = sqlite3_prepare(db, "INSERT INTO MyTable VALUES(NULL, ?)", -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed: %s\n", sqlite3_errmsg(db));

	int length;
	while (sqlite3_step(stmt) == SQLITE_ROW)	// THIS IS WHERE THE LOOP CAN BE BROKEN UP AT!!
	{
		length = sqlite3_column_bytes(stmt, 1);

		KeyValues* pTesterKV = new KeyValues("anotherTester");
		CUtlBuffer buf(0, length, 0);	// the length param should NEVER be zero.
		buf.CopyBuffer(sqlite3_column_blob(stmt, 1), length);
		pTesterKV->ReadAsBinary(buf);
		buf.Purge();
		DevMsg("Value here is: %s\n", pTesterKV->GetString("originalTesterKey"));

		/*
		void* buffer = malloc(length);
		memcpy(buffer, sqlite3_column_blob(stmt, 0), length);
		buf.Put(buffer, length);
		pTesterKV->ReadAsBinary(buf);
		DevMsg("Buf value has: %s\n", pTesterKV->GetString("originalTesterKey"));
		pTesterKV->deleteThis();
		*/

	//	buf.Put
	}
	sqlite3_finalize(stmt);	// TODO: error checking?  Maybe not needed, if this is like a close() operation.

	// Close Database
	DevMsg("Closing MyDb.db ...\n");
	sqlite3_close(db);
	DevMsg("Closed MyDb.db\n");
}

void C_AnarchyManager::SetNextLoadInfo(std::string instanceId, std::string position, std::string rotation)
{
	m_pNextLoadInfo->instanceId = instanceId;
	m_pNextLoadInfo->position = position;
	m_pNextLoadInfo->rotation = rotation;
}

void C_AnarchyManager::TestSQLite()
{
	int rc;
	char *error;

	// create the library database
	DevMsg("Create library database...\n");

	sqlite3 *db;
	rc = sqlite3_open("aarcade_user/library.db", &db);
	if (rc)
	{
		DevMsg("Error opening SQLite3 database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
	}

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

	// FIXME: TODO: Add the default stuff to the database by default?  or maybe this should be done through a higher-level system.

	// DATABASE IS NOW CREATED WITH THE PROPER TABLES, BUT IS COMPLETELY EMPTY.
	sqlite3_close(db);
	DevMsg("Library created.\n");
	// Execute SQL
	/*
	DevMsg("Inserting a value into MyTable ...\n");
	const char *sqlInsert = "INSERT INTO MyTable VALUES(NULL, 'A Value');";
	rc = sqlite3_exec(db, sqlInsert, NULL, NULL, &error);
	if (rc)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}
	else
	{
		DevMsg("Inserted a value into MyTable.\n");
	}
	*/
	/*
	DevMsg("Inserting a BINARY value into MyTable ...\n");
	sqlite3_stmt *stmt = NULL;
	rc = sqlite3_prepare(db,
		"INSERT INTO MyTable VALUES(NULL, ?)", -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed!\n");

	// SQLITE_STATIC because the statement is finalized
	// before the buffer is freed:

	CUtlBuffer buf;
	KeyValues* pObjectKV = new KeyValues("originalTester");//pInstanceObjectsKV->FindKey(VarArgs("%s/local", objectId.c_str()), true);
	pObjectKV->SetString("originalTesterKey", "yup");
	pObjectKV->WriteAsBinary(buf);
	pObjectKV->deleteThis();

	int size = buf.Size();

	rc = sqlite3_bind_blob(stmt, 1, buf.Base(), size, SQLITE_STATIC);
	if (rc != SQLITE_OK)
		DevMsg("bind failed: %s\n", sqlite3_errmsg(db));
	else
	{
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
			DevMsg("execution failed: %s\n", sqlite3_errmsg(db));
		else
			DevMsg("execution worked!\n");
	}

	sqlite3_finalize(stmt);
	*/
	/*
	// Display MyTable
	DevMsg("Retrieving values in MyTable ...\n");
	const char *sqlSelect = "SELECT * FROM MyTable;";
	char **results = NULL;
	int rows, columns;
	sqlite3_get_table(db, sqlSelect, &results, &rows, &columns, &error);
	if (rc)
	{
		DevMsg("Error executing SQLite3 query: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}
	else
	{
		// Display Table
		for (int rowCtr = 0; rowCtr <= rows; ++rowCtr)
		{
			for (int colCtr = 0; colCtr < columns; ++colCtr)
			{
				// Determine Cell Position
				int cellPosition = (rowCtr * columns) + colCtr;

				// Display Cell Value
				DevMsg("%s\t", results[cellPosition]);
				//cout.width(12);
				//cout.setf(ios::left);
				//cout << results[cellPosition] << " ";
			}

			// End Line
			//cout << endl;
			DevMsg("\n");

			// Display Separator For Header
			if (0 == rowCtr)
			{
				for (int colCtr = 0; colCtr < columns; ++colCtr)
				{
					//cout.width(12);
					//cout.setf(ios::left);
					DevMsg("~~~~~~~~~~~~");
				}
				DevMsg("\n");
			}
		}
	}
	sqlite3_free_table(results);
	*/

	// Close Database
	//DevMsg("Closing MyDb.db ...\n");
	//sqlite3_close(db);
	//DevMsg("Closed MyDb.db\n");


	/*
		CUtlBuffer buf;
		//buf.Get()
		KeyValues* pObjectKV = new KeyValues("originalTester");//pInstanceObjectsKV->FindKey(VarArgs("%s/local", objectId.c_str()), true);
		pObjectKV->SetString("originalTesterKey", "yup");
		pObjectKV->WriteAsBinary(buf);
		pObjectKV->deleteThis();

		KeyValues* pTesterKV = new KeyValues("reduxTester");
		pTesterKV->ReadAsBinary(buf);
		DevMsg("Annd here the big result is: %s\n", pTesterKV->GetString("originalTesterKey"));
		pTesterKV->deleteThis();
	*/
}

/*
void C_AnarchyManager::LevelInitPreEntity()
{
	DevMsg("AnarchyManager: LevelInitPreEntity\n");
	//m_pWebViewManager = new C_WebViewManager;
//	m_pWebViewManager->Init();
}

void C_AnarchyManager::LevelShutdownPostEntity()
{
	DevMsg("AnarchyManager: LevelShutdownPostEntity\n");
	// FIXME: Deleting the webview manager prevents it from starting up again.
	// Need to only create/delete it ONCE during the lifetime of AArcade.
	//delete m_pWebViewManager;
}
*/
/*
const char* C_AnarchyManager::GenerateHash(const char* text)
{
	char input[AA_MAX_STRING];
	Q_strcpy(input, text);

	// Convert it to lowercase & change all slashes to back-slashes
	V_FixSlashes(input);
	for( int i = 0; input[i] != '\0'; i++ )
		input[i] = tolower(input[i]);

	char lower[256];
	unsigned m_crc = 0xffffffff;

	int inputLength = strlen(input);
	for (int i = 0; i < inputLength; i++)
	{
		lower[i] = tolower(input[i]);
	}

	for (int i = 0; i < inputLength; i++)
	{
		unsigned c = lower[i];
		m_crc ^= (c << 24);

		for (int j = 0; j < 8; j++)
		{
			const unsigned FLAG = 0x80000000;
			if ((m_crc & FLAG) == FLAG)
			{
				m_crc = (m_crc << 1) ^ 0x04C11DB7;
			}
			else
			{
				m_crc <<= 1;
			}
		}
	}

	return VarArgs("%08x", m_crc);
}
*/